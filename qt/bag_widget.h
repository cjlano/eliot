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

#ifndef BAG_WIDGET_H_
#define BAG_WIDGET_H_

#include <QtGui/QTreeView>


class Game;
class QStandardItemModel;

class BagWidget: public QTreeView
{
    Q_OBJECT;

public:
    explicit BagWidget(QWidget *parent = 0);

public slots:
    void setGame(const Game *iGame);
    void refresh();

protected:
    /// Define a default size
    virtual QSize sizeHint() const;

private:
    /// Encapsulated game, can be NULL
    const Game *m_game;

    /// Model of the bag
    QStandardItemModel *m_model;

    /// Force synchronizing the model with the contents of the bag
    void updateModel();
};

#endif

