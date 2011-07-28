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

#include "config.h"

#include <QtGui/QTreeView>
#include <QtGui/QTabWidget>
#include <QtGui/QStandardItemModel>
#include <QtCore/QSettings>

#include "history_widget.h"
#include "custom_popup.h"
#include "prefs_dialog.h"
#include "qtcommon.h"
#include "public_game.h"
#include "player.h"
#include "history.h"
#include "turn.h"
#include "move.h"

using namespace std;


HistoryWidget::HistoryWidget(QWidget *parent)
    : QTreeView(parent), m_history(NULL), m_forPlayer(false)
{
    // Create the tree view
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setRootIsDecorated(false);

    // Add a context menu for the results
    m_customPopup = new CustomPopup(this);
    QObject::connect(m_customPopup, SIGNAL(popupCreated(QMenu&, const QPoint&)),
                     this, SLOT(populateMenu(QMenu&, const QPoint&)));
    QObject::connect(m_customPopup, SIGNAL(requestDefinition(QString)),
                     this, SIGNAL(requestDefinition(QString)));

    // Associate the model to the view
    m_model = new QStandardItemModel(this);
    setModel(m_model);
    m_model->setColumnCount(6);
    m_model->setHeaderData(0, Qt::Horizontal, _q("Turn"), Qt::DisplayRole);
    m_model->setHeaderData(1, Qt::Horizontal, _q("Rack"), Qt::DisplayRole);
    m_model->setHeaderData(2, Qt::Horizontal, _q("Word"), Qt::DisplayRole);
    m_model->setHeaderData(3, Qt::Horizontal, _q("Ref"), Qt::DisplayRole);
    m_model->setHeaderData(4, Qt::Horizontal, _q("Points"), Qt::DisplayRole);
    updateModel();
}


void HistoryWidget::setHistory(const History *iHistory,
                               const PublicGame *iGame,
                               bool iIsForPlayer)
{
    m_history = iHistory;
    m_game = iGame;
    m_forPlayer = iIsForPlayer;
    updateModel();
}


void HistoryWidget::refresh()
{
    updateModel();
}


void HistoryWidget::populateMenu(QMenu &iMenu, const QPoint &iPoint)
{
    const QModelIndex &index = indexAt(iPoint);
    if (!index.isValid())
        return;

    // Find the selected word
    const QModelIndex &wordIndex = m_model->index(index.row(), 2);
    QString selectedWord = m_model->data(wordIndex).toString();

    if (selectedWord != "")
        m_customPopup->addShowDefinitionEntry(iMenu, selectedWord);
}


