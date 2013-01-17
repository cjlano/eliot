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

#include <QtGui/QStandardItemModel>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QMenu>
#include <QtGui/QShortcut>

#include "arbit_assignments.h"
#include "qtcommon.h"
#include "custom_popup.h"
#include "prefs_dialog.h"

#include "public_game.h"
#include "player.h"
#include "turn_data.h"
#include "rack.h"
#include "results.h"
#include "settings.h"
#include "debug.h"

using namespace std;


INIT_LOGGER(qt, ArbitAssignments);


ArbitAssignments::ArbitAssignments(QWidget *parent, PublicGame *iGame)
    : QWidget(parent), m_game(iGame)
{
    setupUi(this);

    // Associate a model to the players view.
    // We use a proxy, to enable easy sorting of the players.
    m_proxyPlayersModel = new QSortFilterProxyModel(this);
    m_proxyPlayersModel->setDynamicSortFilter(true);
    m_playersModel = new QStandardItemModel(this);
    m_proxyPlayersModel->setSourceModel(m_playersModel);
    treeViewPlayers->setModel(m_proxyPlayersModel);
    m_playersModel->setColumnCount(8);
    m_playersModel->setHeaderData(0, Qt::Horizontal, _q("Table"), Qt::DisplayRole);
    m_playersModel->setHeaderData(1, Qt::Horizontal, _q("Player"), Qt::DisplayRole);
    m_playersModel->setHeaderData(2, Qt::Horizontal, _q("Word"), Qt::DisplayRole);
    m_playersModel->setHeaderData(3, Qt::Horizontal, _q("Ref"), Qt::DisplayRole);
    m_playersModel->setHeaderData(4, Qt::Horizontal, _q("Points"), Qt::DisplayRole);
    m_playersModel->setHeaderData(5, Qt::Horizontal, _q("S"), Qt::DisplayRole);
    m_playersModel->setHeaderData(6, Qt::Horizontal, _q("W"), Qt::DisplayRole);
    m_playersModel->setHeaderData(7, Qt::Horizontal, _q("P"), Qt::DisplayRole);
    treeViewPlayers->sortByColumn(0, Qt::AscendingOrder);

    treeViewPlayers->setColumnWidth(0, 70);
    treeViewPlayers->setColumnWidth(1, 150);
    treeViewPlayers->setColumnWidth(2, 130);
    treeViewPlayers->setColumnWidth(3, 40);
    treeViewPlayers->setColumnWidth(4, 50);
    treeViewPlayers->setColumnWidth(5, 25);
    treeViewPlayers->setColumnWidth(6, 25);
    treeViewPlayers->setColumnWidth(7, 25);

    QShortcut *shortcut;
    shortcut = new QShortcut(QKeySequence::Delete, treeViewPlayers);
    shortcut->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(shortcut, SIGNAL(activated()),
                     this, SLOT(suppressMove()));

    // TRANSLATORS: 'T' is the keyboard shortcut used in arbitration mode
    // to assign the top move to players. If translated, the translation
    // will be used as shortcut instead of 'T'.
    shortcut = new QShortcut(_q("T"), treeViewPlayers);
    shortcut->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(shortcut, SIGNAL(activated()),
                     this, SLOT(assignTopMove()));

    // TRANSLATORS: 'S' stands for Solo, it is used as column header in
    // the history, and in the players table in arbitration mode. It is also
    // used as shortcut to assign a solo, in arbitration mode.
    shortcut = new QShortcut(_q("S"), treeViewPlayers);
    shortcut->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(shortcut, SIGNAL(activated()),
                     this, SLOT(addRemoveSolo()));

    // TRANSLATORS: 'W' stands for Warning, it is used as column header in
    // the history, and in the players table in arbitration mode. It is also
    // used as shortcut to assign a warning, in arbitration mode.
    shortcut = new QShortcut(_q("W"), treeViewPlayers);
    shortcut->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(shortcut, SIGNAL(activated()),
                     this, SLOT(addRemoveWarning()));

    // TRANSLATORS: 'P' stands for Penalty, it is used as column header in
    // the history, and in the players table in arbitration mode. It is also
    // used as shortcut to assign a penalty, in arbitration mode.
    shortcut = new QShortcut(_q("P"), treeViewPlayers);
    shortcut->setContext(Qt::WidgetWithChildrenShortcut);
    QObject::connect(shortcut, SIGNAL(activated()),
                     this, SLOT(addRemovePenalty()));

    // Display a preview of the master word when clicked
    QObject::connect(labelMasterMove, SIGNAL(clicked()),
                     this, SLOT(showMasterPreview()));

    // Enable the assignment buttons according to the selections in trees
    QObject::connect(treeViewPlayers->selectionModel(),
                     SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                     this, SLOT(enableAssignmentButtons()));

    // Emit the "playerSelected" signal when appropriate
    QObject::connect(treeViewPlayers->selectionModel(),
                     SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                     this, SLOT(emitPlayerSelected()));

    // Move assignment
    QObject::connect(buttonSelectMaster, SIGNAL(clicked()),
                     this, SLOT(setMasterMove()));
    QObject::connect(buttonSuppressMove, SIGNAL(clicked()),
                     this, SLOT(suppressMove()));
    QObject::connect(buttonAssign, SIGNAL(clicked()),
                     this, SLOT(assignSelectedMove()));
    QObject::connect(treeViewPlayers, SIGNAL(activated(const QModelIndex&)),
                     this, SLOT(assignSelectedMove()));

    // End turn
    QObject::connect(buttonEndTurn, SIGNAL(clicked()),
                     this, SLOT(endTurn()));

    // Show/hide players with an assigned move
    QObject::connect(checkBoxHideAssigned, SIGNAL(toggled(bool)),
                     this, SLOT(updatePlayersModel()));

    // Add a context menu for the players
    CustomPopup *playersPopup = new CustomPopup(treeViewPlayers);
    QObject::connect(playersPopup, SIGNAL(popupCreated(QMenu&, const QPoint&)),
                     this, SLOT(populatePlayersMenu(QMenu&, const QPoint&)));

    refresh();
}


