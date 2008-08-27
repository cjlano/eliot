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
class PrefsDialog;
class AuxWindow;

class MainWindow: public QMainWindow
{
    Q_OBJECT;

public:
    MainWindow(QWidget *iParent = 0);
    ~MainWindow();

signals:
    void dicChanged(const Dictionary *iDic);
    void gameChanged(const Game *iGame);
    void gameChangedNonConst(Game *iGame);
    void gameUpdated();

public slots:
    /// Display an error message to the user
    void displayErrorMsg(QString iMsg, QString iContext = "");
    void displayInfoMsg(QString iMsg);

protected:
    /// Handler for close events
    virtual void closeEvent(QCloseEvent * e);

private slots:
    void on_action_GameNew_triggered();
    void on_action_GameLoad_triggered();
    void on_action_GameSaveAs_triggered();
    void on_action_GamePrint_triggered();
    void on_action_SettingsChooseDic_triggered();
    void on_action_SettingsPreferences_triggered();
    void on_action_WindowsBag_triggered();
    void on_action_WindowsBoard_triggered();
    void on_action_WindowsHistory_triggered();
    void on_action_WindowsDicTools_triggered();
    void on_action_HelpAbout_triggered();

    /**
     * Perform several updates when the game changes (title bar, status bar,
     * grey out some menu items, ...)
     */
    void updateForGame(const Game *iGame);

private:
    /// Current dictionary
    const Dictionary *m_dic;

    /// Current game
    Game *m_game;

    /// The UI file generated with Qt Designer
    Ui::MainWindow m_ui;

    /// Dialog for creating a new game
    NewGame *m_newGameDialog;

    /// Dialog for the preferences
    PrefsDialog *m_prefsDialog;

    /// Auxiliary windows
    //@{
    AuxWindow *m_bagWindow;
    AuxWindow *m_boardWindow;
    AuxWindow *m_historyWindow;
    AuxWindow *m_dicToolsWindow;
    //@}

    /// Destroy the current game (if any) and the associated widgets
    void destroyCurrentGame();

};

#endif

