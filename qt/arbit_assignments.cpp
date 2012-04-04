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

#include "arbit_assignments.h"
#include "qtcommon.h"
#include "custom_popup.h"
#include "misc_helpers.h"

#include "public_game.h"
#include "player.h"
#include "rack.h"
#include "results.h"
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
    QObject::connect(filter, SIGNAL(keyPressed(int, int)),
                     this, SLOT(assignTopMove()));
    treeViewPlayers->installEventFilter(filter);

    filter = new KeyEventFilter(this, Qt::Key_W);
    QObject::connect(filter, SIGNAL(keyPressed(int, int)),
                     this, SLOT(addRemoveWarning()));
    treeViewPlayers->installEventFilter(filter);

    filter = new KeyEventFilter(this, Qt::Key_P);
    QObject::connect(filter, SIGNAL(keyPressed(int, int)),
                     this, SLOT(addPenalty()));
    treeViewPlayers->installEventFilter(filter);

    // Display a preview of the master word when clicked
    QObject::connect(labelMasterMove, SIGNAL(clicked()),
                     this, SLOT(showMasterPreview()));

    // Enable the assignment buttons according to the selections in trees
    QObject::connect(treeViewPlayers->selectionModel(),
                     SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                     this, SLOT(enableAssignmentButtons()));

    // Move assignment
    QObject::connect(buttonSelectMaster, SIGNAL(clicked()),
                     this, SLOT(assignMasterMove()));
    QObject::connect(buttonNoMove, SIGNAL(clicked()),
                     this, SLOT(assignNoMove()));
    QObject::connect(buttonAssign, SIGNAL(clicked()),
                     this, SLOT(assignSelectedMove()));
    QObject::connect(treeViewPlayers, SIGNAL(activated(const QModelIndex&)),
                     this, SLOT(assignSelectedMove()));

    // End turn
    QObject::connect(buttonEndTurn, SIGNAL(clicked()),
                     this, SLOT(endTurn()));

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
    if (masterMove.getType() == Move::NO_MOVE)
    {
        labelMasterMove->setText(QString("<i>%1</i>").arg(_q("Not selected yet")));
    }
    else
    {
        QString label = QString("<b>%1</b>").arg(formatMove(masterMove));
        labelMasterMove->setText(label);
    }

    if (m_game->isFinished())
    {
        setEnabled(false);
    }
}


void ArbitAssignments::updatePlayersModel()
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
        // Only display human players
        if (!player.isHuman())
            continue;
        // Hide players with an assigned move other than "No move"
        if (hideAssignedPlayers && m_game->hasPlayed(player.getId()) &&
            player.getLastMove().getType() != Move::NO_MOVE)
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


void ArbitAssignments::enableAssignmentButtons()
{
    bool hasSelResult = m_game->isLastTurn() &&
        m_selectedMove.getType() != Move::NO_MOVE;
    bool hasSelPlayer = m_game->isLastTurn() &&
        treeViewPlayers->selectionModel()->hasSelection();
    // Enable the "Assign move" button iff a move is selected
    // and at least one player in the tree view is selected
    buttonAssign->setEnabled(hasSelResult && hasSelPlayer);
    if (hasSelResult && hasSelPlayer)
    {
        const Move &move = m_selectedMove;
        buttonAssign->setToolTip(_q("Assign move (%1) to the selected player(s)")
                                 .arg(formatMove(move)));
    }
    buttonNoMove->setEnabled(hasSelPlayer);
    buttonSelectMaster->setEnabled(hasSelResult);
}