void ArbitAssignments::refresh()
{
    updatePlayersModel();

    // Update the master move
    const Move &masterMove = m_game->duplicateGetMasterMove();
    if (masterMove.isNull())
    {
        labelMasterMove->setText(QString("<i>%1</i>").arg(_q("Not selected yet")));
    }
    else
    {
        QString label = QString("<b>%1</b>").arg(formatMove(masterMove));
        labelMasterMove->setText(label);
    }

    setEnabled(!m_game->isFinished());

    enableAssignmentButtons();
    buttonSelectMaster->setEnabled(isSetMasterAllowed());
    buttonEndTurn->setEnabled(isEndTurnAllowed());
}


void ArbitAssignments::updatePlayersModel()
{
    // Save the ID of the selected players
    QSet<unsigned int> playersIdSet = getSelectedPlayers();
    // Save the currently focused player (to be able to restore the focus properly)
    int currProxyRow = treeViewPlayers->selectionModel()->currentIndex().row();
    QModelIndex currProxyIdx = m_proxyPlayersModel->index(currProxyRow, 0);
    unsigned currPlayerId = m_proxyPlayersModel->data(currProxyIdx, Qt::UserRole).toUInt();

    m_playersModel->removeRows(0, m_playersModel->rowCount());
    if (m_game == NULL)
        return;

    const bool hideAssignedPlayers = checkBoxHideAssigned->isChecked();
    for (unsigned int i = 0; i < m_game->getNbPlayers(); ++i)
    {
        const Player &player = m_game->getPlayer(i);
        // Only display human players
        if (!player.isHuman())
            continue;
        // Hide players with an assigned move other than "No move"
        if (hideAssignedPlayers && m_game->hasPlayed(player.getId()) &&
            !player.getLastMove().isNull())
        {
            continue;
        }

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
            if (move.isValid())
            {
                const Round &round = move.getRound();
                m_playersModel->setData(m_playersModel->index(rowNum, 2), qfw(round.getWord()));
                m_playersModel->setData(m_playersModel->index(rowNum, 3), qfw(round.getCoord().toString()));
            }
            else if (move.isInvalid())
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

        // Print the solos, warnings and penalties
        const TurnData &turnData = player.getHistory().getTurn(m_game->getCurrTurn() - 1);
        if (turnData.getSoloPoints() != 0)
            m_playersModel->setData(m_playersModel->index(rowNum, 5), turnData.getSoloPoints());
        if (turnData.getWarningsNb() != 0)
            m_playersModel->setData(m_playersModel->index(rowNum, 6), turnData.getWarningsNb());
        if (turnData.getPenaltyPoints() != 0)
            m_playersModel->setData(m_playersModel->index(rowNum, 7), turnData.getPenaltyPoints());

        // Restore the selection
        if (playersIdSet.contains(player.getId()))
        {
            LOG_DEBUG("selecting player " << player.getId());
            QModelIndex proxyIndex = m_proxyPlayersModel->mapFromSource(m_playersModel->index(rowNum, 0));
            treeViewPlayers->selectionModel()->select(proxyIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
        // Restore the focus
        if (player.getId() == currPlayerId)
        {
            QModelIndex proxyIndex = m_proxyPlayersModel->mapFromSource(m_playersModel->index(rowNum, 0));
            treeViewPlayers->selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::Current | QItemSelectionModel::Rows);
        }
    }
}


void ArbitAssignments::enableAssignmentButtons()
{
    bool hasSelResult = !m_selectedMove.isNull();
    bool hasSelPlayer = treeViewPlayers->selectionModel()->hasSelection();
    // Enable the "Assign move" button iff a move is selected
    // and at least one player in the tree view is selected
    buttonAssign->setEnabled(hasSelResult && hasSelPlayer);
    if (hasSelResult && hasSelPlayer)
    {
        const Move &move = m_selectedMove;
        buttonAssign->setToolTip(_q("Assign move (%1) to the selected player(s)")
                                 .arg(formatMove(move)));
    }
    buttonSuppressMove->setEnabled(isSuppressMoveAllowed());
    buttonSelectMaster->setEnabled(isSetMasterAllowed());
}


void ArbitAssignments::populatePlayersMenu(QMenu &iMenu, const QPoint &iPoint)
{
    const QModelIndex &index = treeViewPlayers->indexAt(iPoint);
    if (!index.isValid())
        return;

    // Action to assign the selected move
    QString selMoveString = _q("none");
    if (isAssignMoveAllowed())
    {
        const Move &move = m_selectedMove;
        selMoveString = formatMove(move);
    }
    QAction *assignSelMoveAction =
        new QAction(_q("Assign selected move (%1)").arg(selMoveString), this);
    assignSelMoveAction->setStatusTip(_q("Assign move (%1) to the selected player(s)")
                    .arg(selMoveString));
    assignSelMoveAction->setShortcut(Qt::Key_Enter);
    QObject::connect(assignSelMoveAction, SIGNAL(triggered()),
                     this, SLOT(assignSelectedMove()));
    iMenu.addAction(assignSelMoveAction);
    if (!isAssignMoveAllowed())
        assignSelMoveAction->setEnabled(false);

    // Action to assign the top move
    QAction *assignTopMoveAction = new QAction(_q("Assign top move (if unique)"), this);
    assignTopMoveAction->setStatusTip(_q("Assign the top move (if unique) to the selected player(s)"));
    assignTopMoveAction->setShortcut(_q("T"));
    QObject::connect(assignTopMoveAction, SIGNAL(triggered()),
                     this, SLOT(assignTopMove()));
    iMenu.addAction(assignTopMoveAction);

    // Action to suppress an assigned move
    QAction *suppressMoveAction = new QAction(_q("Suppress assigned move"), this);
    suppressMoveAction->setStatusTip(_q("Suppress the currently assigned move for the selected player(s)"));
    suppressMoveAction->setShortcut(Qt::Key_Delete);
    QObject::connect(suppressMoveAction, SIGNAL(triggered()),
                     this, SLOT(suppressMove()));
    iMenu.addAction(suppressMoveAction);
    if (!isSuppressMoveAllowed())
        suppressMoveAction->setEnabled(false);

    // Action to select all the players
    QAction *selectAllAction = new QAction(_q("Select all players"), this);
    selectAllAction->setStatusTip(_q("Select all the players"));
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    QObject::connect(selectAllAction, SIGNAL(triggered()),
                     this, SLOT(selectAllPlayers()));
    iMenu.addAction(selectAllAction);

    // Action to give or remove a solo to players
    QAction *soloAction = new QAction(_q("Give (or remove) a solo"), this);
    soloAction->setStatusTip(_q("Give a solo to the selected player, or remove it if (s)he already has one"));
    soloAction->setShortcut(_q("S"));
    QObject::connect(soloAction, SIGNAL(triggered()),
                     this, SLOT(addRemoveSolo()));
    iMenu.addAction(soloAction);
    if (useSoloAuto())
        soloAction->setEnabled(false);

    // Action to give or remove a warning to players
    QAction *warningAction = new QAction(_q("Give (or remove) a warning"), this);
    warningAction->setStatusTip(_q("Give a warning to the selected player(s), or remove it if they already have one"));
    warningAction->setShortcut(_q("W"));
    QObject::connect(warningAction, SIGNAL(triggered()),
                     this, SLOT(addRemoveWarning()));
    iMenu.addAction(warningAction);

    // Action to give or remove a penalty to players
    QAction *penaltyAction = new QAction(_q("Give (or remove) a penalty"), this);
    penaltyAction->setStatusTip(_q("Give a penalty to the selected player(s), or remove it if they already have one"));
    penaltyAction->setShortcut(_q("P"));
    QObject::connect(penaltyAction, SIGNAL(triggered()),
                     this, SLOT(addRemovePenalty()));
    iMenu.addAction(penaltyAction);
}


void ArbitAssignments::selectedMoveChanged(const Move &iMove)
{
    m_selectedMove = iMove;
    enableAssignmentButtons();
}


void ArbitAssignments::emitPlayerSelected()
{
    QSet<unsigned int> playersIdSet = getSelectedPlayers();
    if (playersIdSet.size() == 1)
        emit playerSelected(*playersIdSet.begin());
}


QSet<unsigned int> ArbitAssignments::getSelectedPlayers() const
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


bool ArbitAssignments::selectPlayerByTable(unsigned tabNb, QString *oName)
{
    // Unselect all the players
    treeViewPlayers->selectionModel()->clearSelection();
    for (int rowNum = 0; rowNum < m_playersModel->rowCount(); ++rowNum)
    {
        const QModelIndex &modelIndex = m_playersModel->index(rowNum, 0);
        if (m_playersModel->data(modelIndex).toUInt() == tabNb)
        {
            // Found!
            const QModelIndex &index = m_proxyPlayersModel->mapFromSource(modelIndex);
            treeViewPlayers->scrollTo(index);
            treeViewPlayers->selectionModel()->select(index,
                    QItemSelectionModel::Select | QItemSelectionModel::Rows);
            *oName = m_playersModel->data(m_playersModel->index(rowNum, 1)).toString();
            return true;
        }
    }
    return false;
}


void ArbitAssignments::showMasterPreview()
{
    const Move &move = m_game->duplicateGetMasterMove();
    if (move.isValid())
    {
        // TODO: deselect move in the Results? or reselect the master move?
        m_game->setTestRound(move.getRound());
        emit gameUpdated();
    }
}


void ArbitAssignments::selectAllPlayers()
{
    treeViewPlayers->selectAll();
    emit notifyInfo(_q("All players selected"));
}


bool ArbitAssignments::isSetMasterAllowed() const
{
    return m_game->isLastTurn() && m_selectedMove.isValid() &&
        !m_game->hasMasterGame();
}


void ArbitAssignments::setMasterMove()
{
    if (!isSetMasterAllowed())
        return;

    const Move &masterMove = m_game->duplicateGetMasterMove();
    // Make sure the user knows what she's doing
    if (!masterMove.isNull())
    {
        QString msg = _q("There is already a master move for this turn.");
        QString question = _q("Do you want to replace it?");
        if (!QtCommon::requestConfirmation(PrefsDialog::kCONFO_ARBIT_REPLACE_MASTER,
                                           msg, question))
        {
            return;
        }
    }

    const Move &move = m_selectedMove;

    // Warn if the selected move is not a top move
    BestResults results;
    results.search(m_game->getDic(), m_game->getBoard(),
                   m_game->getCurrentRack().getRack(),
                   m_game->getHistory().beforeFirstRound());
    ASSERT(!results.isEmpty(), "No possible valid move");
    int bestScore = results.get(0).getPoints();
    if (bestScore > move.getScore())
    {
        QString msg = _q("The selected move scores less than the maximum.");
        QString question = _q("Do you really want to select it as master move?");
        if (!QtCommon::requestConfirmation(PrefsDialog::kCONFO_ARBIT_LOW_MASTER,
                                           msg, question))
        {
            return;
        }
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
                    if (!QtCommon::requestConfirmation(PrefsDialog::kCONFO_ARBIT_MASTER_JOKERS,
                                                       msg, question))
                    {
                        return;
                    }
                    break;
                }
            }
        }
    }

    // Assign the master move
    m_game->duplicateSetMasterMove(move);
    emit gameUpdated();
}


