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

#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <QMainWindow>

#include <ui/main_window.ui.h>


class Dictionary;
class Bag;
class Board;
class History;
class Game;
class NewGame;
class AuxWindow;

class MainWindow: public QMainWindow
{
    Q_OBJECT;

public:
    MainWindow(QWidget *iParent = 0);
    ~MainWindow();

signals:
    void dicChanged(QString iDicFile, QString iDicName);
    void gameChanged(const Game *iGame);
    void bagChanged(const Bag *iBag);
    void boardChanged(const Board *iBoard);
    void historyChanged(const History *iHistory);
    void rackChanged();

public slots:
    void playerPlays(unsigned int p, QString iWord, QString iCoord);
    void playerPasses(unsigned int p, QString iLetters);

private slots:
    /// Emit various specific signals
    void gameUpdated();

    void on_action_About_triggered();
    void on_action_Bag_triggered();
    void on_action_ChooseDic_triggered();
    void on_action_New_Game_triggered();

private:
    /// Display an error message to the user
    void displayErrorMsg(QString iMsg, QString iContext = QString());

    /// Current dictionary
    const Dictionary *m_dic;

    /// Current game
    Game *m_game;

    /// The UI file generated with Qt Designer
    Ui::MainWindow m_ui;

    /// Dialog for creating a new game
    NewGame *m_newGame;

    /// Bag window
    AuxWindow *m_bagWindow;
};

#endif

