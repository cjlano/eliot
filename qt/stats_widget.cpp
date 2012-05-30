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

#include <QtGui/QTableWidget>
#include <QtGui/QHeaderView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>

#include "stats_widget.h"
#include "qtcommon.h"
#include "public_game.h"
#include "player.h"
#include "history.h"
#include "turn.h"

using namespace std;


INIT_LOGGER(qt, StatsWidget);


StatsWidget::StatsWidget(QWidget *parent, const PublicGame *iGame)
    : QWidget(parent), m_game(iGame)
{
    // Layout
    setLayout(new QVBoxLayout);

    // Create the table
    m_table = new QTableWidget(this);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
//     m_table->setSortingEnabled(true);
    //m_table->setAlternatingRowColors(true);
    m_table->horizontalHeader()->setMinimumSectionSize(15);
    m_table->verticalHeader()->setVisible(false);
    layout()->addWidget(m_table);

    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setSizePolicy(policy);

    refresh();
}


void StatsWidget::setGame(const PublicGame *iGame)
{
    m_game = iGame;
    refresh();
}


void StatsWidget::refresh()
{
    m_table->clear();
    if (m_game == NULL)
        return;

    const History &gHistory = m_game->getHistory();
    m_table->setColumnCount(gHistory.getSize() + 8);

    int col = 0;
    m_table->setHorizontalHeaderItem(col++, createItem(_q("Table")));
    m_table->setHorizontalHeaderItem(col++, createItem(_q("Player")));
    for (unsigned i = 1; i <= gHistory.getSize(); ++i)
        m_table->setHorizontalHeaderItem(col++, createItem(QString("%1").arg(i)));
    m_table->setHorizontalHeaderItem(col++, createItem(_q("Sub-total")));
    m_table->setHorizontalHeaderItem(col++, createItem(_q("Warnings")));
    m_table->setHorizontalHeaderItem(col++, createItem(_q("Penalties")));
    m_table->setHorizontalHeaderItem(col++, createItem(_q("Bonuses")));
    m_table->setHorizontalHeaderItem(col++, createItem(_q("Total")));
    m_table->setHorizontalHeaderItem(col++, createItem(_q("Diff")));
    m_table->setHorizontalHeaderItem(col++, createItem(_q("Game %")));

    unsigned nbPlayers = m_game->getNbPlayers();
    m_table->setRowCount(nbPlayers + 1);

    // Game data
    int gameTotal = 0;
    {
        int col = 0;
        // Skip the table number
        ++col;
        m_table->setItem(0, col++, createItem(_q("Game")));
        int score = 0;
        for (unsigned j = 0; j < gHistory.getSize(); ++j)
        {
            m_table->setCellWidget(0, col++, createCell(gHistory.getTurn(j), gHistory.getTurn(j)));
            score += gHistory.getTurn(j).getMove().getScore();
        }
        m_table->setItem(0, col++, createItem(QString("%1").arg(score)));
        // Skip the events columns
        col += 3;
        m_table->setItem(0, col++, createItem(QString("%1").arg(score)));
        // Skip the diff column
        col += 1;
        m_table->setItem(0, col++, createItem("100%"));

        gameTotal = score;
    }

    // Players data
    for (unsigned i = 0; i < nbPlayers; ++i)
    {
        const Player &player = m_game->getPlayer(i);
        int col = 0;

        // Table number
        m_table->setItem(i + 1, col++, createItem(QString("%1").arg(player.getTableNb())));

        // Player name
        m_table->setItem(i + 1, col++, createItem(qfw(player.getName())));

        int score = 0;
        // Normal turns
        for (unsigned j = 0; j < gHistory.getSize(); ++j)
        {
            const History &pHistory = player.getHistory();
            m_table->setCellWidget(i + 1, col++, createCell(pHistory.getTurn(j), gHistory.getTurn(j)));
            score += pHistory.getTurn(j).getMove().getScore();
        }

        // Sub-total
        m_table->setItem(i + 1, col++, createItem(QString("%1").arg(score)));

        // Events columns
        for (int j = 0; j <= 2; ++j)
        {
            m_table->setCellWidget(i + 1, col++, createEventCell(j, player));
        }

        // Final score
        score += player.getSoloPoints() + player.getPenaltyPoints();
        m_table->setItem(i + 1, col++, createItem(QString("%1").arg(score)));

        // Diff with game total
        m_table->setItem(i + 1, col++, createItem(QString("%1").arg(gameTotal - score)));
        // Global score percentage
        m_table->setItem(i + 1, col++, createItem(QString("%1%").arg(100. * score / gameTotal)));
    }

    // Resize
    for (int i = 0; i < m_table->columnCount(); ++i)
    {
        if (i > 0)
        {
            m_table->horizontalHeader()->resizeSection(i, 30);
        }
        m_table->resizeColumnToContents(i);
    }
    for (int i = 0; i < m_table->rowCount(); ++i)
    {
        m_table->resizeRowToContents(i);
    }
}


