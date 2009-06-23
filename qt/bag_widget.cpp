/*****************************************************************************
 * Eliot
 * Copyright (C) 2008 Olivier Teulière
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
#include <QtGui/QTreeView>
#include <QtGui/QStandardItemModel>

#include "bag_widget.h"
#include "qtcommon.h"
#include "public_game.h"
#include "dic.h"
#include "tile.h"
#include "bag.h"

using namespace std;


BagWidget::BagWidget(QWidget *parent)
    : QTreeView(parent), m_game(NULL)
{
    // Create the tree view
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    setSizePolicy(policy);

    // Associate the model to the view
    m_model = new QStandardItemModel(this);
    setModel(m_model);
    m_model->setColumnCount(2);
    m_model->setHeaderData(0, Qt::Horizontal, _q("Letter"), Qt::DisplayRole);
    m_model->setHeaderData(1, Qt::Horizontal, _q("Points"), Qt::DisplayRole);
    updateModel();
}


void BagWidget::setGame(const PublicGame *iGame)
{
    m_game = iGame;
    updateModel();
}


void BagWidget::refresh()
{
    updateModel();
}


void BagWidget::updateModel()
{
    m_model->removeRows(0, m_model->rowCount());

    if (m_game == NULL)
        return;

    const Bag &bag = m_game->getBag();
    BOOST_FOREACH(const Tile &tile, m_game->getDic().getAllTiles())
    {
        unsigned int nb = bag.in(tile);
        if (nb != 0)
        {
            // Concatenate the display char nb times
            wdstring str;
            for (unsigned int i = 0; i < nb; ++i)
                str += tile.getDisplayStr();

            int rowNum = m_model->rowCount();
            m_model->insertRow(rowNum);
            m_model->setData(m_model->index(rowNum, 0), qfw(str));
            m_model->setData(m_model->index(rowNum, 1), tile.getPoints());
        }
    }
    //resizeColumnToContents(0);
    resizeColumnToContents(1);
}


QSize BagWidget::sizeHint() const
{
    return QSize(160, 300);
}

