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

    QRect getBoardRect()
    {

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
    : QFrame(parent), m_game(NULL), m_coordModel(iCoordModel)
{
    // Try to have a black background... FIXME: not working well!
    QPalette pal = palette();
    for (int i = 0; i <= 19; ++i)
        pal.setColor((QPalette::ColorRole)i, Qt::black);
    setPalette(pal);
    setForegroundRole(QPalette::Window);
    setBackgroundRole(QPalette::Window);

    BoardLayout *layout = new BoardLayout(15);
    for (unsigned int row = BOARD_MIN; row <= BOARD_MAX; ++row)
    {
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
            TileWidget *t = new TileWidget(this, mult);
            layout->addWidget(t);
        }
    }

    setLayout(layout);

    setFrameStyle(QFrame::Panel);
    // Use as much space as possible
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    setMinimumSize(200, 200);

    // Listen to changes in the coordinates
    QObject::connect(&m_coordModel, SIGNAL(coordChanged(const Coord&)),
                     this, SLOT(updateArrow(const Coord&)));
}


void BoardWidget::setGame(const PublicGame *iGame)
{
    m_game = iGame;
    refresh();
}


void BoardWidget::updateArrow(const Coord &)
{
    // Refresh everything
    // We could actually refresh only the 2 involved squares...
    refresh();
}


void BoardWidget::refresh()
{
    update();
}


QSize BoardWidget::sizeHint() const
{
    return QSize(400, 400);
}


void BoardWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

#if 0
    const int size = std::min(width(), height());
    const int squareSize = lrint(floor((size - 1) / (BOARD_MAX - BOARD_MIN + 2)));

    // The font must grow with the square size
    QFont letterFont = font();
    letterFont.setPixelSize(squareSize * 2 / 3);

    QFont pointsFont = font();
    const double pointsCoeff = 8. / 25.;
    pointsFont.setPixelSize(squareSize * pointsCoeff);

    // Draw the coordinates
    painter.setFont(letterFont);
    for (unsigned x = 1; x <= BOARD_MAX - BOARD_MIN + 1; ++x)
    {
        painter.drawText(x * squareSize, 1,
                         squareSize, squareSize,
                         Qt::AlignCenter,
                         QString::number(x));
        painter.drawText(0, x * squareSize,
                         squareSize, squareSize,
                         Qt::AlignCenter,
                         QString(1, 'A' + x - 1));
    }
    // Draw the arrow
    const Coord &markCoord = m_coordModel.getCoord();
    if (m_game != NULL && markCoord.isValid())
    {
        const unsigned int xPos = (markCoord.getCol() - BOARD_MIN + 1) * squareSize + 1;
        const unsigned int yPos = (markCoord.getRow() - BOARD_MIN + 1) * squareSize + 1;
        painter.setPen(QPen(ArrowColour, 0));
        painter.setBrush(ArrowColour);
        const int mid = squareSize / 2;
        const int fifth = squareSize / 5;
        const int width = squareSize / 16;
        painter.translate(xPos + mid, yPos + mid);
        if (markCoord.getDir() == Coord::VERTICAL)
            painter.rotate(90);
        const QPoint points[] =
        {
            QPoint(-mid + fifth, -width),
            QPoint(-mid + 3*fifth, -width),
            QPoint(-mid + 3*fifth, -fifth),
            QPoint(-mid + 4*fifth, 0),
            QPoint(-mid + 3*fifth, fifth),
            QPoint(-mid + 3*fifth, width),
            QPoint(-mid + fifth, width)
        };
        painter.drawPolygon(points, 7);

        painter.setPen(QPen());
        painter.setBrush(NormalColour);
    }
#endif
}


/*
void BoardWidget::mousePressEvent(QMouseEvent *iEvent)
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
        // Find the coordinates
        const int size = std::min(width(), height());
        const int squareSize = lrint(floor((size - 1) / (BOARD_MAX - BOARD_MIN + 2)));
        int row = iEvent->y() / squareSize;
        int col = iEvent->x() / squareSize;
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
        // Find the coordinates
        const int size = std::min(width(), height());
        const int squareSize = lrint(floor((size - 1) / (BOARD_MAX - BOARD_MIN + 2)));
        int row = iEvent->y() / squareSize;
        int col = iEvent->x() / squareSize;
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
    // Find the coordinates
    const int size = std::min(width(), height());
    const int squareSize = lrint(floor((size - 1) / (BOARD_MAX - BOARD_MIN + 2)));
    int row = iEvent->y() / squareSize;
    int col = iEvent->x() / squareSize;
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
*/