QTableWidgetItem * StatsWidget::createItem(QString iText)
{
    QTableWidgetItem *item = new QTableWidgetItem(iText);
    item->setTextAlignment(Qt::AlignCenter);
    return item;
}


QString StatsWidget::getScore(const Turn &iTurn)
{
    if (iTurn.getMove().isNull())
        return "";
    return QString("%1").arg(iTurn.getMove().getScore());
}


QString StatsWidget::getFlags(const Turn &iTurn)
{
    QString flags(iTurn.getWarningsNb(), QChar('W'));
    if (iTurn.getPenaltyPoints() != 0)
    {
        if (flags != "")
            flags += "-";
        flags += "P";
    }
    if (iTurn.getSoloPoints() != 0)
    {
        if (flags != "")
            flags += "-";
        flags += "S";
    }

    return flags;
}


QString StatsWidget::getTooltip(const Turn &iTurn, const Turn &iGameTurn)
{
    QString tooltip = _q("Rack: %1").arg(qfw(iTurn.getPlayedRack().toString()));
    if (iTurn.getMove().isValid())
    {
        tooltip += "\n" + _q("Word: %1").arg(qfw(iTurn.getMove().getRound().getWord()));
        tooltip += "\n" + _q("Ref: %1").arg(qfw(iTurn.getMove().getRound().getCoord().toString()));
    }
    if (&iTurn != &iGameTurn)
    {
        unsigned score = iTurn.getMove().getScore();
        unsigned gameScore = iGameTurn.getMove().getScore();
        tooltip += "\n" + _q("Points: %1 (%2%)").arg(score).arg(score * 100 / gameScore);
    }
    if (iTurn.getWarningsNb())
    {
        tooltip += "\n" + _q("Warnings: %1").arg(iTurn.getWarningsNb());
    }
    if (iTurn.getPenaltyPoints())
    {
        tooltip += "\n" + _q("Penalties: %1").arg(iTurn.getPenaltyPoints());
    }
    if (iTurn.getSoloPoints())
    {
        tooltip += "\n" + _q("Solo: %1").arg(iTurn.getSoloPoints());
    }
    return tooltip;
}


QWidget * StatsWidget::createCell(const Turn &iTurn, const Turn &iGameTurn)
{
    QWidget *widget = new QWidget;
    QLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(2, 2, 2, 2);
    widget->setLayout(layout);

    QLabel *label1 = new QLabel;
    layout->addWidget(label1);
    QString score = getScore(iTurn);
    label1->setText(score);
    label1->setAlignment(Qt::AlignCenter);

    QString flags = getFlags(iTurn);
    if (flags != "")
    {
        QLabel *label2 = new QLabel;
        layout->addWidget(label2);
        label2->setText(flags);
        label2->setAlignment(Qt::AlignCenter);
    }

    QString tooltip = getTooltip(iTurn, iGameTurn);
    widget->setToolTip(tooltip);
    return widget;
}


QWidget * StatsWidget::createEventCell(int iEvent, const Player &iPlayer)
{
    QLabel * label = new QLabel;
    label->setAlignment(Qt::AlignCenter);
    if (iEvent == 0 && iPlayer.getWarningsNb() != 0)
        label->setText(QString("%1").arg(iPlayer.getWarningsNb()));
    else if (iEvent == 1 && iPlayer.getPenaltyPoints() != 0)
        label->setText(QString("%1").arg(iPlayer.getPenaltyPoints()));
    else if (iEvent == 2 && iPlayer.getSoloPoints() != 0)
        label->setText(QString("%1").arg(iPlayer.getSoloPoints()));
    return label;
}