void ArbitAssignments::populatePlayersMenu(QMenu &iMenu, const QPoint &iPoint)
{
    const QModelIndex &index = treeViewPlayers->indexAt(iPoint);
    if (!index.isValid())
        return;

    // Action to assign the selected move
    if (m_selectedMove.getType() != Move::NO_MOVE)
    {
        const Move &move = m_selectedMove;
        QAction *assignSelMoveAction =
            new QAction(_q("Assign selected move (%1)").arg(formatMove(move)), this);
        assignSelMoveAction->setStatusTip(_q("Assign move (%1) to the selected player(s)")
                        .arg(formatMove(move)));
        assignSelMoveAction->setShortcut(Qt::Key_Enter);
        QObject::connect(assignSelMoveAction, SIGNAL(triggered()),
                         this, SLOT(assignSelectedMove()));
        iMenu.addAction(assignSelMoveAction);
    }

    // Action to assign the top move
    QAction *assignTopMoveAction = new QAction(_q("Assign top move"), this);
    assignTopMoveAction->setStatusTip(_q("Assign the top move (if unique) to the selected player(s)"));
    assignTopMoveAction->setShortcut(Qt::Key_T);
    QObject::connect(assignTopMoveAction, SIGNAL(triggered()),
                     this, SLOT(assignTopMove()));
    iMenu.addAction(assignTopMoveAction);

    // Action to warn (or "unwarn") players
    QAction *warningAction = new QAction(_q("Give (or remove) a warning"), this);
    warningAction->setStatusTip(_q("Give a warning to the selected player(s), or remove it if they already have one"));
    warningAction->setShortcut(Qt::Key_W);
    QObject::connect(warningAction, SIGNAL(triggered()),
                     this, SLOT(addRemoveWarning()));
    iMenu.addAction(warningAction);

    // Action to give a penalty to players
    QAction *penaltyAction = new QAction(_q("Give a penalty"), this);
    penaltyAction->setStatusTip(_q("Give a penalty to the selected player(s)"));
    penaltyAction->setShortcut(Qt::Key_P);
    QObject::connect(penaltyAction, SIGNAL(triggered()),
                     this, SLOT(addPenalty()));
    iMenu.addAction(penaltyAction);
}


void ArbitAssignments::on_checkBoxHideAssigned_toggled(bool)
{
    updatePlayersModel();
}


void ArbitAssignments::selectedMoveChanged(const Move &iMove)
{
    m_selectedMove = iMove;
    enableAssignmentButtons();
}


bool ArbitAssignments::hasSelectedPlayer() const
{
    return treeViewPlayers->selectionModel()->hasSelection();
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
    if (move.getType() == Move::VALID_ROUND)
    {
        // TODO: deselect move in the Results?
        m_game->setTestRound(move.getRound());
        emit gameUpdated();
    }
}


void ArbitAssignments::assignMasterMove()
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

    const Move &move = m_selectedMove;
    if (move.getType() != Move::VALID_ROUND)
    {
        notifyProblem(_q("The master move must be a valid move."));
        return;
    }

    // Warn if the selected move is not a top move
    BestResults results;
    results.search(m_game->getDic(), m_game->getBoard(),
                   m_game->getCurrentRack().getRack(),
                   m_game->getHistory().beforeFirstRound());
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


void ArbitAssignments::assignDefaultMasterMove()
{
    const Move &currMove = m_game->duplicateGetMasterMove();
    // Do not overwrite an existing move
    if (currMove.getType() != Move::NO_MOVE)
        return;

    // Search the best moves
    BestResults results;
    results.search(m_game->getDic(), m_game->getBoard(),
                   m_game->getCurrentRack().getRack(),
                   m_game->getHistory().beforeFirstRound());
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


void ArbitAssignments::assignSelectedMove()
{
    if (m_selectedMove.getType() == Move::NO_MOVE ||
        !treeViewPlayers->selectionModel()->hasSelection())
    {
        return;
    }
    helperAssignMove(m_selectedMove);
}


void ArbitAssignments::assignTopMove()
{
    BestResults results;
    results.search(m_game->getDic(), m_game->getBoard(),
                   m_game->getCurrentRack().getRack(),
                   m_game->getHistory().beforeFirstRound());
    // TODO: what if there are several moves?
    if (results.size() == 1)
        helperAssignMove(Move(results.get(0)));
}


void ArbitAssignments::assignNoMove()
{
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
            m_game->getPlayer(id).getLastMove().getType() != Move::NO_MOVE)
        {
            assignedIdSet.insert(id);
        }
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


void ArbitAssignments::addPenalty()
{
    QSet<unsigned int> playersIdSet = getSelectedPlayers();
    if (playersIdSet.isEmpty())
        return;

    BOOST_FOREACH(unsigned int id, playersIdSet)
    {
        m_game->arbitrationAddPenalty(id, 0);
    }
    emit gameUpdated();
}


QString ArbitAssignments::formatMove(const Move &iMove) const
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


void ArbitAssignments::endTurn()
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

    emit notifyInfo(_q("New turn started"));

    m_game->removeTestRound();
    m_game->arbitrationFinalizeTurn();
    // FIXME: shouldn't be done here
    setEnabled(!m_game->isFinished());

    emit gameUpdated();
}

