/*****************************************************************************
 * Eliot
 * Copyright (C) 2012-2013 Olivier Teulière
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
#include <QtGui/QMessageBox>
#include <QtGui/QSortFilterProxyModel>

#include "topping_widget.h"
#include "play_word_mediator.h"
#include "hints_dialog.h"
#include "timer_widget.h"
#include "play_model.h"
#include "qtcommon.h"

#include "hints.h"
#include "dic.h"
#include "move.h"
#include "pldrack.h"
#include "player.h"
#include "public_game.h"
#include "game_exception.h"
#include "debug.h"

using namespace std;


INIT_LOGGER(qt, ToppingWidget);


ToppingWidget::ToppingWidget(QWidget *parent, PlayModel &iPlayModel,
                             TimerModel &iTimerModel, PublicGame *iGame)
    : QWidget(parent), m_game(iGame), m_autoResizeColumns(true),
    m_timerModel(&iTimerModel)
{
    setupUi(this);

    QObject::connect(&iPlayModel, SIGNAL(movePlayed(const wstring&, const wstring&)),
                     this, SLOT(playWord(const wstring&, const wstring&)));

    QHBoxLayout *layout = new QHBoxLayout(widgetContainer);
    layout->setContentsMargins(0, 0, 0, 0);

    TimerWidget *timerWidget = new TimerWidget(this, iTimerModel);
    timerWidget->setEnabled(false);
    //iTimerModel.setChronoMode(true);
    QObject::connect(&iTimerModel, SIGNAL(expired()),
                     this, SLOT(timeoutPenalty()));
    layout->addWidget(timerWidget);

    m_hintsDialog = new HintsDialog(this, true);
    QObject::connect(m_hintsDialog, SIGNAL(hintUsed(const AbstractHint&)),
                     this, SLOT(hintUsed(const AbstractHint&)));

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

    QObject::connect(pushButtonShuffle, SIGNAL(clicked()),
                     this, SLOT(shuffle()));

    // Associate the model to the view.
    // We use a proxy for easy sorting.
    m_model = new QStandardItemModel(this);
    QSortFilterProxyModel * proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSourceModel(m_model);
    tableViewMoves->setModel(proxyModel);

    m_model->setColumnCount(4);
    m_model->setHeaderData(0, Qt::Horizontal, _q("#"), Qt::DisplayRole);
    m_model->setHeaderData(1, Qt::Horizontal, _q("Word"), Qt::DisplayRole);
    m_model->setHeaderData(2, Qt::Horizontal, _q("Ref"), Qt::DisplayRole);
    m_model->setHeaderData(3, Qt::Horizontal, _q("Points"), Qt::DisplayRole);
    tableViewMoves->sortByColumn(3);
    // XXX: why is this needed? It is not needed in the ArbitrationWidget class
    tableViewMoves->horizontalHeader()->setSortIndicator(3, Qt::DescendingOrder);

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

    tableViewMoves->horizontalHeader()->resizeSection(1, 140);

    QObject::connect(pushButtonGetHints, SIGNAL(clicked()),
                     this, SLOT(showHintsDialog()));

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
        wstring rack = m_game->getCurrentPlayer().getCurrentRack().toString(PlayedRack::RACK_SIMPLE);
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

    if (m_game == NULL)
        return;

    const vector<Move> &triedMoves = m_game->toppingGetTriedMoves();
    for (unsigned int i = 0; i < triedMoves.size(); ++i)
    {
        const Move &m = triedMoves[i];
        int rowNum = m_model->rowCount();
        m_model->insertRow(rowNum);
        m_model->setData(m_model->index(rowNum, 0), i);
        if (m.isValid())
        {
            m_model->setData(m_model->index(rowNum, 1), qfw(m.getRound().getWord()));
            m_model->setData(m_model->index(rowNum, 2),
                             qfw(m.getRound().getCoord().toString()));
        }
        else
        {
            ASSERT(m.isInvalid(), "Unhandled move type");
            m_model->setData(m_model->index(rowNum, 1), qfw(m.getBadWord()));
            m_model->setData(m_model->index(rowNum, 2), qfw(m.getBadCoord()));
        }
        m_model->setData(m_model->index(rowNum, 3), m.getScore());
    }

    if (m_autoResizeColumns)
    {
        tableViewMoves->resizeColumnToContents(0);
        // tableViewMoves->resizeColumnToContents(1);
        tableViewMoves->resizeColumnToContents(2);
        tableViewMoves->resizeColumnToContents(3);
    }
}


void ToppingWidget::lockSizesChanged(bool checked)
{
    m_autoResizeColumns = !checked;
}


void ToppingWidget::shuffle()
{
    m_game->shuffleRack();
    emit gameUpdated();
}


void ToppingWidget::showHintsDialog()
{
    const Move &move = m_game->toppingGetTopMove();
    m_hintsDialog->setMove(move);
    // We don't care about the return value
    m_hintsDialog->exec();
}


void ToppingWidget::onNewTurn()
{
    m_timerModel->resetTimer();
    m_timerModel->startTimer();
}


void ToppingWidget::playWord(const wstring &iWord, const wstring &iCoord)
{
    ASSERT(m_game != NULL, "No game in progress");

    int elapsed = m_timerModel->getElapsed();
    m_game->toppingPlay(iWord, iCoord, elapsed);
    emit gameUpdated();
}


void ToppingWidget::hintUsed(const AbstractHint &iHint)
{
    LOG_INFO("Hint '" << iHint.getName() << "' used for a cost of " << iHint.getCost());
    m_game->toppingAddPenalty(iHint.getCost());
    emit gameUpdated();
}


void ToppingWidget::timeoutPenalty()
{
    // Show the solution to the player in a dialog box
    const Move &move = m_game->toppingGetTopMove();
    QMessageBox::information(this, "Eliot - " + _q("End of turn"),
                             _q("The allocated time for the turn has expired.\n"
                                "The top is %1 at %2 for %3 points.")
                             .arg(qfw(move.getRound().getWord()))
                             .arg(qfw(move.getRound().getCoord().toString()))
                             .arg(move.getScore()));

    // End the turn
    m_game->toppingTimeOut();
    emit gameUpdated();
}


QSize ToppingWidget::sizeHint() const
{
    return QSize(160, 300);
}

