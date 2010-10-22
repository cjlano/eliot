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

#ifndef TILE_LAYOUT_H_
#define TILE_LAYOUT_H_

#include <QtGui/QLayout>
//#include <QtGui/QList>


class TileLayout : public QLayout
{
    Q_OBJECT

public:
    TileLayout(int nbCols, int spacing);
    virtual ~TileLayout();

    QRect getBoardRect() const;

    int getSquareSize() const;

    int getSpacing() const { return m_space; }

    virtual void addItem(QLayoutItem *item) { m_items.append(item); }
    virtual bool hasHeightForWidth() const { return true; }
    virtual int heightForWidth(int width) const { return width; }
    virtual int count() const { return m_items.size(); }
    virtual QLayoutItem *itemAt(int index) const { return m_items.value(index); }
    virtual QLayoutItem *takeAt(int index);
    virtual QSize minimumSize() const;
    virtual void setGeometry(const QRect &rect);
    virtual QSize sizeHint() const { return minimumSize(); }

private:
    QList<QLayoutItem *> m_items;
    int m_nbCols;
    int m_space;

    void doLayout(const QRect &rect);
};

#endif