void ArbitAssignments::setDefaultMasterMove()
{
    const Move &currMove = m_game->duplicateGetMasterMove();
    // Do not overwrite an existing move
    if (!currMove.isNull())
        return;

    // Search the best moves
    MasterResults results(m_game->getBag());
    results.search(m_game->getDic(), m_game->getBoard(),
                   m_game->getCurrentRack().getRack(),
                   m_game->getHistory().beforeFirstRound());
    // XXX: End of game
    if (results.isEmpty())
        return;

    // Find a good default
    Move move = Move(results.get(0));

    // Assign the master move
    m_game->duplicateSetMasterMove(move);
    emit gameUpdated();
}


bool ArbitAssignments::isAssignMoveAllowed() const
{
    return !m_selectedMove.isNull() &&
        treeViewPlayers->selectionModel()->hasSelection();
}


void ArbitAssignments::assignSelectedMove()
{
    if (!isAssignMoveAllowed())
        return;
    helperAssignMove(m_selectedMove);
}


void ArbitAssignments::assignTopMove()
{
    BestResults results;
    results.search(m_game->getDic(), m_game->getBoard(),
                   m_game->getCurrentRack().getRack(),
                   m_game->getHistory().beforeFirstRound());
    if (results.size() == 1)
        helperAssignMove(Move(results.get(0)));
    else
    {
        LOG_DEBUG("The top move is not unique");
    }
}


