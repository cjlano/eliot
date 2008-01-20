/*****************************************************************************
 * Eliot
 * Copyright (C) 2008 Olivier Teulière
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

#ifndef BOARD_WIDGET_H_
#define BOARD_WIDGET_H_

#include <QtGui/QFrame>


class Board;
class QTreeView;

class BoardWidget: public QFrame
{
    Q_OBJECT;

public:
    explicit BoardWidget(QWidget *parent = 0, const Board *iBoard = NULL);

public slots:
    void setBoard(const Board *iBoard = NULL);
    void refresh();

protected:
    /// Define a default size
    virtual QSize sizeHint() const;
    /// Paint the board
    virtual void paintEvent(QPaintEvent *iEvent);

private:
    /// Encapsulated board, can be NULL
    const Board *m_board;

    /// Define a few background colours
    //@{
    static const QColor EmptyColour;
    static const QColor L2Colour;
    static const QColor L3Colour;
    static const QColor W2Colour;
    static const QColor W3Colour;
    static const QColor TileColour;
    //@}
};

#endif

