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
    : QWidget(parent), m_game(NULL)
{
    TileLayout *layout = new TileLayout;
    layout->setSpacing(5);
    setLayout(layout);
}


void BagWidget2::setGame(const PublicGame *iGame)
{
    bool needToPopulate = (m_game != iGame && iGame != NULL);
    m_game = iGame;

    if (needToPopulate)
    {
        TileLayout *layout = (TileLayout*) this->layout();
        layout->clear();

        BOOST_FOREACH(const Tile &tile, m_game->getDic().getAllTiles())
        {
            for (unsigned i = 0; i < tile.maxNumber(); ++i)
            {
                TileWidget *tileWidget = new TileWidget(NULL, TileWidget::NONE, 0, 0);
                tileWidget->setBorder();
                tileWidget->tileChanged(TileWidget::NORMAL, tile);
                layout->addWidget(tileWidget);
            }
        }
    }

    refresh();
}


void BagWidget2::refresh()
{
    if (m_game == NULL)
        return;

    int index = 0;
    const Bag &bag = m_game->getBag();
    BOOST_FOREACH(const Tile &tile, m_game->getDic().getAllTiles())
    {
        unsigned int nbInBag = bag.in(tile);
        for (unsigned i = 0; i < nbInBag; ++i)
        {
            TileWidget *tileWidget = (TileWidget*) layout()->itemAt(index)->widget();
            tileWidget->tileChanged(TileWidget::NORMAL, tile);
            ++index;
        }
        for (unsigned i = nbInBag; i < tile.maxNumber(); ++i)
        {
            TileWidget *tileWidget = (TileWidget*) layout()->itemAt(index)->widget();
            tileWidget->tileChanged(TileWidget::RACK_PLAYED, tile);
            ++index;
        }
    }
}


QSize BagWidget2::sizeHint() const
{
    return QSize(160, 300);
}