bool ArbitAssignments::isSuppressMoveAllowed() const
{
    // Return true if at least one of the selected players has
    // a move to suppress
    QSet<unsigned int> playersIdSet = getSelectedPlayers();
    BOOST_FOREACH(unsigned int id, playersIdSet)
    {
        if (m_game->hasPlayed(id) &&
            !m_game->getPlayer(id).getLastMove().isNull())
        {
            return true;
        }
    }
    return false;
}


void ArbitAssignments::suppressMove()
{
    if (!isSuppressMoveAllowed())
        return;

    helperAssignMove(Move());
}


void ArbitAssignments::helperAssignMove(const Move &iMove)
{
    QSet<unsigned int> playersIdSet = getSelectedPlayers();

    // Warn if some of the selected players already have an assigned move
    QSet<unsigned int> assignedIdSet;
    BOOST_FOREACH(unsigned int id, playersIdSet)
    {
        if (m_game->hasPlayed(id) &&
            !m_game->getPlayer(id).getLastMove().isNull())
        {
            assignedIdSet.insert(id);
        }
    }
    if (!assignedIdSet.empty())
    {
        QString players;
        BOOST_FOREACH(unsigned int id, assignedIdSet)
        {
            players = QString("\t%1\n").arg(qfw(m_game->getPlayer(id).getName()));
        }
        // The warning is different depending on the type of move
        if (iMove.isNull())
        {
            QString msg = _q("You are going to suppress the assigned move for the following players:\n");
            if (!QtCommon::requestConfirmation(PrefsDialog::kCONFO_ARBIT_SUPPR_MOVE,
                                               msg + players))
            {
                return;
            }
        }
        else
        {
            QString msg = _q("The following players already have an assigned move:\n");
            QString question = _q("Do you want to replace it?");
            if (!QtCommon::requestConfirmation(PrefsDialog::kCONFO_ARBIT_REPLACE_MOVE,
                                               msg + players, question))
            {
                return;
            }
        }
    }

    // Assign the move to each selected player
    BOOST_FOREACH(unsigned int id, playersIdSet)
    {
        LOG_DEBUG(lfq(QString("Assigning move %1 to player %2")
                      .arg(qfw(iMove.toString())).arg(id)));

        // Assign the move
        m_game->arbitrationAssign(id, iMove);
    }
    if (iMove.isNull())
        emit notifyInfo(_q("Move assignment suppressed"));
    else
        emit notifyInfo(_q("Move assigned to player(s)"));
    emit gameUpdated();
}


