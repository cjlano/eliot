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

#include <algorithm> // For std::transform
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtCore/QSettings>

#include "tile_widget.h"
#include "prefs_dialog.h"
#include "qtcommon.h"
#include "tile.h"

using namespace std;


const QColor TileWidget::EmptyColour(Qt::white);
const QColor TileWidget::L2Colour(34, 189, 240);
const QColor TileWidget::L3Colour(29, 104, 240);
const QColor TileWidget::W2Colour(255, 147, 196);
const QColor TileWidget::W3Colour(240, 80, 94);
const QColor TileWidget::TileColour(255, 235, 205);
const QColor TileWidget::PreviewColour(183, 183, 123);
const QColor TileWidget::NormalColour(0, 0, 0);
const QColor TileWidget::JokerColour(255, 0, 0);
const QColor TileWidget::ArrowColour(10, 10, 10);


BasicTileWidget::BasicTileWidget(QWidget *parent, QString text)
    : QWidget(parent), m_text(text)
{
}


int BasicTileWidget::heightForWidth(int width) const
{
    return width;
}


QSize BasicTileWidget::sizeHint() const
{
    return QSize(30, 30);
}


int BasicTileWidget::getSquareSize() const
{
    return std::min(width(), height());
}


void BasicTileWidget::paintEvent(QPaintEvent *)
{
    const int squareSize = getSquareSize();
    QFont letterFont = font();
    letterFont.setPixelSize(squareSize * 2 / 3);

    QPainter painter(this);
    painter.setFont(letterFont);
    painter.drawText(0, 1, squareSize, squareSize, Qt::AlignCenter, m_text);
}


TileWidget::TileWidget(QWidget *parent, Multiplier multiplier)
    : BasicTileWidget(parent), m_multiplier(multiplier), m_isJoker(false),
    m_isPreview(false), m_showArrow(false), m_horizontalArrow(true)
{
    setMinimumSize(15, 15);
    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    policy.setHeightForWidth(true);
    setSizePolicy(policy);

    // Try to have a black background... FIXME: not working well!
    QPalette pal = palette();
    for (int i = 0; i <= 19; ++i)
        pal.setColor((QPalette::ColorRole)i, Qt::black);
    setPalette(pal);
    setForegroundRole(QPalette::Window);
    setBackgroundRole(QPalette::Window);
}


void TileWidget::tileChanged(const Tile &iTile, bool isJoker, bool isPreview,
                             bool showArrow, bool horizontalArrow)
{
    m_tile = iTile;
    m_isJoker = isJoker;
    m_isPreview = isPreview;
    m_showArrow = showArrow;
    m_horizontalArrow = horizontalArrow;
    update();
}


void TileWidget::paintEvent(QPaintEvent *)
{
    const int squareSize = getSquareSize();

    // The font must grow with the square size
    QFont letterFont = font();
    letterFont.setPixelSize(squareSize * 2 / 3);

    QFont pointsFont = font();
    const double pointsCoeff = 8. / 25.;
    pointsFont.setPixelSize(squareSize * pointsCoeff);

    // XXX: Naive implementation: we repaint everything every time
    QPainter painter(this);
    //painter.setPen(Qt::NoPen);
    const unsigned int xPos = 0;
    const unsigned int yPos = 0;

    // Set the brush color
    if (!m_tile.isEmpty())
    {
        if (m_isPreview)
            painter.setBrush(PreviewColour);
        else
            painter.setBrush(TileColour);
    }
    else if (m_multiplier == WORD_TRIPLE)
        painter.setBrush(W3Colour);
    else if (m_multiplier == WORD_DOUBLE)
        painter.setBrush(W2Colour);
    else if (m_multiplier == LETTER_TRIPLE)
        painter.setBrush(L3Colour);
    else if (m_multiplier == LETTER_DOUBLE)
        painter.setBrush(L2Colour);
    else
        painter.setBrush(EmptyColour);
    painter.drawRect(xPos, yPos, squareSize, squareSize);

    // Draw the letter
    if (!m_tile.isEmpty())
    {
        wstring chr = m_tile.getDisplayStr();
        // Make the display char in upper case
        std::transform(chr.begin(), chr.end(), chr.begin(), towupper);
        if (m_isJoker)
            painter.setPen(JokerColour);
        painter.setFont(letterFont);
        painter.drawText(xPos, yPos + 1, squareSize, squareSize,
                         Qt::AlignCenter, qfw(chr));
        painter.setPen(NormalColour);

        // Should we display the tiles points?
        QSettings qs(ORGANIZATION, PACKAGE_NAME);
        const bool showPoints = qs.value(PrefsDialog::kINTF_SHOW_TILES_POINTS, true).toBool();

        // Draw the points of the tile
        if (showPoints && !m_isJoker)
        {
            painter.setFont(pointsFont);
            painter.drawText(xPos + squareSize * (1 - pointsCoeff),
                             yPos + squareSize * (1 - pointsCoeff),
                             squareSize * pointsCoeff, squareSize * pointsCoeff + 3,
                             Qt::AlignRight | Qt::AlignBottom,
                             QString("%1").arg(m_tile.getPoints()));
        }
    }
    // Draw the arrow
    if (m_showArrow)
    {
        const unsigned int xPos = 1;
        const unsigned int yPos = 1;
        painter.setPen(QPen(painter.brush().color(), 0));
        painter.setBrush(ArrowColour);
        const int mid = squareSize / 2;
        const int fifth = squareSize / 5;
        const int width = squareSize / 16;
        painter.translate(xPos + mid, yPos + mid);
        if (m_horizontalArrow)
            painter.rotate(90);
        const QPoint points[] =
        {
            QPoint(-mid + fifth - 1, -width - 1),
            QPoint(-mid + 3*fifth - 1, -width - 1),
            QPoint(-mid + 3*fifth - 1, -fifth - 2),
            QPoint(-mid + 4*fifth + 1, 0),
            QPoint(-mid + 3*fifth - 1, fifth + 2),
            QPoint(-mid + 3*fifth - 1, width + 1),
            QPoint(-mid + fifth - 1, width + 1)
        };
        painter.drawPolygon(points, 7);

        painter.setPen(QPen());
        painter.setBrush(NormalColour);
    }
}

