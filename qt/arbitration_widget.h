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

#ifndef ARBITRATION_WIDGET_H_
#define ARBITRATION_WIDGET_H_

#include <QtGui/QWidget>

#include <ui/arbitration_widget.ui.h>
#include "results.h"
#include "move.h"
#include "logging.h"

class PublicGame;
class CoordModel;
class CustomPopup;
class KeyAccumulator;
class QStandardItemModel;
class QSortFilterProxyModel;
class QMenu;
class QPoint;
class QKeyEvent;

class ArbitrationWidget: public QWidget, private Ui::ArbitrationWidget
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    ArbitrationWidget(QWidget *parent,
                      PublicGame *iGame,
                      CoordModel &iCoordModel);

signals:
    void gameUpdated();
    void notifyProblem(QString iMsg);
    void notifyInfo(QString iMsg);
    void rackUpdated(const QString &iRack);
    void requestDefinition(QString iWord);

public slots:
    void refresh();

private slots:
    void setRackRandom();
    void rackEdited(const QString &);
    void on_buttonSearch_clicked();
    void on_checkBoxHideAssigned_toggled(bool);
    void resultsFilterChanged(const QString &);
    void enableCheckWordButton();
    void enableAssignmentButtons();
    void clearResults();
    void showPreview(const QItemSelection &);
    void showMasterPreview();
    void updateCoordText(const Coord&, const Coord&);
    void updateCoordModel(const QString&);
    void populateResultsMenu(QMenu &iMenu, const QPoint &iPoint);
    void populatePlayersMenu(QMenu &iMenu, const QPoint &iPoint);
    void checkWord();
    void selectTableNumber(int key);
    void assignMasterMove();
    void assignDefaultMasterMove();
    void assignSelectedMove();
    void assignTopMove();
    void assignNoMove();
    void endTurn();

private:
    /// Encapsulated game, can be NULL
    PublicGame *m_game;

    /// Coordinates of the next word to play
    CoordModel &m_coordModel;

    /// Search results
    LimitResults m_results;

    /// Model for the search results
    QStandardItemModel *m_resultsModel;
    /// Proxy for the results model
    QSortFilterProxyModel *m_proxyResultsModel;

    /// Model for the players list
    QStandardItemModel *m_playersModel;
    /// Proxy for the players model
    QSortFilterProxyModel *m_proxyPlayersModel;

    /// Popup menu for the search results
    CustomPopup *m_resultsPopup;

    /// Container for the moves manually entered in the interface
    QVector<Move> m_addedMoves;

    /// Accumulator used to build the table number
    KeyAccumulator *m_keyAccum;

    /// Palette to write text in black
    QPalette blackPalette;

    /// Palette to write text in red
    QPalette redPalette;

    /// Force synchronizing the model with the search results
    void updateResultsModel();
    /// Force synchronizing the model with the players
    void updatePlayersModel();

    /**
     * Add the given move to the results list.
     * Return the row number of the added item.
     */
    int addSingleMove(const Move &iMove, int moveType,
                      unsigned int index, int bestScore);

    /// Return the rack for the current turn
    // FIXME: this feature should be provided by the core
    Rack getRack() const;

    int getBestScore() const;

    /// Helper method to return a structured move for the selected result
    Move getSelectedMove() const;
    /// Helper method to return the ID of the selected player(s)
    QSet<unsigned int> getSelectedPlayers() const;

    /// Helper method to assign the given move to the selected player(s)
    void helperAssignMove(const Move &iMove);

    /// Format the given move under the format WORD - Ref - Pts
    QString formatMove(const Move &iMove) const;
};

#endif

