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

#include "tile_layout.h"
#include "tile_widget.h"

using namespace std;


TileLayout::TileLayout(int nbCols, int spacing)
    : m_nbCols(nbCols), m_space(spacing)
{
    setContentsMargins(0, 0, 0, 0);
}


TileLayout::~TileLayout()
{
    QLayoutItem *item;
    while ((item = takeAt(0)))
        delete item;
}


QRect TileLayout::getBoardRect() const
{
    if (m_items.size() < m_nbCols + 2)
        return QRect();
    return m_items.at(m_nbCols + 1)->geometry().united(m_items.back()->geometry());
}


int TileLayout::getSquareSize() const
{
    if (m_items.empty())
        return 0;
    return m_items.at(0)->geometry().width();
}


QLayoutItem *TileLayout::takeAt(int index)
{
    if (index >= 0 && index < m_items.size())
        return m_items.takeAt(index);
    else
        return 0;
}


QSize TileLayout::minimumSize() const
{
    QSize size(m_space, m_space);
    if (!m_items.empty())
        size += m_items.at(0)->minimumSize();
    return size * m_nbCols + QSize(5, 5);
}


void TileLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect);
}


void TileLayout::doLayout(const QRect &rect)
{
    int size = std::min(rect.width(), rect.height());
    int squareSize = size / m_nbCols - m_space;
    QLayoutItem *item;
    int x = 0;
    int y = 0;
    int nbInRow = 1;
    foreach (item, m_items)
    {
        QRect itemRect(QPoint(x, y), QSize(squareSize, squareSize));
        item->setGeometry(itemRect);
        x += squareSize + m_space;
        ++nbInRow;
        if (nbInRow > m_nbCols)
        {
            x = 0;
            y += squareSize + m_space;
            nbInRow = 1;
        }
    }
}

