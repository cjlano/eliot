/*****************************************************************************
 * Eliot
 * Copyright (C) 2010-2012 Olivier Teulière
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

#include <cmath>

#include "tile_layout.h"
#include "tile_widget.h"
#include "qtcommon.h"

using namespace std;

INIT_LOGGER(qt, TileLayout);


TileLayout::TileLayout(int nbRows, int nbCols)
    : m_dynamicRow(nbRows == 0), m_dynamicCol(nbCols == 0),
    m_nbCols(nbCols), m_nbRows(nbRows)
{
    setContentsMargins(0, 0, 0, 0);
}


TileLayout::~TileLayout()
{
    clear();
}


void TileLayout::clear()
{
    QLayoutItem *item;
    while ((item = takeAt(0)))
    {
        QtCommon::DestroyObject(item->widget());
        delete item;
    }
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
    const QSize marginsSize = geometry().size() - contentsRect().size();
    if (m_items.empty())
        return marginsSize;

    const QSize spacingSize(spacing(), spacing());
    const int size = spacing() + m_items.at(0)->minimumSize().width();
    if (m_dynamicCol && m_dynamicRow)
        return marginsSize - spacingSize + QSize(size, size);
    else if (m_dynamicCol)
        return marginsSize - spacingSize + QSize(size * ((m_items.size() - 1) / m_nbRows + 1), size * m_nbRows);
    else if (m_dynamicRow)
        return marginsSize - spacingSize + QSize(size * m_nbCols, size * ((m_items.size() - 1) / m_nbCols + 1));
    else
        return marginsSize - spacingSize + QSize(size * m_nbCols, size * m_nbRows);
}


QSize TileLayout::sizeHint() const
{
    const QSize marginsSize = geometry().size() - contentsRect().size();
    if (m_items.empty())
        return marginsSize;

    const QSize spacingSize(spacing(), spacing());
    const int size = spacing() + m_items.at(0)->sizeHint().width();
    if (m_dynamicCol && m_dynamicRow)
        return marginsSize - spacingSize + QSize(size, size);
    else if (m_dynamicCol)
        return marginsSize - spacingSize + QSize(size * ((m_items.size() - 1) / m_nbRows + 1), size * m_nbRows);
    else if (m_dynamicRow)
        return marginsSize - spacingSize + QSize(size * m_nbCols, size * ((m_items.size() - 1) / m_nbCols + 1));
    else
        return marginsSize - spacingSize + QSize(size * m_nbCols, size * m_nbRows);
}


void TileLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);

    if (m_items.isEmpty())
        return;

    // We use contentsRect() to take margins into account
    const int width = contentsRect().width() + spacing();
    const int height = contentsRect().height() + spacing();

    if (m_dynamicCol && m_dynamicRow)
    {
        // Dynamic number of columns. The square size is the biggest one
        // allowing to place all the tiles in the given rect without scrolling.
        // This determines the number of columns.
        const int tilesCount = m_items.size();
        int bestNbCols = 0;
        int bestSquareSize = 0;
        for (int nbCols = 1; nbCols <= tilesCount; ++nbCols)
        {
            const int nbRows = (tilesCount - 1) / nbCols + 1;
            int squareSize = std::min(width / nbCols, height / nbRows);
            if (squareSize >= bestSquareSize)
            {
                bestNbCols = nbCols;
                bestSquareSize = squareSize;
            }
            else
            {
                // Maximum reached
                break;
            }
        }

        if (bestNbCols == 0)
        {
            m_nbCols = (int) sqrt(tilesCount);
            m_nbRows = m_nbCols;
        }
        else
        {
            m_nbCols = bestNbCols;
            m_nbRows = (tilesCount - 1) / m_nbCols + 1;
        }
    }
    else if (m_dynamicCol)
    {
        m_nbCols = (m_items.size() - 1) / m_nbRows + 1;
    }
    else if (m_dynamicRow)
    {
        m_nbRows = (m_items.size() - 1) / m_nbCols + 1;
    }

    // Now the number of columns and rows are defined.
    // Use that to draw the tiles.
    const int squareSizeWithSpacing = std::min(width / m_nbCols, height / m_nbRows);
    const int squareSize = squareSizeWithSpacing - spacing();

    // Handle alignment
    int left = contentsRect().left();
    if (alignment() & Qt::AlignRight)
        left += width - m_nbCols * squareSizeWithSpacing;
    else if (alignment() & Qt::AlignHCenter)
        left += (width - m_nbCols * squareSizeWithSpacing) / 2;
    int top = contentsRect().top();
    if (alignment() & Qt::AlignBottom)
        top += height - m_nbRows * squareSizeWithSpacing;
    else if (alignment() & Qt::AlignVCenter)
        top += (height - m_nbRows * squareSizeWithSpacing) / 2;

    // Resize items
    int nbInRow = 1;
    QLayoutItem *item;
    int x = left;
    int y = top;
    foreach (item, m_items)
    {
        QRect itemRect(QPoint(x, y), QSize(squareSize, squareSize));
        item->setGeometry(itemRect);
        x += squareSizeWithSpacing;
        ++nbInRow;
        if (nbInRow > m_nbCols)
        {
            x = left;
            y += squareSizeWithSpacing;
            nbInRow = 1;
        }
    }
}

