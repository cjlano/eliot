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

#ifndef HISTORY_WIDGET_H_
#define HISTORY_WIDGET_H_

#include <QtGui/QTreeView>


class History;
class QStandardItemModel;

class HistoryWidget: public QTreeView
{
    Q_OBJECT;

public:
    explicit HistoryWidget(QWidget *parent = 0, const History *iHistory = NULL);

public slots:
    void setHistory(const History *iHistory = NULL);

private:
    /// Encapsulated history, can be NULL
    const History *m_history;

    /// Model of the history
    QStandardItemModel *m_model;

    /// Force synchronizing the model with the contents of the history
    void updateModel();
};

#endif

