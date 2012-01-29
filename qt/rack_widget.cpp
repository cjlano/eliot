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

#include <QtGui/QHBoxLayout>

#include "rack_widget.h"
#include "tile_widget.h"
#include "tile_layout.h"
#include "qtcommon.h"

#include "public_game.h"
#include "player.h"
#include "pldrack.h"
#include "debug.h"

using namespace std;

INIT_LOGGER(qt, RackWidget);


RackWidget::RackWidget(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    TileLayout *layout = new TileLayout(1);
    layout->setSpacing(5);
    setLayout(layout);
}


void RackWidget::setGame(const PublicGame *iGame)
{
    m_game = iGame;
    if (m_game == NULL)
    {
        // Delete the widgets
        TileLayout *layout = (TileLayout*) this->layout();
        layout->clear();
        m_tilesVect.clear();
    }
    refresh();
}


void RackWidget::refresh()
{
    if (m_game == NULL)
        return;

    // XXX: ugly (and wrong) way to get the rack
    vector<Tile> tiles;
    m_game->getCurrentPlayer().getCurrentRack().getAllTiles(tiles);

    // Make sure we have as many widgets as there are letters in the rack
    while (m_tilesVect.size() > tiles.size())
    {
        QtCommon::DestroyObject(m_tilesVect.back());
        m_tilesVect.pop_back();
    }
    while (m_tilesVect.size() < tiles.size())
    {
        TileWidget *tileWidget = new TileWidget;
        tileWidget->setBorder(2);
        layout()->addWidget(tileWidget);
        m_tilesVect.push_back(tileWidget);
    }
    ASSERT(m_tilesVect.size() == tiles.size(), "Invalid number of tiles");

    // Update the widgets
    for (unsigned int i = 0; i < tiles.size(); ++i)
    {
        TileWidget *tileWidget = m_tilesVect[i];
        tileWidget->tileChanged(TileWidget::NORMAL, tiles[i]);
    }
}


