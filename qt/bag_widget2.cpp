/*****************************************************************************
 * Eliot
 * Copyright (C) 2010 Olivier Teulière
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
#include <QtGui/QAction>

#include "bag_widget2.h"
#include "tile_layout.h"
#include "tile_widget.h"
#include "qtcommon.h"
#include "public_game.h"
#include "dic.h"
#include "tile.h"
#include "bag.h"

using namespace std;


BagWidget2::BagWidget2(QWidget *parent)
    : QWidget(parent), m_game(NULL), m_totalNbTiles(0)
{
    TileLayout *layout = new TileLayout;
    layout->setSpacing(5);
    setLayout(layout);
    m_showPlayedTiles = true;

    // Define an action for the context menu
    QAction *action = new QAction(_q("Show played tiles"), this);
    action->setCheckable(true);
    action->setChecked(true);
    addAction(action);
    QObject::connect(action, SIGNAL(toggled(bool)),
                     this, SLOT(setShowPlayedTiles(bool)));

    setContextMenuPolicy(Qt::ActionsContextMenu);
}


void BagWidget2::setGame(const PublicGame *iGame)
{
    if (iGame == m_game)
        return;

    m_game = iGame;

    TileLayout *layout = (TileLayout*) this->layout();
    layout->clear();
    m_tilesVect.clear();

    if (m_game != NULL)
    {
        m_totalNbTiles = 0;
        BOOST_FOREACH(const Tile &tile, m_game->getDic().getAllTiles())
        {
            m_totalNbTiles += tile.maxNumber();
        }
    }

    refresh();
}


void BagWidget2::refresh()
{
    if (m_game == NULL)
        return;

    const Bag &bag = m_game->getBag();
    unsigned int nbTilesToDisplay =
        m_showPlayedTiles ? m_totalNbTiles : bag.getNbTiles();

    // Ensure a correct number of widgets
    while (m_tilesVect.size() > nbTilesToDisplay)
    {
        // Extra tile: we remove it
        QtCommon::DestroyObject(m_tilesVect.back());
        m_tilesVect.pop_back();
    }
    while (m_tilesVect.size() < nbTilesToDisplay)
    {
        // Missing tile: we add it
        TileWidget *tileWidget = new TileWidget;
        tileWidget->setBorder();
        layout()->addWidget(tileWidget);
        m_tilesVect.push_back(tileWidget);
    }

    // Set the correct content for all the tiles
    unsigned int index = 0;
    BOOST_FOREACH(const Tile &tile, m_game->getDic().getAllTiles())
    {
        unsigned int nbInBag = bag.in(tile);
        for (unsigned i = 0; i < nbInBag; ++i)
        {
            TileWidget *tileWidget = m_tilesVect[index];
            tileWidget->tileChanged(TileWidget::NORMAL, tile);
            ++index;
        }
        if (m_showPlayedTiles)
        {
            for (unsigned i = nbInBag; i < tile.maxNumber(); ++i)
            {
                TileWidget *tileWidget = m_tilesVect[index];
                tileWidget->tileChanged(TileWidget::RACK_PLAYED, tile);
                ++index;
            }
        }
    }
}


void BagWidget2::setShowPlayedTiles(bool iShow)
{
    if (m_showPlayedTiles == iShow)
        return;
    m_showPlayedTiles = iShow;
    refresh();
}


QSize BagWidget2::sizeHint() const
{
    return QSize(160, 300);
}

