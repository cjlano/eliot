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

#ifndef STATS_WIDGET_H_
#define STATS_WIDGET_H_

#include <QtGui/QWidget>

#include "logging.h"


class PublicGame;
class Player;
class Turn;
class QTableView;
class QStandardItemModel;
class QVariant;
class QModelIndex;

class StatsWidget: public QWidget
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    explicit StatsWidget(QWidget *parent = 0, const PublicGame *iGame = NULL);

public slots:
    void setGame(const PublicGame *iGame = NULL);
    void refresh();

private:
    /// Encapsulated game, can be NULL
    const PublicGame *m_game;

    /// Model of the game
    QStandardItemModel *m_model;

    /// Table to display the model data
    QTableView *m_table;

    static const QColor WarningBrush;
    static const QColor PenaltyBrush;
    static const QColor SoloBrush;
    static const QColor PassBrush;
    static const QColor InvalidBrush;


    QModelIndex getIndex(int row, int col) const;
    QString getTooltip(const Turn &iTurn, const Turn &iGameTurn) const;

    void setModelSize(int rowCount, int colCount);
    void setModelHeader(int col, const QString &iText);
    void setModelText(const QModelIndex &iIndex, const QVariant &iData,
                      bool useBoldFont = false);
    void setModelTurnData(const QModelIndex &iIndex,
                          const Turn &iTurn, const Turn &iGameTurn);
    void setModelEventData(const QModelIndex &iIndex,
                           int iEvent, const Player &iPlayer);
};

#endif

