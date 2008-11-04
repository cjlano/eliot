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

#ifndef TRAINING_WIDGET_H_
#define TRAINING_WIDGET_H_

#include <QtGui/QWidget>
#include "ui/training_widget.ui.h"


class Game;
class Training;
class RackValidator;
class QStandardItemModel;
class QString;

class TrainingWidget: public QWidget, private Ui::TrainingWidget
{
    Q_OBJECT;

public:
    explicit TrainingWidget(QWidget *parent = 0);

public slots:
    void setGame(Game *iGame);
    void refresh();

signals:
    void gameUpdated();
    void notifyProblem(QString iMsg);
    void notifyInfo(QString iMsg);

protected:
    /// Define a default size
    virtual QSize sizeHint() const;

private slots:
    void enablePlayButton(const QItemSelection &, const QItemSelection &);
    void showPreview(const QItemSelection &, const QItemSelection &);

    // These slots are automatocally connected
    void on_lineEditRack_textEdited(const QString &iText);
    void on_pushButtonRack_clicked();
    void on_pushButtonComplement_clicked();
    void on_pushButtonSearch_clicked();
    void on_pushButtonPlay_clicked();
    void on_treeViewResults_doubleClicked(const QModelIndex &iIndex);

private:
    /// Encapsulated training game, can be NULL
    Training *m_game;

    /// Model of the search results
    QStandardItemModel *m_model;

    /// Validator for the rack edition
    RackValidator *m_validator;

    /// Force synchronizing the model with the contents of the search results
    void updateModel();
};

#endif

