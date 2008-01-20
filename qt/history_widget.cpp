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

#include <iostream>
#include <QtGui/QTreeView>
#include <QtGui/QStandardItemModel>

#include "history_widget.h"
#include "qtcommon.h"
#include "dic.h"
#include "tile.h"
#include "history.h"
#include "turn.h"
#include "move.h"

using namespace std;


HistoryWidget::HistoryWidget(QWidget *parent, const History *iHistory)
    : QTreeView(parent), m_history(iHistory)
{
    // Create the tree view
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setRootIsDecorated(false);

    // Associate the model to the view
    m_model = new QStandardItemModel(this);
    setModel(m_model);
    updateModel();
}


void HistoryWidget::setHistory(const History *iHistory)
{
    m_history = iHistory;
    updateModel();
}


void HistoryWidget::updateModel()
{
    m_model->clear();
    m_model->setColumnCount(6);
    m_model->setHeaderData(0, Qt::Horizontal, _q("Turn"), Qt::DisplayRole);
    m_model->setHeaderData(1, Qt::Horizontal, _q("Rack"), Qt::DisplayRole);
    m_model->setHeaderData(2, Qt::Horizontal, _q("Word"), Qt::DisplayRole);
    m_model->setHeaderData(3, Qt::Horizontal, _q("Ref"), Qt::DisplayRole);
    m_model->setHeaderData(4, Qt::Horizontal, _q("Points"), Qt::DisplayRole);
    m_model->setHeaderData(5, Qt::Horizontal, _q("Player"), Qt::DisplayRole);

    if (m_history != NULL)
    {
        for (unsigned int i = 0; i < m_history->getSize(); ++i)
        {
            int rowNum = m_model->rowCount();
            m_model->insertRow(rowNum);

            QColor color = Qt::black;

            const Turn& t = m_history->getTurn(i);
            const Move& m = t.getMove();
            // Set data common to all moves)
            m_model->setData(m_model->index(rowNum, 0), i + 1);
            m_model->setData(m_model->index(rowNum, 1),
                             qfw(t.getPlayedRack().toString()));
            m_model->setData(m_model->index(rowNum, 4), m.getScore());
            m_model->setData(m_model->index(rowNum, 5), t.getPlayer() + 1);
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
                m_model->setData(m_model->index(rowNum, col),
                                 QBrush(color), Qt::ForegroundRole);
            }
        }
    }

    resizeColumnToContents(0);
    resizeColumnToContents(3);
    resizeColumnToContents(4);
    resizeColumnToContents(5);
}

