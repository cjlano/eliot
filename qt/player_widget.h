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

#ifndef RACK_WIDGET_H_
#define RACK_WIDGET_H_

#include <QtGui/QWidget>
#include <QtGui/QTabWidget>
#include "ui/player_widget.ui.h"


class QLineEdit;
class Game;

class PlayerWidget: public QWidget, private Ui::PlayerWidget
{
    Q_OBJECT;

public:
    explicit PlayerWidget(QWidget *parent = 0,
                          unsigned int iPlayerNb = 0,
                          const Game *iGame = NULL);

signals:
    void playingWord(unsigned int iPlayer, QString iWord, QString iCoord);
    void passing(unsigned int iPlayer, QString iChangedLetters);

public slots:
    void refresh();

protected:
    virtual QSize sizeHint() const;

private slots:
    void on_pushButtonShuffle_clicked();
    void on_pushButtonPlay_clicked() { on_lineEditPlay_returnPressed(); }
    void on_pushButtonChange_clicked() { on_lineEditChange_returnPressed(); }
    void on_pushButtonPass_clicked() { on_lineEditChange_returnPressed(); }
    void on_lineEditPlay_textChanged();
    void on_lineEditChange_textChanged();
    void on_lineEditPlay_returnPressed();
    void on_lineEditChange_returnPressed();

private:
    /// Encapsulated game, can be NULL
    const Game *m_game;

    /// Encapsulated player, valid iff m_game is not NULL
    unsigned int m_player;

};


class PlayerTabWidget: public QTabWidget
{
    Q_OBJECT;

public:
    explicit PlayerTabWidget(QWidget *parent = 0);

public slots:
    void setGame(const Game *iGame);
    void refresh();

signals:
    void refreshSignal();
    void playingWord(unsigned int iPlayer, QString iWord, QString iCoord);
    void passing(unsigned int iPlayer, QString iChangedLetters);

private:
    /// Encapsulated game, can be NULL
    const Game *m_game;
};

#endif

