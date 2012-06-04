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

#include <QtGui/QStandardItemModel>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QMenu>
#include <QtCore/QSettings>

#include "arbitration_widget.h"
#include "arbit_assignments.h"
#include "qtcommon.h"
#include "prefs_dialog.h"
#include "validator_factory.h"
#include "play_word_mediator.h"
#include "custom_popup.h"
#include "misc_helpers.h"

#include "public_game.h"
#include "player.h"
#include "rack.h"
#include "results.h"
#include "coord_model.h"
#include "settings.h"
#include "game_params.h"
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

    m_assignmentsWidget = new ArbitAssignments(this, iGame);
    layoutAssignments->addWidget(m_assignmentsWidget);
    QObject::connect(m_assignmentsWidget, SIGNAL(gameUpdated()),
                     this, SIGNAL(gameUpdated()));
    QObject::connect(m_assignmentsWidget, SIGNAL(notifyProblem(QString)),
                     this, SIGNAL(notifyProblem(QString)));
    QObject::connect(m_assignmentsWidget, SIGNAL(notifyInfo(QString)),
                     this, SIGNAL(notifyInfo(QString)));
    QObject::connect(m_assignmentsWidget, SIGNAL(endOfTurn()),
                     this, SLOT(endOfTurnRefresh()));
    QObject::connect(m_assignmentsWidget, SIGNAL(playerSelected(unsigned)),
                     this, SIGNAL(playerSelected(unsigned)));

    m_keyAccum = new KeyAccumulator(this, 400);

    // The players widget uses more space by default
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);

    blackPalette = lineEditRack->palette();
    redPalette = lineEditRack->palette();
    redPalette.setColor(QPalette::Text, Qt::red);

    // Define validators
    m_unstrictRackValidator =
        ValidatorFactory::newRackValidator(this, m_game->getBag());
    QValidator * val =
        ValidatorFactory::newRackValidator(this, m_game->getBag(),
                                           true, &m_game->getHistory(),
                                           m_game->getParams().hasVariant(GameParams::k7AMONG8) ? 8 : 7);
    lineEditRack->setValidator(val);
    lineEditCoords->setValidator(ValidatorFactory::newCoordsValidator(this));

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

    KeyEventFilter *masterFilter = new KeyEventFilter(this, Qt::Key_M, Qt::SHIFT);
    QObject::connect(masterFilter, SIGNAL(keyPressed(int, int)),
                     m_assignmentsWidget, SLOT(assignMasterMove()));
    treeViewResults->installEventFilter(masterFilter);

    KeyEventFilter *selectAllFilter = new KeyEventFilter(this, Qt::Key_A, Qt::SHIFT);
    QObject::connect(selectAllFilter, SIGNAL(keyPressed(int, int)),
                     m_assignmentsWidget, SLOT(selectAllPlayers()));
    treeViewResults->installEventFilter(selectAllFilter);

    KeyEventFilter *numFilter = new KeyEventFilter(this, Qt::Key_0);
    numFilter->addKey(Qt::Key_1);
    numFilter->addKey(Qt::Key_2);
    numFilter->addKey(Qt::Key_3);
    numFilter->addKey(Qt::Key_4);
    numFilter->addKey(Qt::Key_5);
    numFilter->addKey(Qt::Key_6);
    numFilter->addKey(Qt::Key_7);
    numFilter->addKey(Qt::Key_8);
    numFilter->addKey(Qt::Key_9);
    numFilter->setIgnoreModifiers();
    QObject::connect(numFilter, SIGNAL(keyPressed(int, int)),
                     this, SLOT(selectTableNumber(int)));
    treeViewResults->installEventFilter(numFilter);

    // Validate manual rack changes
    QObject::connect(lineEditRack, SIGNAL(textEdited(const QString&)),
                     this, SLOT(rackEdited(const QString&)));
    // Propagate the information on rack change
    QObject::connect(lineEditRack, SIGNAL(textChanged(const QString&)),
                     this, SIGNAL(rackUpdated(const QString&)));
    // Clear the results when the rack changes
    QObject::connect(lineEditRack, SIGNAL(textChanged(const QString&)),
                     this, SLOT(rackChanged()));
    // Perform a search on Enter
    QObject::connect(lineEditRack, SIGNAL(returnPressed()),
                     this, SLOT(searchResults()));

    // Set a random rack
    QObject::connect(buttonRandom, SIGNAL(clicked()),
                     this, SLOT(setRackRandom()));

    // Perform a search
    QObject::connect(buttonSearch, SIGNAL(clicked()),
                     this, SLOT(searchResults()));

    // Display a preview of the selected word on the board
    QObject::connect(treeViewResults->selectionModel(),
                     SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                     this, SLOT(showPreview(const QItemSelection&)));

    // Dynamic filter for search results
    QObject::connect(lineEditFilter, SIGNAL(textChanged(const QString&)),
                     this, SLOT(resultsFilterChanged(const QString&)));

    // Enable the assignment buttons according to the selections in trees
    QObject::connect(treeViewResults->selectionModel(),
                     SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                     m_assignmentsWidget, SLOT(enableAssignmentButtons()));

    // Enable the assignment buttons according to the selections in trees
    QObject::connect(treeViewResults->selectionModel(),
                     SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                     this, SLOT(updateSelectedMove()));

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
                     m_assignmentsWidget, SLOT(assignSelectedMove()));

    // Add a context menu for the results
    m_resultsPopup = new CustomPopup(treeViewResults);
    QObject::connect(m_resultsPopup, SIGNAL(popupCreated(QMenu&, const QPoint&)),
                     this, SLOT(populateResultsMenu(QMenu&, const QPoint&)));
    QObject::connect(m_resultsPopup, SIGNAL(requestDefinition(QString)),
                     this, SIGNAL(requestDefinition(QString)));

    refresh();

    // Give focus to the rack
    // FIXME: for some reason, the focus gets lost later...
    lineEditRack->setFocus();
    lineEditRack->selectAll();
}


