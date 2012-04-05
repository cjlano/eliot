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

#ifndef ARBIT_ASSIGNMENTS_H_
#define ARBIT_ASSIGNMENTS_H_

#include <QtGui/QWidget>

#include <ui/arbit_assignments.ui.h>
#include "move.h"
#include "logging.h"

class PublicGame;
class QStandardItemModel;
class QSortFilterProxyModel;
class QMenu;
class QPoint;
class QString;

class ArbitAssignments: public QWidget, private Ui::ArbitAssignments
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    ArbitAssignments(QWidget *parent, PublicGame *iGame);

    /// Format the given move under the format WORD - Ref - Pts
    QString formatMove(const Move &iMove) const;

    /**
     * Select the player with the given table number.
     * Return true if the player was found, false otherwise.
     * If the player is found, the oName parameter is filled
     * with the player name.
     */
    bool selectPlayerByTable(unsigned tabNb, QString *oName);

    bool hasSelectedPlayer() const;

signals:
    void gameUpdated();
    void notifyProblem(QString iMsg);
    void notifyInfo(QString iMsg);
    void endOfTurn();

public slots:
    void refresh();
    void enableAssignmentButtons();
    void selectAllPlayers();
    void assignMasterMove();
    void assignSelectedMove();
    void assignDefaultMasterMove();
    void selectedMoveChanged(const Move&);

private slots:
    void on_checkBoxHideAssigned_toggled(bool);
    void showMasterPreview();
    void populatePlayersMenu(QMenu &iMenu, const QPoint &iPoint);
    void assignTopMove();
    void suppressMove();
    void addRemoveWarning();
    void addPenalty();
    void endTurn();

private:
    /// Encapsulated game, can be NULL
    PublicGame *m_game;

    /// Model for the players list
    QStandardItemModel *m_playersModel;
    /// Proxy for the players model
    QSortFilterProxyModel *m_proxyPlayersModel;

    // Move currently selected (in the Results table)
    Move m_selectedMove;

    /// Force synchronizing the model with the players
    void updatePlayersModel();

    /// Helper method to return the ID of the selected player(s)
    QSet<unsigned int> getSelectedPlayers() const;

    /// Helper method to assign the given move to the selected player(s)
    void helperAssignMove(const Move &iMove);
};

#endif

