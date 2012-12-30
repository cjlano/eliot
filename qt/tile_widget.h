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

#ifndef TILE_WIDGET_H_
#define TILE_WIDGET_H_

#include <QtGui/QFrame>
#include "tile.h"
#include "logging.h"


/**
 * Widget used to display a square on the board, with or without letter.
 * It can also be used to display coordinates, if the setCoordText() method
 * has been called.
 */
class TileWidget: public QFrame
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    enum Multiplier
    {
        NONE,
        LETTER_DOUBLE,
        LETTER_TRIPLE,
        WORD_DOUBLE,
        WORD_TRIPLE
    };

    enum State
    {
        COORDS,
        NORMAL,
        PREVIEW,
        BOARD_EMPTY,
        RACK_PLAYED,
        SHADED,
    };

    explicit TileWidget(QWidget *parent = 0, Multiplier multiplier = NONE,
                        int row = 0, int col = 0);

    void setCoordText(QString iText);

    void setBorder(int width = 2);

    int getSquareSize() const;

    const Tile & getTile() const { return m_tile; }

    virtual int heightForWidth(int w) const;

    /// Define a default size
    virtual QSize sizeHint() const;

public slots:
    void tileChanged(State state, const Tile &iTile = Tile(), bool isJoker = false);
    void arrowChanged(bool showArrow, bool horizontalArrow);

signals:
    void mousePressed(int row, int col, QMouseEvent *iEvent);

protected:
    /// Paint the square
    virtual void paintEvent(QPaintEvent *iEvent);
    /// Catch mouse events to send a signal
    virtual void mousePressEvent(QMouseEvent *iEvent);

private:
    /// Encapsulated tile
    Tile m_tile;

    /// Word or letter multipler
    Multiplier m_multiplier;

    /// Position of the tile, only used to fire the mousePressed() signal
    // XXX: another way would be to send *this in the mousePressed() signal
    int m_row;
    int m_col;

    /// Whether the tile is a joker
    bool m_isJoker;

    /// State of the tile
    State m_state;

    /// Whether we should show the arrow
    bool m_showArrow;

    /// Whether the arrow is horizontal
    bool m_horizontalArrow;

    /// Text used for coordinates
    QString m_text;

    /// Define a few background colours
    //@{
    static const QColor BoardEmptyColour;
    static const QColor BoardL2Colour;
    static const QColor BoardL3Colour;
    static const QColor BoardW2Colour;
    static const QColor BoardW3Colour;
    static const QColor TileNormalColour;
    static const QColor TilePreviewColour;
    static const QColor TilePlayedColour;
    static const QColor TileShadedColour;
    static const QColor TextNormalColour;
    static const QColor TextJokerColour;
    static const QColor ArrowColour;
    //@}
};

#endif

