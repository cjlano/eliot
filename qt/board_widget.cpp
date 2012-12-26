/*****************************************************************************
 * Eliot
 * Copyright (C) 2008-2012 Olivier Teulière
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
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

#include "board_widget.h"
#include "tile_layout.h"
#include "tile_widget.h"
#include "qtcommon.h"
#include "public_game.h"
#include "tile.h"
#include "board_layout.h"
#include "board.h"
#include "play_model.h"
#include "move.h"

using namespace std;


INIT_LOGGER(qt, BoardWidget);


BoardWidget::BoardWidget(PlayModel &iPlayModel, QWidget *parent)
    : QFrame(parent), m_game(NULL),
    m_playModel(iPlayModel), m_showTemporarySigns(true),
    m_showOnlyLastTurn(false)
{
    const BoardLayout & boardLayout = BoardLayout::GetDefault();
    const unsigned nbRows = boardLayout.getRowCount();
    const unsigned nbCols = boardLayout.getColCount();

    m_widgetsMatrix.resize(nbRows + 1, nbCols + 1, 0);

    // Try to have a black background... FIXME: not working well!
    QPalette pal = palette();
    for (int i = 0; i <= 19; ++i)
        pal.setColor((QPalette::ColorRole)i, Qt::black);
    setPalette(pal);
    setForegroundRole(QPalette::Window);
    setBackgroundRole(QPalette::Window);

    TileLayout *layout = new TileLayout(nbRows + 1, nbCols + 1);
    layout->setSpacing(1);
    layout->setAlignment(Qt::AlignHCenter);

    // Line full of coordinates
    TileWidget *cornerTile = new TileWidget;
    cornerTile->setCoordText("");
    layout->addWidget(cornerTile);
    for (unsigned int col = 1; col <= nbCols; ++col)
    {
        TileWidget *coordTile = new TileWidget;
        coordTile->setCoordText(QString("%1").arg(col));
        layout->addWidget(coordTile);
    }

    // Rest of the board
    for (unsigned int row = 1; row <= nbRows; ++row)
    {
        // Add the coordinate
        TileWidget *coordTile = new TileWidget;
        coordTile->setCoordText(QString(QChar('A' + row - 1)));
        layout->addWidget(coordTile);
        // Add the squares
        for (unsigned int col = 1; col <= nbCols; ++col)
        {
            TileWidget::Multiplier mult = TileWidget::NONE;
            if (boardLayout.getWordMultiplier(row, col) == 3)
                mult = TileWidget::WORD_TRIPLE;
            else if (boardLayout.getWordMultiplier(row, col) == 2)
                mult = TileWidget::WORD_DOUBLE;
            else if (boardLayout.getLetterMultiplier(row, col) == 3)
                mult = TileWidget::LETTER_TRIPLE;
            else if (boardLayout.getLetterMultiplier(row, col) == 2)
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

    // Listen to changes in the coordinates
    QObject::connect(&m_playModel, SIGNAL(coordChanged(const Coord&, const Coord&)),
                     this, SLOT(updateArrow(const Coord&, const Coord&)));
    QObject::connect(&m_playModel, SIGNAL(moveChanged(const Move&, const Move&)),
                     this, SLOT(onMoveChanged(const Move&)));
}


void BoardWidget::setGame(const PublicGame *iGame)
{
    m_game = iGame;
    refresh();
}


void BoardWidget::updateArrow(const Coord &iNewCoord, const Coord &iOldCoord)
{
    // Refresh only the 2 involved squares
    if (iOldCoord.isValid())
    {
        TileWidget *t = m_widgetsMatrix[iOldCoord.getRow()][iOldCoord.getCol()];
        t->arrowChanged(false, false);
    }

    if (!m_showTemporarySigns)
        return;

    if (iNewCoord.isValid())
    {
        TileWidget *t = m_widgetsMatrix[iNewCoord.getRow()][iNewCoord.getCol()];
        t->arrowChanged(iNewCoord.isValid(), iNewCoord.getDir() == Coord::VERTICAL);
    }
}


void BoardWidget::onMoveChanged(const Move &iMove)
{
    if (m_game == NULL)
        return;

    // FIXME
    Board &board = const_cast<Board&>(m_game->getBoard());
    board.removeTestRound();
    if (iMove.isValid())
    {
        board.testRound(iMove.getRound());
    }
    refresh();
}


void BoardWidget::refresh()
{
    if (m_game != NULL)
    {
        if (m_showOnlyLastTurn && !m_game->isLastTurn())
            return;
        // Note: the TileWidget class will redraw the tile only if something
        // has changed, to avoid useless repainting.
        const Board &board = m_game->getBoard();
        const unsigned nbRows = board.getLayout().getRowCount();
        const unsigned nbCols = board.getLayout().getColCount();
        for (unsigned row = 1; row <= nbRows; ++row)
        {
            for (unsigned col = 1; col <= nbCols; ++col)
            {
                if (board.isTestChar(row, col) && m_showTemporarySigns)
                {
                    const Tile &tile = board.getTestTile(row, col);
                    m_widgetsMatrix[row][col]->tileChanged(
                                TileWidget::PREVIEW,
                                tile, tile.isJoker());
                }
                else
                {
                    if (board.isVacant(row, col))
                    {
                        // Force an empty square.
                        m_widgetsMatrix[row][col]->tileChanged(TileWidget::BOARD_EMPTY);
                    }
                    else
                    {
                        m_widgetsMatrix[row][col]->tileChanged(
                                TileWidget::NORMAL,
                                board.getTile(row, col),
                                board.isJoker(row, col));
                    }
                }
            }
        }
    }
    else
    {
        // Clear the board
        for (unsigned row = 1; row < m_widgetsMatrix.size(); ++row)
        {
            for (unsigned col = 1; col < m_widgetsMatrix[1].size(); ++col)
            {
                m_widgetsMatrix[row][col]->tileChanged(TileWidget::BOARD_EMPTY);
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
    const TileLayout *boardLayout = (TileLayout*)layout();
    QPainter painter(this);
    QRect rect = boardLayout->getBoardRect();
    const int size = boardLayout->getSquareSize();
    const int spacing = boardLayout->spacing();

    // Draw lines between tiles
    QLine hLine(0, 0, rect.width() + 1, 0);
    QLine vLine(0, 0, 0, rect.height() + 1);
    hLine.translate(rect.left() - 1, rect.top() - 1);
    vLine.translate(rect.left() - 1, rect.top() - 1);
    for (unsigned i = 0; i < m_widgetsMatrix.size(); ++i)
    {
        painter.drawLine(hLine);
        painter.drawLine(vLine);
        hLine.translate(0, size + spacing);
        vLine.translate(size + spacing, 0);
    }
    // Draw a second line around the board, so that it looks nicer
    painter.drawRect(rect.adjusted(-2, -2, 1, 1));
}


void BoardWidget::tileClicked(int row, int col, QMouseEvent *iEvent)
{
    if (m_game == NULL)
    {
        m_playModel.clear();
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
        if (m_playModel.getCoord() == coord)
            coord.setDir(Coord::VERTICAL);
        // Take into acount the new coordinates
        m_playModel.setCoord(coord);
    }
    else if (iEvent->button() == Qt::RightButton)
    {
        // On a right click anywhere on the board, remove the arrow
        m_playModel.clear();
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
        if (m_playModel.getCoord().getRow() == coord.getRow() &&
            m_playModel.getCoord().getCol() == coord.getCol())
        {
            if (m_playModel.getCoord().getDir() == Coord::VERTICAL)
            {
                // Third click: clear the arrow
                m_playModel.clear();
                return;
            }
            coord.setDir(Coord::VERTICAL);
        }
        // Take into acount the new coordinates
        m_playModel.setCoord(coord);
    }
    else if (iEvent->button() == Qt::RightButton)
    {
        // On a right click anywhere on the board, remove the arrow
        m_playModel.clear();
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
        if (m_playModel.getCoord() == coord)
            m_playModel.clear();
        else
            m_playModel.setCoord(coord);
    }
    else if (iEvent->button() == Qt::RightButton)
    {
        Coord coord(row, col, Coord::VERTICAL);
        // Remove the coordinates if they are exactly the same as the current ones,
        // otherwise set the coordinates;
        if (m_playModel.getCoord() == coord)
            m_playModel.clear();
        else
            m_playModel.setCoord(coord);
    }
    else
        m_playModel.clear();
#endif
}


