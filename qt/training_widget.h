/*****************************************************************************
 * Eliot
 * Copyright (C) 2008-2011 Olivier Teulière
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
#include "logging.h"


class QStandardItemModel;
class QString;
class QPoint;
class CoordModel;
class CustomPopup;
class PlayWordMediator;
class PublicGame;

class TrainingWidget: public QWidget, private Ui::TrainingWidget
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    explicit TrainingWidget(QWidget *parent, CoordModel &iCoordModel, PublicGame *iGame);

public slots:
    void refresh();

signals:
    void gameUpdated();
    void notifyProblem(QString iMsg);
    void notifyInfo(QString iMsg);
    void rackUpdated(const QString &iRack);
    void requestDefinition(QString iWord);

protected:
    /// Define a default size
    virtual QSize sizeHint() const;

private slots:
    void enablePlayButton(const QItemSelection &, const QItemSelection &);
    void showPreview(const QItemSelection &, const QItemSelection &);

    void lockSizesChanged(bool checked);
    void populateMenu(QMenu &iMenu, const QPoint &iPoint);

    void onRackEdited(const QString &iText);
    void setNewRack();
    void completeRack();
    void search();
    void playSelectedWord();

private:
    /// Encapsulated training game, can be NULL
    PublicGame *m_game;

    /// Inidicate whether the columns should be resized after a search
    bool m_autoResizeColumns;

    /// Model of the search results
    QStandardItemModel *m_model;

    /// Mediator for the "play word" controls
    PlayWordMediator *m_mediator;

    /// Palette to write text in black
    QPalette blackPalette;

    /// Palette to write text in red
    QPalette redPalette;

    /// Popup menu for words definition
    CustomPopup *m_customPopup;

    /// Force synchronizing the model with the contents of the search results
    void updateModel();

    /// Helper method to set the rack
    void helperSetRack(bool iAll);
};

#endif

