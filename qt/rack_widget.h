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

#include <QtGui/QFrame>

#include "tile.h"
#include "logging.h"

class PublicGame;
class TileWidget;


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

public slots:
    void setGame(const PublicGame *iGame);
    void refresh();

private:
    /// Encapsulated tiles
    vector<TileWidget *> m_tilesVect;

    /// Encapsulated game, can be NULL
    const PublicGame *m_game;

    /**
     * Indicate whether to show only the last rack
     * (useful for on the external board, in particular in arbitration mode)
     */
    bool m_showOnlyLastTurn;
};

#endif

