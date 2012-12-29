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

#include <QtGui/QStandardItemModel>
#include <QtGui/QMenu>
#include <QtGui/QHeaderView>

#include "topping_widget.h"
#include "qtcommon.h"
#include "play_word_mediator.h"
#include "validator_factory.h"

#include "player.h"
#include "dic.h"
#include "move.h"
#include "public_game.h"
#include "game_exception.h"
#include "debug.h"

using namespace std;


INIT_LOGGER(qt, ToppingWidget);


ToppingWidget::ToppingWidget(QWidget *parent, PlayModel &iPlayModel, PublicGame *iGame)
    : QWidget(parent), m_game(iGame), m_autoResizeColumns(true)
{
    setupUi(this);
    tableViewMoves->setAlternatingRowColors(true);

    blackPalette = lineEditRack->palette();
    redPalette = lineEditRack->palette();
    redPalette.setColor(QPalette::Text, Qt::red);

    // Use the mediator
    m_mediator = new PlayWordMediator(this, *lineEditPlay, *lineEditCoords,
                                      *lineEditPoints, *pushButtonPlay,
                                      iPlayModel, m_game);
    QObject::connect(m_mediator, SIGNAL(gameUpdated()),
                     this, SIGNAL(gameUpdated()));
    QObject::connect(m_mediator, SIGNAL(notifyProblem(QString)),
                     this, SIGNAL(notifyProblem(QString)));

    // Associate the model to the view
    m_model = new QStandardItemModel(this);
    tableViewMoves->setModel(m_model);
    m_model->setColumnCount(3);
    m_model->setHeaderData(0, Qt::Horizontal, _q("Word"), Qt::DisplayRole);
    m_model->setHeaderData(1, Qt::Horizontal, _q("Ref"), Qt::DisplayRole);
    m_model->setHeaderData(2, Qt::Horizontal, _q("Points"), Qt::DisplayRole);

    QObject::connect(lineEditRack, SIGNAL(returnPressed()),
                     this, SLOT(search()));

    // Add a context menu to the table header
    QAction *lockSizesAction = new QAction(_q("Lock columns sizes"), this);
    lockSizesAction->setCheckable(true);
    lockSizesAction->setStatusTip(_q("Disable auto-resizing of the columns"));
    tableViewMoves->horizontalHeader()->addAction(lockSizesAction);
    tableViewMoves->horizontalHeader()->setContextMenuPolicy(Qt::ActionsContextMenu);
    QObject::connect(lockSizesAction, SIGNAL(toggled(bool)),
                     this, SLOT(lockSizesChanged(bool)));

    // Allow very thin columns
    tableViewMoves->horizontalHeader()->setMinimumSectionSize(1);

    refresh();
}


void ToppingWidget::refresh()
{
    updateModel();
    if (m_game == NULL)
    {
        lineEditRack->setText("");
        lineEditRack->setEnabled(false);
        pushButtonPlay->setEnabled(false);
    }
    else
    {
        wstring rack = m_game->getPlayer(0).getCurrentRack().toString(PlayedRack::RACK_SIMPLE);
        // Update the rack only if it is needed, to avoid losing cursor position
        if (qfw(rack) != lineEditRack->text())
            lineEditRack->setText(qfw(rack));
        lineEditPlay->clear();
        lineEditCoords->clear();
        lineEditRack->setEnabled(true);
        // Do not allow entering a move when displaying an old turn
        setEnabled(m_game->isLastTurn());
    }
}


void ToppingWidget::updateModel()
{
    // Clear the table
    m_model->removeRows(0, m_model->rowCount());

    // Force the sort column
    tableViewMoves->sortByColumn(2);

    if (m_game == NULL)
        return;

    const vector<Move> &triedMoves = m_game->toppingGetTriedMoves();
    for (unsigned int i = 0; i < triedMoves.size(); ++i)
    {
        const Move &m = triedMoves[i];
        int rowNum = m_model->rowCount();
        m_model->insertRow(rowNum);
        if (m.isValid())
        {
            m_model->setData(m_model->index(rowNum, 0), qfw(m.getRound().getWord()));
            m_model->setData(m_model->index(rowNum, 1),
                             qfw(m.getRound().getCoord().toString()));
        }
        else
        {
            ASSERT(m.isInvalid(), "Unhandled move type");
            m_model->setData(m_model->index(rowNum, 0), qfw(m.getBadWord()));
            m_model->setData(m_model->index(rowNum, 1), qfw(m.getBadCoord()));
        }
        m_model->setData(m_model->index(rowNum, 2), m.getScore());
    }

    if (m_autoResizeColumns)
    {
        tableViewMoves->resizeColumnToContents(0);
        tableViewMoves->resizeColumnToContents(1);
        tableViewMoves->resizeColumnToContents(2);
    }
}


void ToppingWidget::lockSizesChanged(bool checked)
{
    m_autoResizeColumns = !checked;
}


QSize ToppingWidget::sizeHint() const
{
    return QSize(160, 300);
}

