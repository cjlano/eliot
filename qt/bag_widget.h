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

#ifndef BAG_WIDGET_H_
#define BAG_WIDGET_H_

#include <vector>
#include <QtGui/QWidget>

#include <ui/bag_widget.ui.h>
#include "logging.h"

using std::vector;

class QStandardItemModel;
class PublicGame;
class TileWidget;


class BagWidget: public QWidget, private Ui::BagWidget
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    explicit BagWidget(QWidget *parent = 0);

public slots:
    void setGame(const PublicGame *iGame);
    void refresh();

protected:
    /// Define a default size
    virtual QSize sizeHint() const;

private:
    /// Encapsulated game, can be NULL
    const PublicGame *m_game;

//QTreeView *m_treeView;

    /// Model of the bag
    QStandardItemModel *m_model;

    /// Force synchronizing the model with the contents of the bag
    void updateModel();
};


class BagWidget2: public QWidget
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    explicit BagWidget2(QWidget *parent = 0);

public slots:
    void setGame(const PublicGame *iGame);
    void setShowPlayedTiles(bool iShow);
    void setShowTilesInRack(bool iShow);
    void refresh();

protected:
    /// Define a default size
    virtual QSize sizeHint() const;

private:
    /// Encapsulated game, can be NULL
    const PublicGame *m_game;

    /// Encapsulated tiles
    vector<TileWidget *> m_tilesVect;

    /// Cached value, storing the total number of tiles in the game
    unsigned int m_totalNbTiles;

    /// Indicate whether the played tiles should be displayed
    bool m_showPlayedTiles;

    /// Indicate whether the tiles in the current rack should be displayed highlighted
    bool m_showTilesInRack;

};

#endif

