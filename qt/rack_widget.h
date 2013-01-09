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

#ifndef RACK_WIDGET_H_
#define RACK_WIDGET_H_

#include <vector>
#include <QtGui/QFrame>

#include "tile.h"
#include "logging.h"

using std::vector;


class PublicGame;
class TileWidget;
class PlayedRack;
class PlayModel;


/**
 * Widget used to display a rack with big letters
 */
class RackWidget: public QFrame
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    explicit RackWidget(QWidget *parent = 0);
    void setShowOnlyLastTurn(bool iShow) { m_showOnlyLastTurn = iShow; }

    void setPlayModel(PlayModel *iPlayModel);

public slots:
    void setGame(const PublicGame *iGame);
    void setRack(const PlayedRack &iRack);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);

private slots:
    /**
     * Refresh the widget, using m_tiles as a reference.
     */
    void refresh();
    /// Set the tiles and refresh the widget
    void setTiles(const vector<Tile> &iTiles);

private:
    /// Tiles from which widgets are created
    vector<Tile> m_tiles;

    /// Tiles after filtering (removing from m_tiles the ones in the word being played)
    vector<Tile> m_filteredTiles;

    /**
     * Encapsulated tiles.
     * Always in sync with m_tiles, except maybe during a drag & drop operation.
     */
    vector<TileWidget *> m_tilesVect;

    /// Encapsulated game, can be NULL
    const PublicGame *m_game;

    /// Word being played
    PlayModel *m_playModel;

    /**
     * Indicate whether to show only the last rack
     * (useful on the external board, in particular in arbitration mode)
     */
    bool m_showOnlyLastTurn;

    int m_dragOrigin;

    /**
     * If the play model is not NULL and contains a valid move, this method
     * returns a vector of tiles filtered from the tiles in the move.
     * Otherwise, the given tiles are returned without any change.
     */
    vector<Tile> filterRack(const vector<Tile> &iTiles) const;

    /**
     * Return the 0-based index of the tile found at the given (relative)
     * position. If there is no such tile, return -1.
     */
    int findTile(const QPoint &iPos) const;

    int findClosestTile(const QPoint &iPos) const;

    void moveTile(int fromPos, int toPos, bool shaded = false);

private slots:
    void tilePressed(int row, int col, QMouseEvent *event);
};

#endif

