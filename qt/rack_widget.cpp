/*****************************************************************************
 * Eliot
 * Copyright (C) 2012 Olivier Teulière
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

#include <QtGui/QHBoxLayout>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragLeaveEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>

#include "rack_widget.h"
#include "tile_widget.h"
#include "tile_layout.h"
#include "play_model.h"
#include "qtcommon.h"

#include "public_game.h"
#include "pldrack.h"
#include "debug.h"

using namespace std;

INIT_LOGGER(qt, RackWidget);


#define MIME_TYPE "text/x-tile"


RackWidget::RackWidget(QWidget *parent)
    : QFrame(parent), m_game(NULL),
      m_playModel(NULL), m_showOnlyLastTurn(false),
      m_dragOrigin(-1)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    TileLayout *layout = new TileLayout(1);
    layout->setSpacing(5);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);

    setAcceptDrops(true);
}


void RackWidget::setPlayModel(PlayModel *iPlayModel)
{
    if (m_playModel != NULL)
        m_playModel->disconnect(this, SLOT(refresh()));
    if (iPlayModel != NULL)
    {
        QObject::connect(iPlayModel, SIGNAL(moveChanged(const Move &, const Move&)),
                         this, SLOT(refresh()));
    }
    m_playModel = iPlayModel;
}


void RackWidget::setGame(const PublicGame *iGame)
{
    m_game = iGame;
    if (m_game == NULL)
    {
        // Delete the widgets
        TileLayout *layout = (TileLayout*) this->layout();
        layout->clear();
        m_tilesVect.clear();
    }
}


void RackWidget::setRack(const PlayedRack &iRack)
{
    ASSERT(m_game != NULL, "setRack() called without a game");
    if (m_showOnlyLastTurn && !m_game->isLastTurn())
        return;

    // Get the tiles
    vector<Tile> tiles;
    iRack.getAllTiles(tiles);

    // Update the rack
    setTiles(tiles);
}


void RackWidget::setTiles(const vector<Tile> &iTiles)
{
    m_tiles = iTiles;
    refresh();
}


void RackWidget::refresh()
{
    m_filteredTiles = filterRack(m_tiles);

    unsigned tilesCount = m_filteredTiles.size();

    // Make sure we have as many widgets as there are letters in the rack
    while (m_tilesVect.size() > tilesCount)
    {
        QtCommon::DestroyObject(m_tilesVect.back());
        m_tilesVect.pop_back();
    }
    while (m_tilesVect.size() < tilesCount)
    {
        TileWidget *tileWidget =
            new TileWidget(0, TileWidget::NONE, 0, m_tilesVect.size());
        QObject::connect(tileWidget, SIGNAL(mousePressed(int, int, QMouseEvent*)),
                         this, SLOT(tilePressed(int, int, QMouseEvent*)));
        tileWidget->setBorder(2);
        layout()->addWidget(tileWidget);
        m_tilesVect.push_back(tileWidget);
    }
    ASSERT(m_tilesVect.size() == tilesCount, "Invalid number of tiles");

    // Update the widgets
    for (unsigned int i = 0; i < tilesCount; ++i)
    {
        TileWidget *tileWidget = m_tilesVect[i];
        tileWidget->tileChanged(TileWidget::NORMAL, m_tiles[i]);
    }
}


vector<Tile> RackWidget::filterRack(const vector<Tile> &iTiles) const
{
    if (m_playModel == NULL || !m_playModel->getMove().isValid())
        return iTiles;

    vector<Tile> result = iTiles;
    const Round &round = m_playModel->getMove().getRound();
    for (unsigned i = 0; i < round.getWordLen(); ++i)
    {
        if (round.isPlayedFromRack(i))
        {
            const Tile &t = round.getTile(i);
            vector<Tile>::iterator it;
            for (it = result.begin(); it != result.end(); ++it)
            {
                if (*it == t)
                {
                    result.erase(it);
                    break;
                }
            }
        }
    }

    return result;
}


// Drag & drop handling


bool RackWidget::canStartDragDrop() const
{
    if (m_game == NULL || m_game->isFinished())
        return false;
    // Drag & drop is not allowed when a word is being played
    return m_tiles.size() == m_filteredTiles.size();
}


void RackWidget::tilePressed(int row, int col, QMouseEvent *event)
{
    ASSERT(row == 0, "Multi-line racks are not supported");
    ASSERT(col >= 0 && (unsigned)col < m_tilesVect.size(),
           "Invalid tile index: " << col);

    if (!canStartDragDrop())
        return;

    LOG_DEBUG("Starting drag for tile " << col);

    TileWidget *tileWidget = m_tilesVect[col];

    // Save the initial column of the moved tile
    QMimeData *mimeData = new QMimeData;
    mimeData->setData(MIME_TYPE, QByteArray());
    m_dragOrigin = col;

    // Create an image of the tile
    QPixmap pixmap(tileWidget->size());
    tileWidget->render(&pixmap);

    // Initiate the drag
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setHotSpot(event->pos());
    drag->setPixmap(pixmap);

    if (!(drag->exec(Qt::MoveAction) == Qt::MoveAction))
    {
        // TODO
    }
}


void RackWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(MIME_TYPE))
        event->accept();
    else
        event->ignore();
}


void RackWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}


void RackWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(MIME_TYPE))
    {
        event->setDropAction(Qt::MoveAction);

        int closestCol = findClosestTile(event->pos());
        moveTile(m_dragOrigin, closestCol, true);
        // We have a new drag origin, since we just moved the tile
        m_dragOrigin = closestCol;

        event->accept();
    }
    else
    {
        event->ignore();
    }
}


void RackWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat(MIME_TYPE))
    {
        int closestCol = findClosestTile(event->pos());
        LOG_DEBUG("Dropping tile " << m_dragOrigin << " closest to tile " << closestCol);

        moveTile(m_dragOrigin, closestCol);

        event->setDropAction(Qt::MoveAction);
        event->accept();

        // Notify the rest of the world that the rack has changed
        ASSERT(m_game != NULL, "No game in progress");
        PlayedRack pldRack;
        Q_FOREACH(const Tile &t, m_tiles)
        {
            pldRack.addNew(t);
        }
        // FIXME: const_cast
        const_cast<PublicGame*>(m_game)->reorderRack(pldRack);
        emit gameUpdated();
    }
    else
    {
        event->ignore();
    }
}


int RackWidget::findTile(const QPoint &iPos) const
{
    for (unsigned i = 0; i < m_tilesVect.size(); ++i)
    {
        if (m_tilesVect[i]->geometry().contains(iPos))
            return i;
    }
    return -1;
}


int RackWidget::findClosestTile(const QPoint &iPos) const
{
    int minDist = geometry().bottomRight().manhattanLength();
    int minIdx = -1;
    for (unsigned i = 0; i < m_tilesVect.size(); ++i)
    {
        QPoint distPoint = iPos - m_tilesVect[i]->geometry().center();
        int dist = distPoint.manhattanLength();
        if (dist < minDist)
        {
            minDist = dist;
            minIdx = i;
        }
    }
    return minIdx;
}


void RackWidget::moveTile(int fromPos, int toPos, bool shaded)
{
    if (fromPos != toPos)
    {
        // Change the order
        Tile moved = m_filteredTiles[fromPos];
        m_filteredTiles.erase(m_filteredTiles.begin() + fromPos);
        m_filteredTiles.insert(m_filteredTiles.begin() + toPos, moved);
        // Update the rack
        setTiles(m_filteredTiles);
    }
    // Change the look of the moved tile
    m_tilesVect[toPos]->tileChanged(shaded ? TileWidget::SHADED : TileWidget::NORMAL,
                                    m_filteredTiles[toPos]);
}


