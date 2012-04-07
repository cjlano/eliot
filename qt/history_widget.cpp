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

#include "config.h"

#include <cmath>
#include <QtGui/QTreeView>
#include <QtGui/QTabWidget>
#include <QtGui/QStandardItemModel>
#include <QtGui/QMenu>
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
#include "game_params.h"

using namespace std;


INIT_LOGGER(qt, HistoryWidget);


HistoryWidget::HistoryWidget(QWidget *parent)
    : QTreeView(parent), m_history(NULL), m_forPlayer(false), m_isFreeGame(false)
{
    m_colTurn = 0;
    m_colRack = 1;
    m_colWord = 2;
    m_colRef = 3;
    m_colPoints = 4;
    m_colTotal = -1;
    m_colPercent = -1;
    m_colPlayer = -1;

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
    m_model->setColumnCount(5);
    m_model->setHeaderData(m_colTurn, Qt::Horizontal, _q("Turn"));
    m_model->setHeaderData(m_colRack, Qt::Horizontal, _q("Rack"));
    m_model->setHeaderData(m_colWord, Qt::Horizontal, _q("Word"));
    m_model->setHeaderData(m_colRef, Qt::Horizontal, _q("Ref"));
    m_model->setHeaderData(m_colPoints, Qt::Horizontal, _q("Points"));
}


void HistoryWidget::setHistory(const History *iHistory,
                               const PublicGame *iGame,
                               bool iIsForPlayer)
{
    m_history = iHistory;
    m_game = iGame;
    m_forPlayer = iIsForPlayer;
    m_isFreeGame = (iGame != 0 && iGame->getMode() == PublicGame::kFREEGAME);

    int currColumn = m_colPoints + 1;
    if (m_forPlayer)
    {
        m_colTotal = currColumn++;
        m_colPlayer = -1;
        m_colPercent = m_isFreeGame ? -1 : currColumn++;
    }
    else
    {
        m_colTotal = m_isFreeGame ? -1 : currColumn++;
        m_colPlayer = m_isFreeGame ? currColumn++ : -1;
        m_colPercent = -1;
    }
    m_model->setColumnCount(currColumn);

    m_model->setHeaderData(m_colTotal, Qt::Horizontal, _q("Total"));
    m_model->setHeaderData(m_colPercent, Qt::Horizontal, _q("Game %"));
    m_model->setHeaderData(m_colPlayer, Qt::Horizontal, _q("Player"));

    refresh();
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
    const QModelIndex &wordIndex = m_model->index(index.row(), m_colWord);
    QString selectedWord = m_model->data(wordIndex).toString();

    if (selectedWord != "")
        iMenu.addAction(m_customPopup->getShowDefinitionEntry(selectedWord));
}


void HistoryWidget::updateModel()
{
    m_model->removeRows(0, m_model->rowCount());

    if (m_history != NULL && m_history->getSize() != 0)
    {
        // Should we align the rack with its solution?
        QSettings qs;
        bool align = qs.value(PrefsDialog::kINTF_ALIGN_HISTORY).toBool();

        if (!align)
            m_model->insertRow(0);

        int totalScore = 0;
        int gameScore = 0;
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
            setCellData(prevRowNum, m_colTurn, i + 1);
            setCellData(prevRowNum, m_colRack, qfw(t.getPlayedRack().toString()));
            setCellData(rowNum, m_colPoints, m.getScore());
            totalScore += m.getScore();
            setCellData(rowNum, m_colTotal, totalScore);
            if (m_game->getHistory().getSize() > i)
            {
                gameScore += m_game->getHistory().getTurn(i).getMove().getScore();
            }
            if (gameScore != 0)
            {
                int percentage = totalScore * 100 / gameScore;
                setCellData(rowNum, m_colPercent, QString("%1%").arg(percentage));
            }
            if (m_isFreeGame)
            {
                const wstring &name = m_game->getPlayer(t.getPlayer()).getName();
                setCellData(rowNum, m_colPlayer, qfw(name));
            }

            // Set the rest
            if (m.getType() == Move::VALID_ROUND)
            {
                const Round &r = m.getRound();
                wstring coord = r.getCoord().toString();
                setCellData(rowNum, m_colWord, qfw(r.getWord()));
                setCellData(rowNum, m_colRef, qfw(coord));
                color = Qt::black;
            }
            else if (m.getType() == Move::INVALID_WORD)
            {
                setCellData(rowNum, m_colWord, "<" + qfw(m.getBadWord()) + ">");
                setCellData(rowNum, m_colRef, qfw(m.getBadCoord()));
                color = Qt::red;
            }
            else if (m.getType() == Move::NO_MOVE)
            {
                setCellData(rowNum, m_colWord, _q("(NO MOVE)"));
                color = Qt::blue;
            }
            else if (m.getType() == Move::PASS)
            {
                setCellData(rowNum, m_colWord, _q("(PASS)"));
                color = Qt::blue;
            }
            else
            {
                setCellData(rowNum, m_colWord,
                            "[-" + qfw(m.getChangedLetters()) + "]");
                color = Qt::blue;
            }

            // Set the color of the text
            for (int col = 0; col < m_model->columnCount(); ++col)
            {
                int row = rowNum;
                if (!align && col < 2)
                    row = prevRowNum;
                m_model->setData(m_model->index(row, col),
                                 QBrush(color), Qt::ForegroundRole);
            }
        }
    }

    resizeColumnToContents(m_colTurn);
    resizeColumnToContents(m_colRef);
    resizeColumnToContents(m_colPoints);
    resizeColumnToContents(m_colTotal);
    resizeColumnToContents(m_colPlayer);
}


void HistoryWidget::setCellData(int iRow, int iCol, const QVariant &iData)
{
    m_model->setData(m_model->index(iRow, iCol), iData);
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
        m_gameHistoryWidget->setHistory(NULL, NULL, false);
    }
    else
    {
        // Refresh the Game tab
        m_gameHistoryWidget->setHistory(&m_game->getHistory(), m_game, false);
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

