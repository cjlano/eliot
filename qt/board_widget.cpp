/*****************************************************************************
 * Eliot
 * Copyright (C) 2008-2010 Olivier Teulière
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

#include <algorithm> // For std::transform
#include <cmath>
//#include <QtGui/QPainter>
#include <QtGui/QGridLayout>
#include <QtGui/QMouseEvent>
// XXX
#include <QtGui/QTreeView>
#include <QtGui/QPainter>
#include <iostream>

#include "board_widget.h"
#include "tile_widget.h"
#include "qtcommon.h"
#include "public_game.h"
#include "tile.h"
#include "board.h"
#include "coord_model.h"

using namespace std;


class BoardLayout : public QLayout
{
    //Q_OBJECT

public:
    BoardLayout(int nbCols): m_nbCols(nbCols), m_space(0)
    {
        setContentsMargins(0, 0, 0, 0);
    }
    ~BoardLayout()
    {
        QLayoutItem *item;
        while ((item = takeAt(0)))
            delete item;
    }

    QRect getBoardRect() const
    {
        if (m_items.size() < m_nbCols + 2)
            return QRect();
        return m_items.at(m_nbCols + 1)->geometry().united(m_items.back()->geometry());
    }

    virtual void addItem(QLayoutItem *item)
    {
        m_items.append(item);
    }
    virtual bool hasHeightForWidth() const { return true; }
    virtual int heightForWidth(int width) const { return width; }
    virtual int count() const { return m_items.size(); }
    virtual QLayoutItem *itemAt(int index) const { return m_items.value(index); }
    virtual QLayoutItem *takeAt(int index)
    {
        if (index >= 0 && index < m_items.size())
            return m_items.takeAt(index);
        else
            return 0;
    }
    virtual QSize minimumSize() const
    {
        QSize size;
        if (!m_items.empty())
            size.expandedTo(m_items.at(0)->minimumSize());
        return size * m_nbCols;
    }
    virtual void setGeometry(const QRect &rect)
    {
        QLayout::setGeometry(rect);
        doLayout(rect);
    }
    virtual QSize sizeHint() const { return minimumSize(); }

private:
    QList<QLayoutItem *> m_items;
    int m_nbCols;
    int m_space;

    void doLayout(const QRect &rect)
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
};

BoardWidget::BoardWidget(CoordModel &iCoordModel, QWidget *parent)
    : QFrame(parent), m_game(NULL), m_coordModel(iCoordModel),
    m_widgetsMatrix(BOARD_MAX + 1, BOARD_MAX + 1, 0)
{
    // Try to have a black background... FIXME: not working well!
    QPalette pal = palette();
    for (int i = 0; i <= 19; ++i)
        pal.setColor((QPalette::ColorRole)i, Qt::black);
    setPalette(pal);
    setForegroundRole(QPalette::Window);
    setBackgroundRole(QPalette::Window);

    BoardLayout *layout = new BoardLayout(BOARD_MAX + 1);
    // Line full of coordinates
    layout->addWidget(new BasicTileWidget(this, ""));
    for (unsigned int col = BOARD_MIN; col <= BOARD_MAX; ++col)
    {
        BasicTileWidget *coordTile =
            new BasicTileWidget(this, QString("%1").arg(col));
        layout->addWidget(coordTile);
    }
    // Rest of the board
    for (unsigned int row = BOARD_MIN; row <= BOARD_MAX; ++row)
    {
        // Add the coordinate
        BasicTileWidget *coordTile =
            new BasicTileWidget(this, QString(QChar('A' + row - BOARD_MIN)));
        layout->addWidget(coordTile);
        // Add the squares
        for (unsigned int col = BOARD_MIN; col <= BOARD_MAX; ++col)
        {
            TileWidget::Multiplier mult = TileWidget::NONE;
            if (Board::GetWordMultiplier(row, col) == 3)
                mult = TileWidget::WORD_TRIPLE;
            else if (Board::GetWordMultiplier(row, col) == 2)
                mult = TileWidget::WORD_DOUBLE;
            else if (Board::GetLetterMultiplier(row, col) == 3)
                mult = TileWidget::LETTER_TRIPLE;
            else if (Board::GetLetterMultiplier(row, col) == 2)
                mult = TileWidget::LETTER_DOUBLE;
            TileWidget *t = new TileWidget(this, mult, row, col);
            m_widgetsMatrix[row][col] = t;
            layout->addWidget(t);
            // Listen to mouse events on the tile
            connect(t, SIGNAL(mousePressed(int, int, QMouseEvent*)),
                    this, SLOT(tileClicked(int, int, QMouseEvent*)));
        }
    }

    setLayout(layout);

    setFrameStyle(QFrame::Panel);
    // Use as much space as possible
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setMinimumSize(200, 200);

    // Listen to changes in the coordinates
    QObject::connect(&m_coordModel, SIGNAL(coordChanged(const Coord&, const Coord&)),
                     this, SLOT(updateArrow(const Coord&, const Coord&)));
}


void BoardWidget::setGame(const PublicGame *iGame)
{
    m_game = iGame;
    refresh();
}


void BoardWidget::updateArrow(const Coord &iOldCoord, const Coord &iNewCoord)
{
    // Refresh only the 2 involved squares
    if (iOldCoord.isValid())
    {
        TileWidget *t = m_widgetsMatrix[iOldCoord.getRow()][iOldCoord.getCol()];
        t->arrowChanged(false, false);
    }
    if (iNewCoord.isValid())
    {
        TileWidget *t = m_widgetsMatrix[iNewCoord.getRow()][iNewCoord.getCol()];
        t->arrowChanged(iNewCoord.isValid(), iNewCoord.getDir() == Coord::VERTICAL);
    }
}


void BoardWidget::refresh()
{
    if (m_game != NULL)
    {
        // XXX: in the future, this code could be changed to use signals
        // emitted from the core. This would allow repainting only the needed
        // tiles (the same performance improvement could be done with caching
        // in the TileWidget class, though)
        const Board &board = m_game->getBoard();
        for (unsigned int row = BOARD_MIN; row <= BOARD_MAX; ++row)
        {
            for (unsigned int col = BOARD_MIN; col <= BOARD_MAX; ++col)
            {
                m_widgetsMatrix[row][col]->tileChanged(
                        board.getTile(row, col),
                        board.isJoker(row, col),
                        board.isTestChar(row, col));
            }
        }
    }
    update();
}


QSize BoardWidget::sizeHint() const
{
    return QSize(400, 400);
}


void BoardWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QRect rect = ((BoardLayout*)layout())->getBoardRect();
    painter.drawRect(rect);
    painter.drawRect(rect.adjusted(-1, -1, 1, 1));
}



void BoardWidget::tileClicked(int row, int col, QMouseEvent *iEvent)
{
    if (m_game == NULL)
    {
        m_coordModel.clear();
        return;
    }

#if 0
    // First version:
    //  - a left click toggles between horizontal and vertical arrows
    //  - a right click clears any arrow
    if (iEvent->button() == Qt::LeftButton)
    {
        // Change the direction if this is exactly the same as the current one
        Coord coord(row, col, Coord::HORIZONTAL);
        if (m_coordModel.getCoord() == coord)
            coord.setDir(Coord::VERTICAL);
        // Take into acount the new coordinates
        m_coordModel.setCoord(coord);
    }
    else if (iEvent->button() == Qt::RightButton)
    {
        // On a right click anywhere on the board, remove the arrow
        m_coordModel.clear();
    }
#endif
#if 1
    // Second version:
    //  - a left click cycles between horizontal arrow, vertical arrow and no arrow
    //  - a right click clears any arrow
    if (iEvent->button() == Qt::LeftButton)
    {
        // Change the direction if this is exactly the same as the current one
        Coord coord(row, col, Coord::HORIZONTAL);
        if (m_coordModel.getCoord().getRow() == coord.getRow() &&
            m_coordModel.getCoord().getCol() == coord.getCol())
        {
            if (m_coordModel.getCoord().getDir() == Coord::VERTICAL)
            {
                // Third click: clear the arrow
                m_coordModel.clear();
                return;
            }
            coord.setDir(Coord::VERTICAL);
        }
        // Take into acount the new coordinates
        m_coordModel.setCoord(coord);
    }
    else if (iEvent->button() == Qt::RightButton)
    {
        // On a right click anywhere on the board, remove the arrow
        m_coordModel.clear();
    }
#endif
#if 0
    // Third version:
    //  - a left click toggles between horizontal arrow and no arrow
    //  - a right click toggles between vertical arrow and no arrow
    if (iEvent->button() == Qt::LeftButton)
    {
        Coord coord(row, col, Coord::HORIZONTAL);
        // Remove the coordinates if they are exactly the same as the current ones,
        // otherwise set the coordinates;
        if (m_coordModel.getCoord() == coord)
            m_coordModel.clear();
        else
            m_coordModel.setCoord(coord);
    }
    else if (iEvent->button() == Qt::RightButton)
    {
        Coord coord(row, col, Coord::VERTICAL);
        // Remove the coordinates if they are exactly the same as the current ones,
        // otherwise set the coordinates;
        if (m_coordModel.getCoord() == coord)
            m_coordModel.clear();
        else
            m_coordModel.setCoord(coord);
    }
    else
        m_coordModel.clear();
#endif
}


