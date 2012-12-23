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

#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtCore/QSettings>

#include "tile_widget.h"
#include "prefs_dialog.h"
#include "qtcommon.h"
#include "tile.h"
#include "encoding.h"

using namespace std;

INIT_LOGGER(qt, TileWidget);


const QColor TileWidget::BoardEmptyColour(Qt::white);
const QColor TileWidget::BoardL2Colour(34, 189, 240);
const QColor TileWidget::BoardL3Colour(29, 104, 240);
const QColor TileWidget::BoardW2Colour(255, 147, 196);
const QColor TileWidget::BoardW3Colour(240, 80, 94);
const QColor TileWidget::TileNormalColour(255, 235, 205);
const QColor TileWidget::TilePreviewColour(183, 183, 123);
const QColor TileWidget::TilePlayedColour(Qt::white);
const QColor TileWidget::TextNormalColour(0, 0, 0);
const QColor TileWidget::TextJokerColour(255, 0, 0);
const QColor TileWidget::ArrowColour(10, 10, 10);


TileWidget::TileWidget(QWidget *parent, Multiplier multiplier,
                       int row, int col)
    : QFrame(parent), m_multiplier(multiplier),
    m_row(row), m_col(col), m_isJoker(false),
    m_state(NORMAL), m_showArrow(false), m_horizontalArrow(true)
{
    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    policy.setHeightForWidth(true);
    setSizePolicy(policy);

    setMidLineWidth(0);
    setBorder(0);
}


void TileWidget::setCoordText(QString iText)
{
    m_state = COORDS;
    m_text = iText;
    setBorder(0);
}


void TileWidget::setBorder(int width)
{
    if (frameWidth() == width)
        return;

    if (width <= 0)
    {
        setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    }
    else
    {
        setFrameStyle(QFrame::Box | QFrame::Plain);
        setLineWidth(width);
    }
    setMinimumSize(QSize(15, 15) + 2 * QSize(width, width));
}


int TileWidget::heightForWidth(int width) const
{
    return width;
}


QSize TileWidget::sizeHint() const
{
    const int width = frameWidth();
    return QSize(30, 30) + 2 * QSize(width, width);
}


int TileWidget::getSquareSize() const
{
    return std::min(contentsRect().width(), contentsRect().height());
}


void TileWidget::tileChanged(State state, const Tile &iTile, bool isJoker)
{
    // This avoids a lot of useless redraws
    if (m_state == state && m_tile == iTile && m_isJoker == isJoker)
        return;

    m_state = state;
    m_tile = iTile;
    m_isJoker = isJoker;
    m_state = state;
    update();
}


void TileWidget::arrowChanged(bool showArrow, bool horizontalArrow)
{
    m_showArrow = showArrow;
    m_horizontalArrow = horizontalArrow;
    update();
}


void TileWidget::paintEvent(QPaintEvent *iEvent)
{
    QFrame::paintEvent(iEvent);

    const int squareSize = getSquareSize();

    QPainter painter(this);
    painter.translate(QPoint(contentsMargins().left(), contentsMargins().top()));

    // The font must grow with the square size
    QFont letterFont = font();
    letterFont.setPixelSize(std::max(1, squareSize * 2 / 3));

    if (m_state == COORDS)
    {
        painter.setFont(letterFont);
        painter.drawText(1, 1, squareSize, squareSize, Qt::AlignCenter, m_text);

        return;
    }

    QFont pointsFont = font();
    const double pointsCoeff = 8. / 25.;
    pointsFont.setPixelSize(std::max(1., squareSize * pointsCoeff));

    // XXX: Naive implementation: we repaint everything every time

    // Set the square color
    QColor color;
    if (!m_tile.isEmpty() && m_state != BOARD_EMPTY)
    {
        if (m_state == PREVIEW)
            color = TilePreviewColour;
        else if (m_state == RACK_PLAYED)
            color = TilePlayedColour;
        else
            color = TileNormalColour;
    }
    else if (m_multiplier == WORD_TRIPLE)
        color = BoardW3Colour;
    else if (m_multiplier == WORD_DOUBLE)
        color = BoardW2Colour;
    else if (m_multiplier == LETTER_TRIPLE)
        color = BoardL3Colour;
    else if (m_multiplier == LETTER_DOUBLE)
        color = BoardL2Colour;
    else
        color = BoardEmptyColour;
    painter.fillRect(0, 0, squareSize, squareSize, color);

    // Draw the letter
    if (!m_tile.isEmpty())
    {
        wstring chr = toUpper(m_tile.getDisplayStr());
        // Make the display char in upper case
        painter.setPen(m_isJoker ? TextJokerColour : TextNormalColour);
        painter.setFont(letterFont);
        if (!m_tile.isPureJoker())
        {
            painter.drawText(0, 0, squareSize, squareSize,
                             Qt::AlignCenter, qfw(chr));
        }
        painter.setPen(TextNormalColour);

        // Should we display the tiles points?
        QSettings qs;
        const bool showPoints = qs.value(PrefsDialog::kINTF_SHOW_TILES_POINTS, true).toBool();

        // Draw the points of the tile
        if (showPoints && !m_isJoker && !m_tile.isJoker())
        {
            painter.setFont(pointsFont);
            painter.drawText(0,
                             squareSize * (1 - pointsCoeff),
                             squareSize - 1, squareSize * pointsCoeff + 3,
                             Qt::AlignRight | Qt::AlignBottom,
                             QString("%1").arg(m_tile.getPoints()));
        }
    }

    // Draw the arrow
    if (m_showArrow)
    {
        painter.setPen(ArrowColour);
        painter.setBrush(ArrowColour);
        const int mid = squareSize / 2;
        const int fifth = squareSize / 5;
        const int width = squareSize / 16;
        painter.translate(mid, mid);
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
    }

    if (m_state == RACK_PLAYED)
    {
        painter.setPen(TextNormalColour);
        painter.drawLine(QLine(0, 0, squareSize, squareSize));
        painter.drawLine(QLine(0, squareSize, squareSize, 0));
    }
}


void TileWidget::mousePressEvent(QMouseEvent *iEvent)
{
    emit mousePressed(m_row, m_col, iEvent);
}

