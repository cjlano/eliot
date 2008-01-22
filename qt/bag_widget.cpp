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

#include <vector>
#include <QtGui/QTreeView>
#include <QtGui/QStandardItemModel>

#include "bag_widget.h"
#include "qtcommon.h"
#include "game.h"
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
    updateModel();
}


void BagWidget::setGame(const Game *iGame)
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
    m_model->clear();
    m_model->setColumnCount(2);
    m_model->setHeaderData(0, Qt::Horizontal, _q("Letter"), Qt::DisplayRole);
    m_model->setHeaderData(1, Qt::Horizontal, _q("Points"), Qt::DisplayRole);

    if (m_game == NULL)
        return;

    const vector<Tile>& allTiles = m_game->getDic().getAllTiles();
    vector<Tile>::const_iterator it;
    for (it = allTiles.begin(); it != allTiles.end(); ++it)
    {
        unsigned int nb = m_game->getBag().in(*it);
        if (nb != 0)
        {
            int rowNum = m_model->rowCount();
            m_model->insertRow(rowNum);
            m_model->setData(m_model->index(rowNum, 0),
                             qfw(wstring(nb, it->toChar())));
            m_model->setData(m_model->index(rowNum, 1), it->getPoints());
        }
    }
    //resizeColumnToContents(0);
    resizeColumnToContents(1);
}


QSize BagWidget::sizeHint() const
{
    return QSize(160, 300);
}