void HistoryWidget::updateModel()
{
    m_model->removeRows(0, m_model->rowCount());
    if (m_forPlayer)
    {
        // Empty column
        m_model->setHeaderData(5, Qt::Horizontal, "", Qt::DisplayRole);
    }
    else
    {
        m_model->setHeaderData(5, Qt::Horizontal, _q("Player"), Qt::DisplayRole);
    }

    if (m_history != NULL && m_history->getSize() != 0)
    {
        // Should we align the rack with its solution?
        QSettings qs(ORGANIZATION, PACKAGE_NAME);
        bool align = qs.value(PrefsDialog::kINTF_ALIGN_HISTORY).toBool();

        if (!align)
            m_model->insertRow(0);

        for (unsigned int i = 0; i < m_history->getSize(); ++i)
        {
            int rowNum = m_model->rowCount();
            m_model->insertRow(rowNum);
            int prevRowNum;
            if (align)
                prevRowNum = rowNum;
            else
                prevRowNum = rowNum - 1;

            QColor color = Qt::black;

            const Turn& t = m_history->getTurn(i);
            const Move& m = t.getMove();

            // Set data common to all moves
            m_model->setData(m_model->index(prevRowNum, 0), i + 1);
            m_model->setData(m_model->index(prevRowNum, 1),
                             qfw(t.getPlayedRack().toString()));
            m_model->setData(m_model->index(rowNum, 4), m.getScore());
            if (!m_forPlayer && m_game != NULL)
            {
                const wstring &name = m_game->getPlayer(t.getPlayer()).getName();
                m_model->setData(m_model->index(rowNum, 5), qfw(name));
            }

            // Set the rest
            if (m.getType() == Move::VALID_ROUND)
            {
                const Round &r = m.getRound();
                wstring coord = r.getCoord().toString();
                m_model->setData(m_model->index(rowNum, 2), qfw(r.getWord()));
                m_model->setData(m_model->index(rowNum, 3), qfw(coord));
                color = Qt::black;
            }
            else if (m.getType() == Move::INVALID_WORD)
            {
                m_model->setData(m_model->index(rowNum, 2),
                                 "<" + qfw(m.getBadWord()) + ">");
                m_model->setData(m_model->index(rowNum, 3), qfw(m.getBadCoord()));
                color = Qt::red;
            }
            else if (m.getType() == Move::PASS)
            {
                m_model->setData(m_model->index(rowNum, 2), _q("(PASS)"));
                color = Qt::blue;
            }
            else
            {
                m_model->setData(m_model->index(rowNum, 2),
                                 "[-" + qfw(m.getChangedLetters()) + "]");
                color = Qt::blue;
            }

            // Set the color of the text
            for (int col = 0; col < 6; ++col)
            {
                int row = rowNum;
                if (!align && col < 2)
                    row = prevRowNum;
                m_model->setData(m_model->index(row, col),
                                 QBrush(color), Qt::ForegroundRole);
            }
        }
    }

    resizeColumnToContents(0);
    resizeColumnToContents(3);
    resizeColumnToContents(4);
    resizeColumnToContents(5);
}



HistoryTabWidget::HistoryTabWidget(QWidget *parent)
    : QTabWidget(parent), m_game(NULL)
{
    m_gameHistoryWidget = new HistoryWidget(NULL);
    insertTab(0, m_gameHistoryWidget, _q("&Game"));
    //setMinimalSize(300, 100);

    setGame(m_game);
}


void HistoryTabWidget::setGame(const PublicGame *iGame)
{
    m_game = iGame;

    // Keep only the Game tab, because it is nicer to have something, even
    // if it is empty
    int nbTabs = count();
    for (int i = nbTabs - 1; i > 0; --i)
    {
        setCurrentIndex(i);
        // Cut all the connections with the page (needed because removeTab()
        // doesn't really destroy the widget)
        disconnect(currentWidget());
        removeTab(i);
    }

    if (m_game == NULL)
    {
        // Tell the remaining tab that there is no more history to display
        m_gameHistoryWidget->setHistory(NULL);
    }
    else
    {
        // Refresh the Game tab
        m_gameHistoryWidget->setHistory(&m_game->getHistory(), m_game);
        QObject::connect(this, SIGNAL(refreshSignal()),
                         m_gameHistoryWidget, SLOT(refresh()));
        QObject::connect(m_gameHistoryWidget, SIGNAL(requestDefinition(QString)),
                         this, SIGNAL(requestDefinition(QString)));

        // In training mode, the players history is completely useless
        if (m_game->getMode() == PublicGame::kTRAINING)
            return;

        // Add one history tab per player
        for (unsigned int i = 0; i < m_game->getNbPlayers(); ++i)
        {
            const Player &player = m_game->getPlayer(i);
            HistoryWidget *h = new HistoryWidget(NULL);
            h->setHistory(&player.getHistory(), m_game, true);
            QObject::connect(this, SIGNAL(refreshSignal()), h, SLOT(refresh()));
            QObject::connect(h, SIGNAL(requestDefinition(QString)),
                             this, SIGNAL(requestDefinition(QString)));
            addTab(h, qfw(player.getName()));
        }
    }
}


void HistoryTabWidget::refresh()
{
    emit refreshSignal();
}


QSize HistoryTabWidget::sizeHint() const
{
    return QSize(500, 300);
}

