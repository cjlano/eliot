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

#include <QtGui/QTableView>
#include <QtGui/QHeaderView>
#include <QtGui/QStandardItemModel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtCore/QLocale>

#include "stats_widget.h"
#include "qtcommon.h"
#include "public_game.h"
#include "player.h"
#include "history.h"
#include "turn.h"
#include "game_params.h"
#include "settings.h"

using namespace std;


INIT_LOGGER(qt, StatsWidget);


const QColor StatsWidget::WarningBrush(220, 220, 0);
const QColor StatsWidget::PenaltyBrush(220, 120, 0);
const QColor StatsWidget::SoloBrush(0, 200, 0);
const QColor StatsWidget::PassBrush(210, 210, 210);
const QColor StatsWidget::InvalidBrush(255, 0, 0);


const bool HORIZONTAL = false;


StatsWidget::StatsWidget(QWidget *parent, const PublicGame *iGame)
    : QWidget(parent), m_game(iGame)
{
    // Layout
    setLayout(new QVBoxLayout);

    // Create the table
    m_table = new QTableView(this);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setHighlightSections(false);
    m_table->verticalHeader()->setHighlightSections(false);
    m_table->horizontalHeader()->setMinimumSectionSize(15);
    m_table->verticalHeader()->setMinimumSectionSize(15);
    //m_table->setSortingEnabled(true);
    layout()->addWidget(m_table);

    m_model = new QStandardItemModel();
    m_table->setModel(m_model);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    refresh();
}


void StatsWidget::setGame(const PublicGame *iGame)
{
    m_game = iGame;
    refresh();
}


void StatsWidget::refresh()
{
    m_model->clear();

    unsigned histSize = m_game == NULL ? 0 : m_game->getHistory().getSize();
    unsigned nbPlayers = m_game == NULL ? 0 : m_game->getNbPlayers();

    setModelSize(nbPlayers + 1, histSize + 9);

    // Some fields are displayed only in some cases
    const bool isArbit = m_game != NULL &&
        m_game->getParams().getMode() == GameParams::kARBITRATION;
    const bool canHaveSolos = m_game != NULL &&
        m_game->getParams().getMode() == GameParams::kDUPLICATE &&
        Settings::Instance().getInt("duplicate.solo-players") <= (int)m_game->getNbPlayers();

    // Define columns (or rows, depending on the orientation)
    int col = 0;
    setSectionHidden(col, !isArbit);
    setModelHeader(col++, _q("Table"), false);

    for (unsigned i = 1; i <= histSize; ++i)
        setModelHeader(col++, QString("#%1").arg(i), false);

    setSectionHidden(col, !isArbit && !canHaveSolos);
    setModelHeader(col++, _q("Sub-total"), false);
    setSectionHidden(col, !isArbit);
    setModelHeader(col++, _q("Warnings"), false);
    setSectionHidden(col, !isArbit);
    setModelHeader(col++, _q("Penalties"), false);
    setSectionHidden(col, !isArbit && !canHaveSolos);
    setModelHeader(col++, _q("Solo points"), false);

    setModelHeader(col++, _q("Total"), false);
    setModelHeader(col++, _q("Diff"), false);
    setModelHeader(col++, _q("Game %"), false);
    setModelHeader(col++, _q("Ranking"), false);

    // Define the header for the Game pseudo-player
    setModelHeader(0, _q("Game"), true);

    if (m_game == NULL)
        return;

    QLocale locale;
    const History &gHistory = m_game->getHistory();

    // Game data
    int gameTotal = 0;
    {
        const int row = 0;
        int col = 0;
        // Skip the table number
        ++col;
        int score = 0;
        for (unsigned j = 0; j < gHistory.getSize(); ++j)
        {
            setModelTurnData(getIndex(row, col++), gHistory.getTurn(j), gHistory.getTurn(j));
            score += gHistory.getTurn(j).getMove().getScore();
        }
        setModelText(getIndex(row, col++), score, true);
        // Skip the events columns
        col += 3;
        setModelText(getIndex(row, col++), score, true);
        // Skip the diff column
        ++col;
        setModelText(getIndex(row, col++), locale.toString((double)100, 'f', 1) + "%", true);
        // Skip the ranking column
        ++col;

        gameTotal = score;
    }

    // Players data
    for (unsigned i = 0; i < nbPlayers; ++i)
    {
        const Player &player = m_game->getPlayer(i);
        int col = 0;
        setModelHeader(i + 1, qfw(player.getName()), true);

        // Table number
        setModelText(getIndex(i + 1, col++), player.getTableNb());

        int score = 0;
        // Normal turns
        for (unsigned j = 0; j < gHistory.getSize(); ++j)
        {
            const History &pHistory = player.getHistory();
            setModelTurnData(getIndex(i + 1, col++),
                             pHistory.getTurn(j), gHistory.getTurn(j));
            score += pHistory.getTurn(j).getMove().getScore();
        }

        // Sub-total
        setModelText(getIndex(i + 1, col++), score, score >= gameTotal);

        // Events columns
        for (int j = 0; j <= 2; ++j)
        {
            setModelEventData(getIndex(i + 1, col++), j, player);
        }

        // Final score
        score += player.getSoloPoints() + player.getPenaltyPoints();
        setModelText(getIndex(i + 1, col++), score, score >= gameTotal);

        // Diff with game total
        setModelText(getIndex(i + 1, col++), score - gameTotal);
        // Global score percentage
        setModelText(getIndex(i + 1, col++),
                     locale.toString(100. * score / gameTotal, 'f', 1) + "%",
                     score >= gameTotal);

        // Ranking
        // FIXME: quadratic complexity, we can probably do better
        int rank = 1;
        for (unsigned j = 0; j < nbPlayers; ++j)
        {
                if (i == j)
                    continue;
            if (player.getPoints() < m_game->getPlayer(j).getPoints())
                ++rank;
        }
        setModelText(getIndex(i + 1, col++), rank, rank == 1);
    }

    // Resize
    m_table->resizeRowsToContents();
    m_table->resizeColumnsToContents();
}


