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

#ifndef TILE_WIDGET_H_
#define TILE_WIDGET_H_

#include <QtGui/QWidget>
#include "tile.h"


/**
 * Simplified tile, only used to draw the coordinates of the board
 */
class BasicTileWidget: public QWidget
{
public:
    BasicTileWidget(QWidget *parent = 0, QString text = "");

    int getSquareSize() const;

    virtual int heightForWidth(int w) const;

protected:
    /// Define a default size
    virtual QSize sizeHint() const;
    /// Paint the square
    virtual void paintEvent(QPaintEvent *iEvent);

private:
    QString m_text;
};


/**
 * Widget used to display a square on the board, with or without letter.
 */
class TileWidget: public BasicTileWidget
{
    Q_OBJECT;

public:
    enum Multiplier
    {
        NONE = 0,
        LETTER_DOUBLE = 1,
        LETTER_TRIPLE = 2,
        WORD_DOUBLE = 3,
        WORD_TRIPLE = 4
    };

    explicit TileWidget(QWidget *parent = 0, Multiplier multiplier = NONE);

public slots:
    void tileChanged(const Tile &iTile, bool isJoker, bool isPreview,
                     bool showArrow, bool horizontalArrow);

protected:
    /// Paint the square
    virtual void paintEvent(QPaintEvent *iEvent);

private:
    /// Encapsulated tile
    Tile m_tile;

    /// Word or letter multipler
    Multiplier m_multiplier;

    /// Whether the tile is a joker
    bool m_isJoker;

    /// Whether the tile is used as a preview
    bool m_isPreview;

    /// Whether we should show the arrow
    bool m_showArrow;

    /// Whether the arrow is horizontal
    bool m_horizontalArrow;

    /// Define a few background colours
    //@{
    static const QColor EmptyColour;
    static const QColor L2Colour;
    static const QColor L3Colour;
    static const QColor W2Colour;
    static const QColor W3Colour;
    static const QColor TileColour;
    static const QColor PreviewColour;
    static const QColor NormalColour;
    static const QColor JokerColour;
    static const QColor ArrowColour;
    //@}
};

#endif

