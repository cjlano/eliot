/*****************************************************************************
 * Eliot
 * Copyright (C) 2012 Olivier Teulière
 * Authors: Olivier Teulière <ipkiss @@ gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

#include <boost/foreach.hpp>
#include <vector>

#include <QtGui/QStandardItemModel>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QMessageBox>
#include <QtGui/QMenu>
#include <QtGui/QKeyEvent>
#include <QtCore/QSettings>

#include "arbitration_widget.h"
#include "qtcommon.h"
#include "prefs_dialog.h"
#include "validator_factory.h"
#include "play_word_mediator.h"
#include "custom_popup.h"
#include "misc_helpers.h"

#include "public_game.h"
#include "player.h"
#include "pldrack.h"
#include "rack.h"
#include "results.h"
#include "coord_model.h"
#include "settings.h"
#include "dic.h"
#include "debug.h"

using namespace std;

static const int MOVE_TYPE_ROLE = Qt::UserRole;
static const int MOVE_INDEX_ROLE = Qt::UserRole + 1;

static const int TYPE_ROUND = 1; // The result is a valid round, coming from a search
static const int TYPE_ADDED = 2; // The result can be valid or invalid, it was manually added

INIT_LOGGER(qt, ArbitrationWidget);


ArbitrationWidget::ArbitrationWidget(QWidget *parent,
                                     PublicGame *iGame, CoordModel &iCoordModel)
    : QWidget(parent), m_game(iGame), m_coordModel(iCoordModel), m_results(10)
{
    setupUi(this);

    // FIXME arbitration begin
    // The players widget uses more space by default
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    // FIXME arbitration end

    blackPalette = lineEditRack->palette();
    redPalette = lineEditRack->palette();
    redPalette.setColor(QPalette::Text, Qt::red);

    // Define validators
    QValidator * val =
        ValidatorFactory::newRackValidator(this, m_game->getBag(),
                                           true, &m_game->getHistory());
    lineEditRack->setValidator(val);
    lineEditCoords->setValidator(ValidatorFactory::newCoordsValidator(this));

    // Associate a model to the players view.
    // We use a proxy, to enable easy sorting of the players.
    m_proxyPlayersModel = new QSortFilterProxyModel(this);
    m_proxyPlayersModel->setDynamicSortFilter(true);
    m_playersModel = new QStandardItemModel(this);
    m_proxyPlayersModel->setSourceModel(m_playersModel);
    treeViewPlayers->setModel(m_proxyPlayersModel);
    m_playersModel->setColumnCount(4);
    m_playersModel->setHeaderData(0, Qt::Horizontal, _q("Table"), Qt::DisplayRole);
    m_playersModel->setHeaderData(1, Qt::Horizontal, _q("Player"), Qt::DisplayRole);
    m_playersModel->setHeaderData(2, Qt::Horizontal, _q("Word"), Qt::DisplayRole);
    m_playersModel->setHeaderData(3, Qt::Horizontal, _q("Ref"), Qt::DisplayRole);
    m_playersModel->setHeaderData(4, Qt::Horizontal, _q("Points"), Qt::DisplayRole);
    treeViewPlayers->sortByColumn(0, Qt::AscendingOrder);

    treeViewPlayers->setColumnWidth(0, 70);
    treeViewPlayers->setColumnWidth(1, 160);
    treeViewPlayers->setColumnWidth(2, 160);
    treeViewPlayers->setColumnWidth(3, 40);
    treeViewPlayers->setColumnWidth(4, 50);

    KeyEventFilter *filter = new KeyEventFilter(this, Qt::Key_T);
    QObject::connect(filter, SIGNAL(keyPressed()),
                     this, SLOT(assignTopMove()));
    treeViewPlayers->installEventFilter(filter);

    // Associate a model to the results view.
    // We use a proxy, to enable easy sorting/filtering of the results.
    m_proxyResultsModel = new QSortFilterProxyModel(this);
    m_proxyResultsModel->setDynamicSortFilter(true);
    m_proxyResultsModel->setFilterKeyColumn(0);
    m_proxyResultsModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_resultsModel = new QStandardItemModel(this);
    m_proxyResultsModel->setSourceModel(m_resultsModel);
    treeViewResults->setModel(m_proxyResultsModel);

    m_resultsModel->setColumnCount(4);
    m_resultsModel->setHeaderData(0, Qt::Horizontal, _q("Word"), Qt::DisplayRole);
    m_resultsModel->setHeaderData(1, Qt::Horizontal, _q("Ref"), Qt::DisplayRole);
    m_resultsModel->setHeaderData(2, Qt::Horizontal, _q("Points"), Qt::DisplayRole);
    m_resultsModel->setHeaderData(3, Qt::Horizontal, _q("Status"), Qt::DisplayRole);
    treeViewResults->sortByColumn(2);

    m_proxyResultsModel->setSupportedDragActions(Qt::CopyAction);

    treeViewResults->setColumnWidth(0, 120);
    treeViewResults->setColumnWidth(1, 40);
    treeViewResults->setColumnWidth(2, 70);

    // Validate manual rack changes
    QObject::connect(lineEditRack, SIGNAL(textEdited(const QString&)),
                     this, SLOT(rackEdited(const QString&)));
    // Propagate the information on rack change
    QObject::connect(lineEditRack, SIGNAL(textChanged(const QString&)),
                     this, SIGNAL(rackUpdated(const QString&)));
    // Clear the results when the rack changes
    QObject::connect(lineEditRack, SIGNAL(textChanged(const QString&)),
                     this, SLOT(clearResults()));

    // Set a random rack
    QObject::connect(buttonRandom, SIGNAL(clicked()),
                     this, SLOT(setRackRandom()));

    // Display a preview of the selected word on the board
    QObject::connect(treeViewResults->selectionModel(),
                     SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                     this, SLOT(showPreview(const QItemSelection&)));
    // Display a preview of the master word when clicked
    QObject::connect(labelMasterMove, SIGNAL(clicked()),
                     this, SLOT(showMasterPreview()));

    // Dynamic filter for search results
    QObject::connect(lineEditFilter, SIGNAL(textChanged(const QString&)),
                     this, SLOT(resultsFilterChanged(const QString&)));

    // Enable the assignment buttons according to the selections in trees
    QObject::connect(treeViewPlayers->selectionModel(),
                     SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                     this, SLOT(enableAssignmentButtons()));
    QObject::connect(treeViewResults->selectionModel(),
                     SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                     this, SLOT(enableAssignmentButtons()));

    // Coordinates model
    QObject::connect(&m_coordModel, SIGNAL(coordChanged(const Coord&, const Coord&)),
                     this, SLOT(updateCoordText(const Coord&, const Coord&)));
    QObject::connect(lineEditCoords, SIGNAL(textChanged(const QString&)),
                     this, SLOT(updateCoordModel(const QString&)));

    // Enable the "Check word" button only when there is a word with coordinates
    QObject::connect(lineEditWord, SIGNAL(textChanged(const QString&)),
                     this, SLOT(enableCheckWordButton()));
    QObject::connect(lineEditCoords, SIGNAL(textChanged(const QString&)),
                     this, SLOT(enableCheckWordButton()));

    // Check the given word
    QObject::connect(lineEditWord, SIGNAL(returnPressed()),
                     this, SLOT(checkWord()));
    QObject::connect(lineEditCoords, SIGNAL(returnPressed()),
                     this, SLOT(checkWord()));
    QObject::connect(buttonCheck, SIGNAL(clicked()),
                     this, SLOT(checkWord()));

    // Move assignment
    QObject::connect(treeViewResults, SIGNAL(activated(const QModelIndex&)),
                     this, SLOT(assignMasterMove()));
    QObject::connect(buttonSelectMaster, SIGNAL(clicked()),
                     this, SLOT(assignMasterMove()));
    QObject::connect(buttonAssign, SIGNAL(clicked()),
                     this, SLOT(assignSelectedMove()));
    QObject::connect(buttonNoMove, SIGNAL(clicked()),
                     this, SLOT(assignNoMove()));
    QObject::connect(treeViewPlayers, SIGNAL(activated(const QModelIndex&)),
                     this, SLOT(assignSelectedMove()));

    // End turn
    QObject::connect(buttonEndTurn, SIGNAL(clicked()),
                     this, SLOT(endTurn()));

    // Add a context menu for the results
    m_resultsPopup = new CustomPopup(treeViewResults);
    QObject::connect(m_resultsPopup, SIGNAL(popupCreated(QMenu&, const QPoint&)),
                     this, SLOT(populateResultsMenu(QMenu&, const QPoint&)));
    QObject::connect(m_resultsPopup, SIGNAL(requestDefinition(QString)),
                     this, SIGNAL(requestDefinition(QString)));
    // Add a context menu for the players
    CustomPopup *playersPopup = new CustomPopup(treeViewPlayers);
    QObject::connect(playersPopup, SIGNAL(popupCreated(QMenu&, const QPoint&)),
                     this, SLOT(populatePlayersMenu(QMenu&, const QPoint&)));

    refresh();
}


void ArbitrationWidget::refresh()
{
    const PlayedRack &pldRack = m_game->getHistory().getCurrentRack();
    // Update the rack only if needed, to avoid losing cursor position
    QString qrack = qfw(pldRack.toString(PlayedRack::RACK_SIMPLE));
    if (qrack != lineEditRack->text()) {
        // Must be done before updateResultsModel(), because it will
        // indirectly call the clearResults() slot
        lineEditRack->setText(qrack);
    }

    updateResultsModel();
    updatePlayersModel();

    // Update the master move
    const Move &masterMove = m_game->duplicateGetMasterMove();
    if (masterMove.getType() == Move::NO_MOVE)
    {
        labelMasterMove->setText(QString("<i>%1</i>").arg(_q("Not selected yet")));
    }
    else
    {
        QString label = QString("<b>%1</b>").arg(formatMove(masterMove));
        labelMasterMove->setText(label);
    }

    // The rack can only be changed at the last turn
    bool isLastTurn = m_game->isLastTurn();
    lineEditRack->setReadOnly(!isLastTurn);
    buttonRandom->setEnabled(isLastTurn);

    if (m_game->isFinished())
    {
        setEnabled(false);
    }
}


void ArbitrationWidget::updateResultsModel()
{
#if 1
    // FIXME arbitration begin
    // Consider that there is nothing to do if the number of lines is correct
    // This avoids problems when the game is updated for a test play
    if (m_game != NULL &&
        m_results.size() + m_addedMoves.size() ==
                static_cast<unsigned int>(m_resultsModel->rowCount()))
    {
        return;
    }
    // FIXME arbitration end
#endif

    m_resultsModel->removeRows(0, m_resultsModel->rowCount());
    if (m_game == NULL)
        return;

    // First step: add the search results (type TYPE_ROUND)
    // Find the highest score
    int bestScore = getBestScore();
    for (unsigned int i = 0; i < m_results.size(); ++i)
    {
        Move move(m_results.get(i));
        addSingleMove(move, TYPE_ROUND, i, bestScore);
    }

    // Second step: add the manually added words (type TYPE_ADDED)
    for (int i = 0; i < m_addedMoves.size(); ++i)
    {
        const Move &move = m_addedMoves.at(i);
        LOG_DEBUG("Adding custom move: " << lfw(move.toString()));
        addSingleMove(move, TYPE_ADDED, i, bestScore);
    }

    // FIXME arbitration begin
    // XXX: useful?
    //m_proxyResultsModel->invalidate();
    // FIXME arbitration end
}


void ArbitrationWidget::updatePlayersModel()
{
    // Save the ID of the selected players
    QSet<unsigned int> playersIdSet = getSelectedPlayers();

    m_playersModel->removeRows(0, m_playersModel->rowCount());
    if (m_game == NULL)
        return;

    const bool hideAssignedPlayers = checkBoxHideAssigned->isChecked();
    for (unsigned int i = 0; i < m_game->getNbPlayers(); ++i)
    {
        const Player &player = m_game->getPlayer(i);
        if (hideAssignedPlayers && m_game->hasPlayed(player.getId()))
            continue;
        // Only display human players
        if (!player.isHuman())
            continue;
        const int rowNum = m_playersModel->rowCount();
        m_playersModel->insertRow(rowNum);
        m_playersModel->setData(m_playersModel->index(rowNum, 0), player.getTableNb());
        m_playersModel->setData(m_playersModel->index(rowNum, 1), qfw(player.getName()));
        // Store the player ID
        m_playersModel->setData(m_playersModel->index(rowNum, 0),
                                player.getId(), Qt::UserRole);
        // Display the played word, if any
        if (m_game->hasPlayed(player.getId()))
        {
            const Move &move = player.getLastMove();
            m_playersModel->setData(m_playersModel->index(rowNum, 4), move.getScore());

            QPalette palette = treeViewPlayers->palette();
            QColor color = palette.color(QPalette::Normal, QPalette::WindowText);
            if (move.getType() == Move::VALID_ROUND)
            {
                const Round &round = move.getRound();
                m_playersModel->setData(m_playersModel->index(rowNum, 2), qfw(round.getWord()));
                m_playersModel->setData(m_playersModel->index(rowNum, 3), qfw(round.getCoord().toString()));
            }
            else if (move.getType() == Move::NO_MOVE)
            {
                m_playersModel->setData(m_playersModel->index(rowNum, 2), _q("(NO MOVE)"));
                color = Qt::blue;
            }
            else if (move.getType() == Move::INVALID_WORD)
            {
                m_playersModel->setData(m_playersModel->index(rowNum, 2), "<" + qfw(move.getBadWord()) + ">");
                m_playersModel->setData(m_playersModel->index(rowNum, 3), qfw(move.getBadCoord()));
                color = Qt::red;
            }
            // Apply the color
            const QBrush brush(color);
            m_playersModel->setData(m_playersModel->index(rowNum, 2),
                                    brush, Qt::ForegroundRole);
            m_playersModel->setData(m_playersModel->index(rowNum, 3),
                                    brush, Qt::ForegroundRole);
        }
        // Restore the selection
        if (playersIdSet.contains(player.getId()))
        {
            LOG_DEBUG("selecting player " << player.getId());
            treeViewPlayers->selectionModel()->select(m_playersModel->index(rowNum, 0),
                                                      QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }
}


int ArbitrationWidget::addSingleMove(const Move &iMove, int moveType,
                                     unsigned int index, int bestScore)
{
    static const QBrush redBrush(Qt::red);

    int rowNum = m_resultsModel->rowCount();
    m_resultsModel->insertRow(rowNum);
    if (iMove.getType() == Move::VALID_ROUND)
    {
        const Round &r = iMove.getRound();
        m_resultsModel->setData(m_resultsModel->index(rowNum, 0), qfw(r.getWord()));
        m_resultsModel->setData(m_resultsModel->index(rowNum, 1),
                                qfw(r.getCoord().toString()));
    }
    else
    {
        ASSERT(iMove.getType() == Move::INVALID_WORD, "Unexpected move type");
        m_resultsModel->setData(m_resultsModel->index(rowNum, 0), qfw(iMove.getBadWord()));
        m_resultsModel->setData(m_resultsModel->index(rowNum, 1), qfw(iMove.getBadCoord()));
        m_resultsModel->setData(m_resultsModel->index(rowNum, 3), _q("Invalid"));
        m_resultsModel->setData(m_resultsModel->index(rowNum, 3), redBrush, Qt::ForegroundRole);
#if 0
        QFont myFont;
        myFont.setBold(true);
        for (int i = 0; i < 4; ++i)
        {
            m_resultsModel->setData(m_resultsModel->index(rowNum, i),
                                    myFont, Qt::FontRole);
        }
#endif
    }
    m_resultsModel->setData(m_resultsModel->index(rowNum, 2), iMove.getScore());

    if (iMove.getScore() == bestScore)
    {
        // Color the line in red if this is the top score
        for (int i = 0; i < 3; ++i)
        {
            m_resultsModel->setData(m_resultsModel->index(rowNum, i),
                                    redBrush, Qt::ForegroundRole);
        }
    }

    // Store the move type and round index in the first column, as user data
    m_resultsModel->setData(m_resultsModel->index(rowNum, 0), moveType, MOVE_TYPE_ROLE);
    m_resultsModel->setData(m_resultsModel->index(rowNum, 0), index, MOVE_INDEX_ROLE);

    return rowNum;
}


void ArbitrationWidget::setRackRandom()
{
    ASSERT(m_game->isLastTurn(),
           "The Random button should only be active in the last turn");

    // Warn if some players have already played
    bool someoneHasPlayed = false;
    for (unsigned int i = 0; i < m_game->getNbPlayers(); ++i)
    {
        if (m_game->hasPlayed(i))
            someoneHasPlayed = true;
    }
    if (someoneHasPlayed)
    {
        QString msg = _q("Some player(s) already have an assigned move. "
                         "These moves will be lost if you change the rack.");
        QString question = _q("Do you really want to change the rack?");
        if (!QtCommon::requestConfirmation(msg, question))
            return;
    }

    m_game->removeTestRound();
    try
    {
        m_game->arbitrationSetRackRandom();
        emit gameUpdated();
    }
    catch (const std::exception &e)
    {
        emit notifyProblem(_q(e.what()));
    }
}


void ArbitrationWidget::rackEdited(const QString &iText)
{
    ASSERT(m_game->isLastTurn(),
           "Rack edition button should only be active in the last turn");

    // Warn if some players have already played
    bool someoneHasPlayed = false;
    for (unsigned int i = 0; i < m_game->getNbPlayers(); ++i)
    {
        if (m_game->hasPlayed(i))
            someoneHasPlayed = true;
    }
    if (someoneHasPlayed)
    {
        QString msg = _q("Some player(s) already have an assigned move. "
                         "These moves will be lost if you change the rack.");
        QString question = _q("Do you really want to change the rack?");
        if (!QtCommon::requestConfirmation(msg, question))
        {
            // Restore the rack (visually)
            const PlayedRack &pldRack = m_game->getHistory().getCurrentRack();
            QString qrack = qfw(pldRack.toString(PlayedRack::RACK_SIMPLE));
            lineEditRack->setText(qrack);

            return;
        }
    }

    m_game->removeTestRound();
    if (!lineEditRack->hasAcceptableInput())
    {
        lineEditRack->setPalette(redPalette);
        return;
    }
    try
    {
        lineEditRack->setPalette(blackPalette);
        const wstring &input = m_game->getDic().convertFromInput(wfq(iText));
        m_game->arbitrationSetRackManual(input);
        buttonSearch->setEnabled(m_resultsModel->rowCount() == 0 &&
                                 lineEditRack->text() != "");
        emit gameUpdated();
    }
    catch (std::exception &e)
    {
        lineEditRack->setPalette(redPalette);
        emit notifyProblem(_q("Warning: Cannot set the rack to '%1':\n%2").arg(iText).arg(e.what()));
    }
}


void ArbitrationWidget::resultsFilterChanged(const QString &iFilter)
{
    treeViewResults->clearSelection();
    m_proxyResultsModel->setFilterFixedString(iFilter);
    emit gameUpdated();
}


void ArbitrationWidget::updateCoordText(const Coord &, const Coord &iNewCoord)
{
    if (iNewCoord.isValid() && lineEditCoords->text() != qfw(iNewCoord.toString()))
        lineEditCoords->setText(qfw(iNewCoord.toString()));
    else if (!iNewCoord.isValid() && lineEditCoords->hasAcceptableInput())
        lineEditCoords->setText("");
    lineEditWord->setFocus();
}


void ArbitrationWidget::updateCoordModel(const QString &iText)
{
    Coord c(wfq(iText));
    if (!(m_coordModel.getCoord() == c))
        m_coordModel.setCoord(Coord(wfq(iText)));
}


void ArbitrationWidget::enableCheckWordButton()
{
    buttonCheck->setEnabled(!lineEditWord->text().isEmpty() &&
                            !lineEditCoords->text().isEmpty());
}


void ArbitrationWidget::enableAssignmentButtons()
{
    bool hasSelResult = m_game->isLastTurn() &&
        treeViewResults->selectionModel()->hasSelection();
    bool hasSelPlayer = m_game->isLastTurn() &&
        treeViewPlayers->selectionModel()->hasSelection();
    // Enable the "Assign move" button iff a move is selected
    // and at least one player in the tree view is selected
    buttonAssign->setEnabled(hasSelResult && hasSelPlayer);
    if (hasSelResult && hasSelPlayer)
    {
        const Move &move = getSelectedMove();
        buttonAssign->setToolTip(_q("Assign move (%1) to the selected player(s)")
                                 .arg(formatMove(move)));
    }
    buttonNoMove->setEnabled(hasSelPlayer);
    buttonSelectMaster->setEnabled(hasSelResult);
}


void ArbitrationWidget::on_buttonSearch_clicked()
{
    m_game->removeTestRound();
    emit notifyInfo(_q("Searching with rack '%1'...").arg(lineEditRack->text()));
    m_results.clear();
    m_game->arbitrationSearch(m_results);
    emit notifyInfo(_q("Search done"));
    emit gameUpdated();

    QSettings settings;
    if (settings.value(PrefsDialog::kARBIT_AUTO_MASTER, false).toBool())
    {
        assignDefaultMasterMove();
    }
}


void ArbitrationWidget::populateResultsMenu(QMenu &iMenu, const QPoint &iPoint)
{
    const QModelIndex &index = treeViewResults->indexAt(iPoint);
    if (!index.isValid())
        return;

    const Move &move = getSelectedMove();
    if (move.getType() == Move::VALID_ROUND)
    {
        // Action to display the word definition
        const QModelIndex &wordIndex = m_resultsModel->index(index.row(), 0);
        QString selectedWord = m_resultsModel->data(wordIndex).toString();
        m_resultsPopup->addShowDefinitionEntry(iMenu, selectedWord);

        // Action to select as master move
        QAction *setAsMasterAction =
            new QAction(_q("Use as master move"), this);
        setAsMasterAction->setStatusTip(_q("Use the selected move (%1) as master move")
                                        .arg(formatMove(move)));
        setAsMasterAction->setShortcut(Qt::Key_Enter);
        QObject::connect(setAsMasterAction, SIGNAL(triggered()),
                         this, SLOT(assignMasterMove()));
        iMenu.addAction(setAsMasterAction);
    }
}


void ArbitrationWidget::populatePlayersMenu(QMenu &iMenu, const QPoint &iPoint)
{
    const QModelIndex &index = treeViewPlayers->indexAt(iPoint);
    if (!index.isValid())
        return;

    // Action to assign the selected move
    if (treeViewResults->selectionModel()->hasSelection())
    {
        const Move &move = getSelectedMove();
        QAction *assignSelMoveAction =
            new QAction(_q("Assign selected move (%1)").arg(formatMove(move)), this);
        assignSelMoveAction->setStatusTip(_q("Assign move (%1) to the selected player(s)")
                        .arg(formatMove(move)));
        assignSelMoveAction->setShortcut(Qt::Key_Enter);
        QObject::connect(assignSelMoveAction, SIGNAL(triggered()),
                         this, SLOT(assignSelectedMove()));
        iMenu.addAction(assignSelMoveAction);
    }

#if 1
    // Action to assign the top move
    QAction *assignTopMoveAction =
        new QAction(_q("Assign top move"), this);
    assignTopMoveAction->setStatusTip(_q("Assign the top move (if unique) to the selected player(s)"));
    assignTopMoveAction->setShortcut(Qt::Key_T);
    QObject::connect(assignTopMoveAction, SIGNAL(triggered()),
                     this, SLOT(assignTopMove()));
    iMenu.addAction(assignTopMoveAction);
#endif

    // TODO: add other actions
}


void ArbitrationWidget::checkWord()
{
    if (lineEditWord->text().isEmpty() ||
        lineEditCoords->text().isEmpty())
    {
        return;
    }

    // Retrieve the played word
    wstring playedWord;
    QString errorMsg;
    bool ok = PlayWordMediator::GetPlayedWord(*lineEditWord, m_game->getDic(),
                                              &playedWord, &errorMsg);
    if (!ok)
    {
        emit notifyProblem(errorMsg);
        return;
    }
    // Retrieve the coordinates
    const wstring &coords = wfq(lineEditCoords->text());

    // Check the word
    const Move &move = m_game->arbitrationCheckWord(playedWord, coords);

    // XXX: we add it even if it is already present in the results.
    // This is not a real problem, but it is not particularly nice :)
    m_addedMoves.append(move);
    int rowNum = addSingleMove(move, TYPE_ADDED, m_addedMoves.size() - 1,
                               getBestScore());

    lineEditWord->clear();
    lineEditCoords->clear();

    // Show the new result and select it
    const QModelIndex &index =
        m_proxyResultsModel->mapFromSource(m_resultsModel->index(rowNum, 0));
    treeViewResults->scrollTo(index);
    treeViewResults->selectionModel()->clearSelection();
    treeViewResults->selectionModel()->select(index,
                    QItemSelectionModel::Select | QItemSelectionModel::Rows);
    treeViewResults->setFocus();
}


void ArbitrationWidget::on_checkBoxHideAssigned_toggled(bool)
{
    treeViewPlayers->selectionModel()->clearSelection();
    updatePlayersModel();
}


Move ArbitrationWidget::getSelectedMove() const
{
    // Get the tree selection
    const QItemSelection &proxySelected = treeViewResults->selectionModel()->selection();
    ASSERT(!proxySelected.empty(), "Empty selection!");

    // Map the selection to a source model index
    const QItemSelection &srcSelected = m_proxyResultsModel->mapSelectionToSource(proxySelected);
    // Use the hidden column to get the result number
    const QModelIndex &index =
        m_resultsModel->index(srcSelected.indexes().first().row(), 0);

    int origin = m_resultsModel->data(index, MOVE_TYPE_ROLE).toInt();
    if (origin == TYPE_ROUND)
    {
        unsigned int resNb = m_resultsModel->data(index, MOVE_INDEX_ROLE).toUInt();
        ASSERT(resNb < m_results.size(), "Wrong result number");
        return Move(m_results.get(resNb));
    }
    else
    {
        int vectPos = m_resultsModel->data(index, MOVE_INDEX_ROLE).toInt();
        ASSERT(vectPos >= 0 && vectPos < m_addedMoves.size(), "Wrong index number");
        return m_addedMoves.at(vectPos);
    }
}


QSet<unsigned int> ArbitrationWidget::getSelectedPlayers() const
{
    QSet<unsigned int> playersIdSet;

    // Get the tree selection
    const QItemSelection &proxySelected = treeViewPlayers->selectionModel()->selection();
    // Map the selection to a source model index
    const QItemSelection &srcSelected = m_proxyPlayersModel->mapSelectionToSource(proxySelected);
    // Get the player ID for each line
    Q_FOREACH(const QModelIndex &index, srcSelected.indexes())
    {
        // Only take the first column into account
        if (index.column() != 0)
            continue;
        playersIdSet.insert(m_playersModel->data(index, Qt::UserRole).toUInt());
    }

    return playersIdSet;
}


void ArbitrationWidget::clearResults()
{
    m_game->removeTestRound();
    m_results.clear();
}


void ArbitrationWidget::showPreview(const QItemSelection &iSelected)
{
    m_game->removeTestRound();
    if (!iSelected.indexes().empty())
    {
        const Move &move = getSelectedMove();
        if (move.getType() == Move::VALID_ROUND)
        {
            m_game->setTestRound(move.getRound());
        }
        emit gameUpdated();
    }
}


void ArbitrationWidget::showMasterPreview()
{
    const Move &move = m_game->duplicateGetMasterMove();
    if (move.getType() == Move::VALID_ROUND)
    {
        treeViewResults->clearSelection();
        m_game->setTestRound(move.getRound());
    }
    emit gameUpdated();
}


Rack ArbitrationWidget::getRack() const
{
    return m_game->getHistory().getCurrentRack().getRack();
}


int ArbitrationWidget::getBestScore() const
{
    // Note: we could cache the result of this method
    BestResults results;
    results.search(m_game->getDic(), m_game->getBoard(),
                   getRack(), m_game->getHistory().beforeFirstRound());
    ASSERT(results.size() != 0, "No possible valid move");
    return results.get(0).getPoints();
}


void ArbitrationWidget::assignMasterMove()
{
    if (m_game->isFinished())
        return;

    const Move &masterMove = m_game->duplicateGetMasterMove();
    // Make sure the user knows what she's doing
    if (masterMove.getType() != Move::NO_MOVE)
    {
        QString msg = _q("There is already a master move for this turn.");
        QString question = _q("Do you want to replace it?");
        if (!QtCommon::requestConfirmation(msg, question))
            return;
    }

    const Move &move = getSelectedMove();
    if (move.getType() != Move::VALID_ROUND)
    {
        notifyProblem(_q("The master move must be a valid move."));
        return;
    }

    // Warn if the selected move is not a top move
    BestResults results;
    results.search(m_game->getDic(), m_game->getBoard(),
                   getRack(), m_game->getHistory().beforeFirstRound());
    ASSERT(results.size() != 0, "No possible valid move");
    int bestScore = results.get(0).getPoints();
    if (bestScore > move.getScore())
    {
        QString msg = _q("The selected move scores less than the maximum.");
        QString question = _q("Do you really want to select it as master move?");
        if (!QtCommon::requestConfirmation(msg, question))
            return;
    }
    else
    {
        // A top move saving a joker should be prefered over one not
        // saving it, according to the French official rules.
        // Warn if the user tries to ignore the rule.
        unsigned jokerCount = move.getRound().countJokersFromRack();
        if (jokerCount > 0)
        {
            for (unsigned i = 0; i < results.size(); ++i)
            {
                if (results.get(i).countJokersFromRack() < jokerCount)
                {
                    QString msg = _q("The selected move uses more jokers than "
                                     "another move with the same score (%1).")
                        .arg(formatMove(Move(results.get(i))));
                    QString question = _q("Do you really want to select it as master move?");
                    if (!QtCommon::requestConfirmation(msg, question))
                        return;
                    break;
                }
            }
        }
    }

    // Assign the master move
    m_game->duplicateSetMasterMove(move);
    emit gameUpdated();
}


void ArbitrationWidget::assignDefaultMasterMove()
{
    const Move &currMove = m_game->duplicateGetMasterMove();
    // Do not overwrite an existing move
    if (currMove.getType() != Move::NO_MOVE)
        return;

    // Search the best moves
    BestResults results;
    results.search(m_game->getDic(), m_game->getBoard(),
                   getRack(), m_game->getHistory().beforeFirstRound());
    // XXX: End of game
    if (results.size() == 0)
        return;

    unsigned currIndex = 0;
    unsigned jokerCount = results.get(0).countJokersFromRack();
    if (jokerCount > 0)
    {
        for (unsigned i = 1; i < results.size(); ++i)
        {
            if (results.get(i).countJokersFromRack() < jokerCount)
            {
                currIndex = i;
                jokerCount = results.get(i).countJokersFromRack();
            }
        }
    }

    // Assign the master move
    Move move = Move(results.get(currIndex));
    m_game->duplicateSetMasterMove(move);
    emit gameUpdated();
}


void ArbitrationWidget::assignSelectedMove()
{
    if (!treeViewResults->selectionModel()->hasSelection())
        return;
    helperAssignMove(getSelectedMove());
}


void ArbitrationWidget::assignTopMove()
{
    BestResults results;
    results.search(m_game->getDic(), m_game->getBoard(),
                   getRack(), m_game->getHistory().beforeFirstRound());
    // TODO: what if there are several moves?
    if (results.size() == 1)
        helperAssignMove(Move(results.get(0)));
}


void ArbitrationWidget::assignNoMove()
{
    helperAssignMove(Move());
}


void ArbitrationWidget::helperAssignMove(const Move &iMove)
{
    QSet<unsigned int> playersIdSet = getSelectedPlayers();

    // Warn if some of the selected players already have an assigned move
    QSet<unsigned int> assignedIdSet;
    BOOST_FOREACH(unsigned int id, playersIdSet)
    {
        if (m_game->hasPlayed(id))
            assignedIdSet.insert(id);
    }
    if (!assignedIdSet.empty())
    {
        QString msg = _q("The following players already have an assigned move:\n");
        BOOST_FOREACH(unsigned int id, assignedIdSet)
        {
            msg += QString("\t%1\n").arg(qfw(m_game->getPlayer(id).getName()));
        }
        QString question = _q("Do you want to replace it?");
        if (!QtCommon::requestConfirmation(msg, question))
            return;
    }

    // Assign the move to each selected player
    BOOST_FOREACH(unsigned int id, playersIdSet)
    {
        LOG_DEBUG(lfq(QString("Assigning move %1 to player %2")
                      .arg(qfw(iMove.toString())).arg(id)));

        // Assign the move
        m_game->arbitrationAssign(id, iMove);
    }
    emit notifyInfo(_q("Move assigned to player(s)"));
    emit gameUpdated();
}


QString ArbitrationWidget::formatMove(const Move &iMove) const
{
    if (iMove.getType() == Move::VALID_ROUND)
    {
        return QString("%1 - %2 - %3")
            .arg(qfw(iMove.getRound().getWord()))
            .arg(qfw(iMove.getRound().getCoord().toString()))
            .arg(iMove.getScore());
    }
    else
    {
        ASSERT(iMove.getType() == Move::INVALID_WORD, "Unexpected move type");
        return QString("%1 - %2 - %3")
            .arg(qfw(iMove.getBadWord()))
            .arg(qfw(iMove.getBadCoord()))
            .arg(iMove.getScore());
    }
}


void ArbitrationWidget::endTurn()
{
    if (m_game->duplicateGetMasterMove().getType() != Move::VALID_ROUND)
    {
        notifyProblem(_q("You must select a master move before ending the turn."));
        return;
    }

    bool allPlayed = true;
    for (unsigned int i = 0; i < m_game->getNbPlayers(); ++i)
    {
        if (!m_game->hasPlayed(i) && m_game->getPlayer(i).isHuman())
            allPlayed = false;
    }
    if (!allPlayed)
    {
        QString msg = _q("Some player(s) have no assigned move. "
                         "If you continue, they will be assigned a \"(NO MOVE)\" "
                         "pseudo-move, but you will be able to change that later.");
        if (!QtCommon::requestConfirmation(msg))
            return;
    }

    m_addedMoves.clear();

    emit notifyInfo(_q("New turn started"));

    m_game->removeTestRound();
    m_game->arbitrationFinalizeTurn();
    // FIXME: shouldn't be done here
    setEnabled(!m_game->isFinished());

    emit gameUpdated();
}

