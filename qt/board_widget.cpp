/*****************************************************************************
 * Eliot
 * Copyright (C) 2008-2009 Olivier Teulière
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
#include <math.h>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QMouseEvent>
#include <QtCore/QSettings>

#include "board_widget.h"
#include "prefs_dialog.h"
#include "qtcommon.h"
#include "public_game.h"
#include "tile.h"
#include "board.h"
#include "coord_model.h"

using namespace std;


const QColor BoardWidget::EmptyColour(Qt::white);
const QColor BoardWidget::L2Colour(34, 189, 240);
const QColor BoardWidget::L3Colour(29, 104, 240);
const QColor BoardWidget::W2Colour(255, 147, 196);
const QColor BoardWidget::W3Colour(240, 80, 94);
const QColor BoardWidget::TileColour(255, 235, 205);
const QColor BoardWidget::PreviewColour(183, 183, 123);
const QColor BoardWidget::NormalColour(0, 0, 0);
const QColor BoardWidget::JokerColour(255, 0, 0);
const QColor BoardWidget::ArrowColour(10, 10, 10);


BoardWidget::BoardWidget(CoordModel &iCoordModel, QWidget *parent)
    : QFrame(parent), m_game(NULL), m_coordModel(iCoordModel)
{
    setFrameStyle(QFrame::Panel);
    // Use as much space as possible
    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setSizePolicy(policy);
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
    const int size = std::min(width(), height());
    const int squareSize = (int)floor((size - 1) / (BOARD_MAX - BOARD_MIN + 2));

    // The font must grow with the square size
    QFont letterFont = font();
    letterFont.setPixelSize(squareSize * 2 / 3);

    QFont pointsFont = font();
    const double pointsCoeff = 8. / 25.;
    pointsFont.setPixelSize(squareSize * pointsCoeff);

    // Should we display the tiles points?
    QSettings qs(ORGANIZATION, PACKAGE_NAME);
    bool showPoints = qs.value(PrefsDialog::kINTF_SHOW_TILES_POINTS, true).toBool();

    // XXX: Naive implementation: we repaint everything every time
    QPainter painter(this);
    //painter.setPen(Qt::NoPen);
    for (unsigned int row = BOARD_MIN; row <= BOARD_MAX; ++row)
    {
        for (unsigned int col = BOARD_MIN; col <= BOARD_MAX; ++col)
        {
            const unsigned int xPos = (col - BOARD_MIN + 1) * squareSize;
            const unsigned int yPos = (row - BOARD_MIN + 1) * squareSize;

            // Set the brush color
            if (m_game != NULL && !m_game->getBoard().getTile(row, col).isEmpty())
            {
                if (m_game->getBoard().getCharAttr(row, col) & ATTR_TEST)
                    painter.setBrush(PreviewColour);
                else
                    painter.setBrush(TileColour);
            }
            else if (Board::GetWordMultiplier(row, col) == 3)
                painter.setBrush(W3Colour);
            else if (Board::GetWordMultiplier(row, col) == 2)
                painter.setBrush(W2Colour);
            else if (Board::GetLetterMultiplier(row, col) == 3)
                painter.setBrush(L3Colour);
            else if (Board::GetLetterMultiplier(row, col) == 2)
                painter.setBrush(L2Colour);
            else
                painter.setBrush(EmptyColour);
            painter.drawRect(xPos, yPos, squareSize, squareSize);

            // Draw the letter
            if (m_game != NULL && !m_game->getBoard().getTile(row, col).isEmpty())
            {
                wstring chr = m_game->getBoard().getTile(row, col).getDisplayStr();
                // Make the display char in upper case
                std::transform(chr.begin(), chr.end(), chr.begin(), towupper);
                if (m_game->getBoard().getCharAttr(row, col) & ATTR_JOKER)
                    painter.setPen(JokerColour);
                painter.setFont(letterFont);
                painter.drawText(xPos, yPos + 1, squareSize, squareSize,
                                 Qt::AlignCenter, qfw(chr));
                painter.setPen(NormalColour);

                // Draw the points of the tile
                if (showPoints &&
                    !m_game->getBoard().getCharAttr(row, col) & ATTR_JOKER)
                {
                    painter.setFont(pointsFont);
                    painter.drawText(xPos + squareSize * (1 - pointsCoeff),
                                     yPos + squareSize * (1 - pointsCoeff) + 1,
                                     squareSize * pointsCoeff, squareSize * pointsCoeff + 3,
                                     Qt::AlignRight | Qt::AlignBottom,
                                     QString("%1").arg(m_game->getBoard().getTile(row, col).getPoints()));
                }
            }
        }
    }
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
}


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
        const int squareSize = (int)floor((size - 1) / (BOARD_MAX - BOARD_MIN + 2));
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
        const int squareSize = (int)floor((size - 1) / (BOARD_MAX - BOARD_MIN + 2));
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
    const int squareSize = (int)floor((size - 1) / (BOARD_MAX - BOARD_MIN + 2));
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

