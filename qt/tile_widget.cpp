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

INIT_LOGGER(qt, BasicTileWidget);
INIT_LOGGER(qt, TileWidget);
INIT_LOGGER(qt, TileWidgetDecorator);


const QColor TileWidget::EmptyColour(Qt::white);
const QColor TileWidget::L2Colour(34, 189, 240);
const QColor TileWidget::L3Colour(29, 104, 240);
const QColor TileWidget::W2Colour(255, 147, 196);
const QColor TileWidget::W3Colour(240, 80, 94);
const QColor TileWidget::TileColour(255, 235, 205);
const QColor TileWidget::PreviewColour(183, 183, 123);
const QColor TileWidget::PlayedColour(Qt::white);
const QColor TileWidget::NormalColour(0, 0, 0);
const QColor TileWidget::JokerColour(255, 0, 0);
const QColor TileWidget::ArrowColour(10, 10, 10);


BasicTileWidget::BasicTileWidget(QWidget *parent, QString text)
    : QWidget(parent), m_text(text)
{
    setMinimumSize(10, 10);
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
    painter.drawText(1, 1, squareSize, squareSize, Qt::AlignCenter, m_text);
}

// --------------

TileWidget::TileWidget(QWidget *parent, Multiplier multiplier,
                       int row, int col)
    : BasicTileWidget(parent), m_multiplier(multiplier),
    m_row(row), m_col(col), m_isJoker(false),
    m_state(NORMAL), m_showArrow(false), m_horizontalArrow(true)
{
    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    policy.setHeightForWidth(true);
    setSizePolicy(policy);
}


void TileWidget::tileChanged(const Tile &iTile, bool isJoker, State state)
{
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

    // Set the square color
    QColor color;
    if (!m_tile.isEmpty())
    {
        if (m_state == PREVIEW)
            color = PreviewColour;
        else if (m_state == PLAYED)
            color = PlayedColour;
        else
            color = TileColour;
    }
    else if (m_multiplier == WORD_TRIPLE)
        color = W3Colour;
    else if (m_multiplier == WORD_DOUBLE)
        color = W2Colour;
    else if (m_multiplier == LETTER_TRIPLE)
        color = L3Colour;
    else if (m_multiplier == LETTER_DOUBLE)
        color = L2Colour;
    else
        color = EmptyColour;
    painter.fillRect(0, 0, squareSize, squareSize, color);

    // Draw the letter
    if (!m_tile.isEmpty())
    {
        wstring chr = m_tile.getDisplayStr();
        // Make the display char in upper case
        std::transform(chr.begin(), chr.end(), chr.begin(), towupper);
        if (m_isJoker)
            painter.setPen(JokerColour);
        painter.setFont(letterFont);
        if (!m_tile.isPureJoker())
        {
            painter.drawText(0, 0, squareSize, squareSize,
                             Qt::AlignCenter, qfw(chr));
        }
        painter.setPen(NormalColour);

        // Should we display the tiles points?
        QSettings qs(ORGANIZATION, PACKAGE_NAME);
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
        painter.setPen(QPen(painter.brush().color(), 0));
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

        painter.setPen(QPen());
        painter.setBrush(NormalColour);
    }
    if (m_state == PLAYED)
    {
        painter.drawLine(QLine(0, 0, squareSize, squareSize));
        painter.drawLine(QLine(0, squareSize, squareSize, 0));
    }
}


void TileWidget::mousePressEvent(QMouseEvent *iEvent)
{
    emit mousePressed(m_row, m_col, iEvent);
}

// --------------

TileWidgetDecorator::TileWidgetDecorator(QWidget *parent, TileWidget &wrapped)
    : TileWidget(parent), m_wrapped(wrapped)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(1, 1, 1, 1);
    layout->addWidget(&m_wrapped);
    setLayout(layout);

    QObject::connect(&m_wrapped, SIGNAL(mousePressed(int, int, QMouseEvent*)),
                     this, SIGNAL(mousePressed(int, int, QMouseEvent*)));
}


void TileWidgetDecorator::tileChanged(const Tile &iTile, bool isJoker, State state)
{
    m_wrapped.tileChanged(iTile, isJoker, state);
}


void TileWidgetDecorator::arrowChanged(bool showArrow, bool horizontalArrow)
{
    m_wrapped.arrowChanged(showArrow, horizontalArrow);
}

void TileWidgetDecorator::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawRect(0, 0, width() - 1, height() - 1);
}

