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

#ifndef PLAYER_WIDGET_H_
#define PLAYER_WIDGET_H_

#include <QtGui/QWidget>
#include <QtGui/QTabWidget>

#include "ui/player_widget.ui.h"
#include "logging.h"


class QLineEdit;
class PublicGame;
class PlayWordMediator;
class CoordModel;
class Coord;

class PlayerWidget: public QWidget, private Ui::PlayerWidget
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    explicit PlayerWidget(QWidget *parent,
                          CoordModel &iCoordModel,
                          unsigned int iPlayerNb = 0,
                          PublicGame *iGame = NULL);

signals:
    void gameUpdated();
    void notifyProblem(QString iMsg);
    void notifyInfo(QString iMsg);

public slots:
    void refresh();

protected:
    virtual QSize sizeHint() const;

private slots:
    void shuffle();
    void pass();
    void changeLetters();
    void enableChangeButton();

private:
    /// Encapsulated game, can be NULL
    PublicGame *m_game;

    /// Mediator for the "play word" controls
    PlayWordMediator *m_mediator;

    /// Encapsulated player, valid iff m_game is not NULL
    unsigned int m_player;

    void helperChangePass(QString inputLetters = "");

};


class PlayerTabWidget: public QTabWidget
{
    Q_OBJECT;

public:
    explicit PlayerTabWidget(CoordModel &iCoordModel, QWidget *parent = 0);

public slots:
    void setGame(PublicGame *iGame);
    void refresh();

signals:
    void refreshSignal();
    void gameUpdated();
    void notifyProblem(QString iMsg);
    void notifyInfo(QString iMsg);
    void requestDefinition(QString iWord);

private slots:
    void changeCurrentPlayer(int);

private:
    /// Encapsulated game, can be NULL
    PublicGame *m_game;

    /// Model for the word coordinates
    CoordModel &m_coordModel;
};

#endif

