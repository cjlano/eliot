/*****************************************************************************
 * Eliot
 * Copyright (C) 2008-2012 Olivier Teulière
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

#ifndef BOARD_WIDGET_H_
#define BOARD_WIDGET_H_

#include <QtGui/QFrame>

#include "matrix.h"
#include "logging.h"


class PublicGame;
class TileWidget;
class PlayModel;
class Coord;
class Move;

class BoardWidget: public QFrame
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    explicit BoardWidget(PlayModel &iPlayModel, QWidget *parent = 0);
    void setShowTempSigns(bool iShow) { m_showTemporarySigns = iShow; }
    void setShowOnlyLastTurn(bool iShow) { m_showOnlyLastTurn = iShow; }

public slots:
    void setGame(const PublicGame *iGame);
    void refresh();

protected:
    /// Define a default size
    virtual QSize sizeHint() const;
    /// Paint the board
    virtual void paintEvent(QPaintEvent *iEvent);
    /// Catch mouse clicks on the board
    //virtual void mousePressEvent(QMouseEvent *iEvent);

private slots:
    void tileClicked(int row, int col, QMouseEvent *iEvent);
    void updateArrow(const Coord &iNewCoord, const Coord &iOldCoord);
    void onMoveChanged(const Move &iMove);

private:
    /// Encapsulated game, can be NULL
    const PublicGame *m_game;

    /// Coordinates of the next word to play
    PlayModel &m_playModel;

    /// Indicate whether to draw the arrow and the previewed word on the board
    bool m_showTemporarySigns;

    /**
     * Indicate whether to show only the board as it is on the last turn
     * (useful for on the external board, in particular in arbitration mode)
     */
    bool m_showOnlyLastTurn;

    Matrix<TileWidget*> m_widgetsMatrix;
};

#endif