QModelIndex StatsWidget::getIndex(int row, int col) const
{
    if (HORIZONTAL)
        return m_model->index(row, col);
    else
        return m_model->index(col, row);
}


void StatsWidget::setSectionHidden(int index, bool iHide)
{
    if (HORIZONTAL)
        m_table->setColumnHidden(index, iHide);
    else
        m_table->setRowHidden(index, iHide);
}


void StatsWidget::setModelSize(int rowCount, int colCount)
{
    m_model->setRowCount(HORIZONTAL ? rowCount : colCount);
    m_model->setColumnCount(HORIZONTAL ? colCount : rowCount);
}


void StatsWidget::setModelHeader(int index, const QString &iText, bool iPlayerNames)
{
    Qt::Orientation orientation;
    if ((HORIZONTAL && iPlayerNames) || (!HORIZONTAL && !iPlayerNames))
        orientation = Qt::Vertical;
    else
        orientation = Qt::Horizontal;
    m_model->setHeaderData(index, orientation, iText);
    m_model->setHeaderData(index, orientation, Qt::AlignCenter, Qt::TextAlignmentRole);
}


void StatsWidget::setModelText(const QModelIndex &iIndex,
                               const QVariant &iData, bool useBoldFont)
{
    m_model->setData(iIndex, iData);
    m_model->setData(iIndex, Qt::AlignCenter, Qt::TextAlignmentRole);
    if (useBoldFont)
    {
        QFont boldFont = font();
        boldFont.setBold(true);
        m_model->setData(iIndex, boldFont, Qt::FontRole);
    }
}


void StatsWidget::setModelTurnData(const QModelIndex &iIndex,
                                   const Turn &iTurn, const Turn &iGameTurn)
{
    // Set the text (score for the turn)
    if (!iTurn.getMove().isNull())
    {
        int score = iTurn.getMove().getScore();
        setModelText(iIndex, QVariant(score),
                     score >= iGameTurn.getMove().getScore());
    }

    // Set the background c constolor
    if (iTurn.getPenaltyPoints() != 0)
        m_model->setData(iIndex, PenaltyBrush, Qt::BackgroundRole);
    else if (iTurn.getWarningsNb() != 0)
        m_model->setData(iIndex, WarningBrush, Qt::BackgroundRole);
    else if (iTurn.getSoloPoints() != 0)
        m_model->setData(iIndex, SoloBrush, Qt::BackgroundRole);
    else if (iTurn.getMove().isNull())
        m_model->setData(iIndex, PassBrush, Qt::BackgroundRole);

    // Set the foreground color
    if (iTurn.getMove().isInvalid())
        m_model->setData(iIndex, InvalidBrush, Qt::ForegroundRole);

    // Set the tooltip
    const QString &tooltip = getTooltip(iTurn, iGameTurn);
    m_model->setData(iIndex, tooltip, Qt::ToolTipRole);
}


void StatsWidget::setModelEventData(const QModelIndex &iIndex,
                                    int iEvent, const Player &iPlayer)
{
    QVariant text;
    if (iEvent == 0 && iPlayer.getWarningsNb() != 0)
        text = iPlayer.getWarningsNb();
    else if (iEvent == 1 && iPlayer.getPenaltyPoints() != 0)
        text = iPlayer.getPenaltyPoints();
    else if (iEvent == 2 && iPlayer.getSoloPoints() != 0)
        text = iPlayer.getSoloPoints();
    setModelText(iIndex, text);
}


QString StatsWidget::getTooltip(const Turn &iTurn, const Turn &iGameTurn) const
{
    QString tooltip = _q("Rack: %1").arg(qfw(iTurn.getPlayedRack().toString()));
    const Move &move = iTurn.getMove();
    if (move.isValid())
    {
        tooltip += "\n" + _q("Word: %1").arg(qfw(move.getRound().getWord()));
        tooltip += "\n" + _q("Ref: %1").arg(qfw(move.getRound().getCoord().toString()));
    }
    else if (move.isInvalid())
    {
        tooltip += "\n" + _q("Invalid move (%1 - %2)")
            .arg(qfw(move.getBadWord()))
            .arg(qfw(move.getBadCoord()));
    }
    else if (move.isChangeLetters())
    {
        tooltip += "\n" + _q("Changing letters: %1").arg(qfw(move.getChangedLetters()));
    }
    else if (move.isPass())
    {
        tooltip += "\n" + _q("Passing turn");
    }
    else
    {
        tooltip += "\n" + _q("No move");
    }

    // Points
    int score = move.getScore();
    int gameScore = iGameTurn.getMove().getScore();
    if (move.isNull())
        tooltip += _q("Points: %1").arg(score);
    else
    {
        QString scoreString = _q("Points: %1 (%2)").arg(score);
        if (score == gameScore)
            tooltip += "\n" + scoreString.arg(_q("max"));
        else
            tooltip += "\n" + scoreString.arg(score - gameScore);
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