void ArbitrationWidget::refresh()
{
    const PlayedRack &pldRack = m_game->getCurrentRack();
    // Update the rack only if needed, to avoid losing cursor position
    QString qrack = qfw(pldRack.toString(PlayedRack::RACK_SIMPLE));
    if (qrack != lineEditRack->text()) {
        // Save the selection status
        bool isAllselected = lineEditRack->text() == lineEditRack->selectedText();

        // Must be done before updateResultsModel(), because it will
        // indirectly call the clearResults() method
        lineEditRack->setText(qrack);

        // Restore the selection
        if (isAllselected)
            lineEditRack->selectAll();
    }

    updateResultsModel();
    m_assignmentsWidget->refresh();

    // The rack can only be changed at the last turn
    bool isLastTurn = m_game->isLastTurn();
    lineEditRack->setReadOnly(!isLastTurn);
    buttonRandom->setEnabled(isLastTurn);

    // Disable controls when the game is finished, but only
    // for the last turn (to allow changing players moves in
    // previous turns, for example)
    bool disable = m_game->isFinished() && isLastTurn;
    setEnabled(!disable);
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


int ArbitrationWidget::addSingleMove(const Move &iMove, int moveType,
                                     unsigned int index, int bestScore)
{
    static const QBrush redBrush(Qt::red);

    int rowNum = m_resultsModel->rowCount();
    m_resultsModel->insertRow(rowNum);
    if (iMove.isValid())
    {
        const Round &r = iMove.getRound();
        m_resultsModel->setData(m_resultsModel->index(rowNum, 0), qfw(r.getWord()));
        m_resultsModel->setData(m_resultsModel->index(rowNum, 1),
                                qfw(r.getCoord().toString()));
    }
    else
    {
        ASSERT(iMove.isInvalid(), "Unexpected move type");
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


bool ArbitrationWidget::confirmRackChange() const
{
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
        if (!QtCommon::requestConfirmation(PrefsDialog::kCONFO_ARBIT_CHANGE_RACK,
                                           msg, question))
        {
            return false;
        }
    }
    return true;
}


void ArbitrationWidget::setRackRandom()
{
    ASSERT(m_game->isLastTurn(),
           "The Random button should only be active in the last turn");

    // Request confirmation
    if (!confirmRackChange())
        return;

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

    // Request confirmation
    if (!confirmRackChange())
    {
        // Restore the rack (visually)
        const PlayedRack &pldRack = m_game->getCurrentRack();
        QString qrack = qfw(pldRack.toString(PlayedRack::RACK_SIMPLE));
        lineEditRack->setText(qrack);
        return;
    }

    try
    {
        // Update the game rack if it is valid, or if it is "almost valid",
        // i.e. in Intermediate state due to the duplicate constraints only.
        // This is practical to have the rack updated letter by letter on the
        // external board.
        QString copy = iText;
        int unused = 0;
        if (lineEditRack->hasAcceptableInput() ||
            m_unstrictRackValidator->validate(copy, unused) == QValidator::Acceptable)
        {
            const wstring &input = m_game->getDic().convertFromInput(wfq(iText));
            m_game->arbitrationSetRackManual(input);
            emit gameUpdated();
        }
    }
    catch (std::exception &e)
    {
        lineEditRack->setPalette(redPalette);
        emit notifyProblem(_q("Warning: Cannot set the rack to '%1':\n%2").arg(iText).arg(e.what()));
    }
}


void ArbitrationWidget::rackChanged()
{
    bool acceptableInput = lineEditRack->hasAcceptableInput();
    buttonSearch->setEnabled(acceptableInput);
    lineEditRack->setPalette(acceptableInput ? blackPalette : redPalette);
    clearResults();
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


void ArbitrationWidget::searchResults()
{
    m_game->removeTestRound();
    emit notifyInfo(_q("Searching with rack '%1'...").arg(lineEditRack->text()));
    lineEditFilter->clear();
    m_results.clear();
    m_game->arbitrationSearch(m_results);
    emit notifyInfo(_q("Search done"));
    emit gameUpdated();

    QSettings settings;
    if (settings.value(PrefsDialog::kARBIT_AUTO_MASTER, false).toBool())
    {
        m_assignmentsWidget->assignDefaultMasterMove();
    }

    // Set the focus to the first result
    selectAndFocusResult(0, false);
}


void ArbitrationWidget::populateResultsMenu(QMenu &iMenu, const QPoint &iPoint)
{
    const QModelIndex &index = treeViewResults->indexAt(iPoint);
    if (!index.isValid())
        return;

    const Move &move = getSelectedMove();

    // Action to display the word definition
    const QModelIndex &wordIndex = m_resultsModel->index(index.row(), 0);
    QString selectedWord = m_resultsModel->data(wordIndex).toString();
    QAction *showDefAction = m_resultsPopup->getShowDefinitionEntry(selectedWord);
    iMenu.addAction(showDefAction);
    if (!move.isValid())
        showDefAction->setEnabled(false);

    // Action to select as master move
    QAction *setAsMasterAction =
        new QAction(_q("Use as master move"), this);
    setAsMasterAction->setStatusTip(_q("Use the selected move (%1) as master move")
                                    .arg(formatMove(move)));
    setAsMasterAction->setShortcut(Qt::SHIFT + Qt::Key_M);
    QObject::connect(setAsMasterAction, SIGNAL(triggered()),
                     m_assignmentsWidget, SLOT(assignMasterMove()));
    iMenu.addAction(setAsMasterAction);
    if (!move.isValid())
        setAsMasterAction->setEnabled(false);

    // Action to select all the players
    QAction *selectAllAction =
        new QAction(_q("Select all players"), this);
    selectAllAction->setStatusTip(_q("Select all the players"));
    selectAllAction->setShortcut(Qt::SHIFT + Qt::Key_A);
    QObject::connect(selectAllAction, SIGNAL(triggered()),
                     m_assignmentsWidget, SLOT(selectAllPlayers()));
    iMenu.addAction(selectAllAction);

    // Action to assign the selected move
    QAction *assignSelMoveAction =
        new QAction(_q("Assign selected move (%1)").arg(formatMove(move)), this);
    assignSelMoveAction->setStatusTip(_q("Assign move (%1) to the selected player(s)")
                                      .arg(formatMove(move)));
    assignSelMoveAction->setShortcut(Qt::Key_Enter);
    QObject::connect(assignSelMoveAction, SIGNAL(triggered()),
                     m_assignmentsWidget, SLOT(assignSelectedMove()));
    iMenu.addAction(assignSelMoveAction);
    if (!m_assignmentsWidget->hasSelectedPlayer())
        assignSelMoveAction->setEnabled(false);
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
    selectAndFocusResult(rowNum);
}


void ArbitrationWidget::selectAndFocusResult(int iRowNum, bool logical)
{
    QModelIndex index = m_resultsModel->index(iRowNum, 0);
    if (logical)
        index = m_proxyResultsModel->mapFromSource(index);
    treeViewResults->scrollTo(index);
    treeViewResults->selectionModel()->clearSelection();
    treeViewResults->selectionModel()->select(index,
                    QItemSelectionModel::Select | QItemSelectionModel::Rows);
    treeViewResults->setFocus();
}


void ArbitrationWidget::selectTableNumber(int key)
{
    QString keyStr = "";
    if (key == Qt::Key_0) keyStr = "0";
    else if (key == Qt::Key_1) keyStr = "1";
    else if (key == Qt::Key_2) keyStr = "2";
    else if (key == Qt::Key_3) keyStr = "3";
    else if (key == Qt::Key_4) keyStr = "4";
    else if (key == Qt::Key_5) keyStr = "5";
    else if (key == Qt::Key_6) keyStr = "6";
    else if (key == Qt::Key_7) keyStr = "7";
    else if (key == Qt::Key_8) keyStr = "8";
    else if (key == Qt::Key_9) keyStr = "9";
    ASSERT(keyStr != "", "Unexpected key");

    // Build (and retrieve) the table number
    QString tableNum = m_keyAccum->addText(keyStr);

    // Select the player with this table number
    LOG_DEBUG("Selecting player with table number: " + lfq(tableNum));
    QString name;
    bool found = m_assignmentsWidget->selectPlayerByTable(tableNum.toUInt(), &name);
    if (found)
    {
        // Keep the focus in the results view
        treeViewResults->setFocus();

        // Write a nice message on the status bar
        emit notifyInfo(_q("Player at table %1 selected (%2)").arg(tableNum).arg(name));
        return;
    }
    LOG_DEBUG("Not found");
}


void ArbitrationWidget::endOfTurnRefresh()
{
    // Refresh everything
    emit gameUpdated();

    // Give focus to the rack
    lineEditRack->setFocus();
}


void ArbitrationWidget::updateSelectedMove()
{
    bool hasSelection = treeViewResults->selectionModel()->hasSelection();
    if (hasSelection)
        m_assignmentsWidget->selectedMoveChanged(getSelectedMove());
    else
        m_assignmentsWidget->selectedMoveChanged(Move());
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


void ArbitrationWidget::clearResults()
{
    m_game->removeTestRound();
    m_results.clear();
    m_addedMoves.clear();
    m_resultsModel->removeRows(0, m_resultsModel->rowCount());
}


void ArbitrationWidget::showPreview(const QItemSelection &iSelected)
{
    m_game->removeTestRound();
    if (!iSelected.indexes().empty())
    {
        const Move &move = getSelectedMove();
        if (move.isValid())
        {
            m_game->setTestRound(move.getRound());
        }
        emit gameUpdated();
    }
}


int ArbitrationWidget::getBestScore() const
{
    // Note: we could cache the result of this method
    BestResults results;
    results.search(m_game->getDic(), m_game->getBoard(),
                   m_game->getCurrentRack().getRack(),
                   m_game->getHistory().beforeFirstRound());
    ASSERT(results.size() != 0, "No possible valid move");
    return results.get(0).getPoints();
}


QString ArbitrationWidget::formatMove(const Move &iMove) const
{
    return m_assignmentsWidget->formatMove(iMove);
}

