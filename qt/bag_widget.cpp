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

#include <boost/foreach.hpp>
#include <vector>
#include <QTreeView>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QAction>

#include "bag_widget.h"
#include "tile_layout.h"
#include "tile_widget.h"
#include "qtcommon.h"
#include "public_game.h"
#include "dic.h"
#include "tile.h"
#include "bag.h"
#include "rack.h"
#include "player.h"
#include "debug.h"

using namespace std;


INIT_LOGGER(qt, BagWidget);
INIT_LOGGER(qt, BagWidget2);


BagWidget::BagWidget(QWidget *parent)
    : QWidget(parent), m_game(NULL)
{
    setupUi(this);

    // Associate the model to the view
    m_model = new QStandardItemModel(this);
    treeView->setModel(m_model);
    m_model->setColumnCount(2);
    m_model->setHeaderData(0, Qt::Horizontal, _q("Letter"), Qt::DisplayRole);
    m_model->setHeaderData(1, Qt::Horizontal, _q("Points"), Qt::DisplayRole);
    updateModel();

    treeView->setColumnWidth(0, 90);
    treeView->setColumnWidth(1, 10);
}


void BagWidget::setGame(const PublicGame *iGame)
{
    m_game = iGame;
    updateModel();

    treeView->resizeColumnToContents(0);
    treeView->resizeColumnToContents(1);
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
        unsigned int nb = bag.count(tile);
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

    labelVowels->setText(QString("%1").arg(bag.getNbVowels()));
    labelConsonants->setText(QString("%1").arg(bag.getNbConsonants()));
    labelJokers->setText(QString("%1").arg(bag.count(Tile::Joker())));
}


QSize BagWidget::sizeHint() const
{
    return QSize(160, 300);
}



BagWidget2::BagWidget2(QWidget *parent)
    : QWidget(parent), m_game(NULL), m_totalNbTiles(0),
    m_showPlayedTiles(true), m_showTilesInRack(true)
{
    TileLayout *layout = new TileLayout;
    layout->setSpacing(5);
    setLayout(layout);

    // Define actions for the context menu
    QAction *actionPlayed = new QAction(_q("Show played tiles"), this);
    actionPlayed->setCheckable(true);
    actionPlayed->setChecked(true);
    addAction(actionPlayed);
    QObject::connect(actionPlayed, SIGNAL(toggled(bool)),
                     this, SLOT(setShowPlayedTiles(bool)));

    QAction *actionInRack = new QAction(_q("Highlight tiles present in the rack"), this);
    actionInRack->setCheckable(true);
    actionInRack->setChecked(true);
    addAction(actionInRack);
    QObject::connect(actionInRack, SIGNAL(toggled(bool)),
                     this, SLOT(setShowTilesInRack(bool)));

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

    const Rack &rack = m_game->getCurrentRack().getRack();
    TileWidget::State previewState =
        m_showTilesInRack ? TileWidget::PREVIEW : TileWidget::NORMAL;

    // Set the correct content for all the tiles
    unsigned int index = 0;
    BOOST_FOREACH(const Tile &tile, m_game->getDic().getAllTiles())
    {
        const unsigned int nbInBag = bag.count(tile);
        const unsigned int nbInRack = rack.count(tile);
        ASSERT(nbInBag >= nbInRack, "Unexpected letters in the rack");
        for (unsigned i = 0; i < nbInBag - nbInRack; ++i)
        {
            m_tilesVect[index]->tileChanged(TileWidget::NORMAL, tile);
            ++index;
        }
        for (unsigned i = 0; i < nbInRack; ++i)
        {
            m_tilesVect[index]->tileChanged(previewState, tile);
            ++index;
        }
        if (m_showPlayedTiles)
        {
            for (unsigned i = nbInBag; i < tile.maxNumber(); ++i)
            {
                m_tilesVect[index]->tileChanged(TileWidget::RACK_PLAYED, tile);
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


void BagWidget2::setShowTilesInRack(bool iShow)
{
    if (m_showTilesInRack == iShow)
        return;
    m_showTilesInRack = iShow;
    refresh();
}


QSize BagWidget2::sizeHint() const
{
    return QSize(160, 300);
}