bool ArbitAssignments::useSoloAuto() const
{
    return Settings::Instance().getBool("arbitration.solo-auto");
}


void ArbitAssignments::addRemoveSolo()
{
    if (useSoloAuto())
        return;

    QSet<unsigned int> playersIdSet = getSelectedPlayers();
    // Only one player can have a solo
    if (playersIdSet.size() != 1)
        return;

    BOOST_FOREACH(unsigned int id, playersIdSet)
    {
        m_game->arbitrationToggleSolo(id);
    }
    emit gameUpdated();
}


void ArbitAssignments::addRemoveWarning()
{
    QSet<unsigned int> playersIdSet = getSelectedPlayers();
    if (playersIdSet.isEmpty())
        return;

    BOOST_FOREACH(unsigned int id, playersIdSet)
    {
        m_game->arbitrationToggleWarning(id);
    }
    emit gameUpdated();
}


void ArbitAssignments::addRemovePenalty()
{
    QSet<unsigned int> playersIdSet = getSelectedPlayers();
    if (playersIdSet.isEmpty())
        return;

    BOOST_FOREACH(unsigned int id, playersIdSet)
    {
        m_game->arbitrationTogglePenalty(id);
    }
    emit gameUpdated();
}


QString ArbitAssignments::formatMove(const Move &iMove) const
{
    if (iMove.isValid())
    {
        return QString("%1 - %2 - %3")
            .arg(qfw(iMove.getRound().getWord()))
            .arg(qfw(iMove.getRound().getCoord().toString()))
            .arg(iMove.getScore());
    }
    else
    {
        ASSERT(iMove.isInvalid(), "Unexpected move type");
        return QString("%1 - %2 - %3")
            .arg(qfw(iMove.getBadWord()))
            .arg(qfw(iMove.getBadCoord()))
            .arg(iMove.getScore());
    }
}


bool ArbitAssignments::isEndTurnAllowed() const
{
    return m_game->isLastTurn();
}


void ArbitAssignments::endTurn()
{
    if (!isEndTurnAllowed())
        return;

    if (!m_game->duplicateGetMasterMove().isValid())
    {
        notifyProblem(_q("You must select a master move before ending the turn."));
        return;
    }

    bool allPlayed = true;
    for (unsigned int i = 0; i < m_game->getNbPlayers(); ++i)
    {
        if (m_game->getPlayer(i).isHuman() &&
            (!m_game->hasPlayed(i) || m_game->getPlayer(i).getLastMove().isNull()))
        {
            allPlayed = false;
            break;
        }
    }
    if (!allPlayed)
    {
        QString msg = _q("Some player(s) have no assigned move for this turn. "
                         "If you continue, they will be assigned a \"(NO MOVE)\" "
                         "pseudo-move, but you will be able to change that later.");
        if (!QtCommon::requestConfirmation(PrefsDialog::kCONFO_ARBIT_INCOMPLETE_TURN,
                                           msg))
        {
            return;
        }
    }

    emit notifyInfo(_q("New turn started"));

    m_game->removeTestRound();
    m_game->arbitrationFinalizeTurn();

    emit endOfTurn();
}

