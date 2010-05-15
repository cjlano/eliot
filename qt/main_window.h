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
#include "coord_model.h"


class Dictionary;
class Bag;
class Board;
class History;
class PublicGame;
class NewGame;
class PrefsDialog;
class PlayerTabWidget;
class AuxWindow;
class QLabel;
class QAction;

class MainWindow: public QMainWindow
{
    Q_OBJECT;

public:
    MainWindow(QWidget *iParent = 0);
    ~MainWindow();

signals:
    void dicChanged(const Dictionary *iDic);
    void gameChanged(const PublicGame *iGame);
    void gameChangedNonConst(PublicGame *iGame);
    void gameUpdated();

public slots:
    /// Display an error message to the user
    void displayErrorMsg(QString iMsg, QString iContext = "");
    void displayInfoMsg(QString iMsg);

protected:
    /// Handler for close events
    virtual void closeEvent(QCloseEvent * e);

private slots:
    void onGameNew();
    void onGameLoad();
    void onGameSaveAs();
    void onGamePrint();
    void onSettingsChooseDic();
    void onSettingsCreateDic();
    void onSettingsPreferences();
    void onWindowsToolbar();
    void onWindowsBag();
    void onWindowsBoard();
    void onWindowsHistory();
    void onWindowsDicTools();
    void onHelpAbout();

    void onHistoryFirstTurn();
    void onHistoryPrevTurn();
    void onHistoryNextTurn();
    void onHistoryLastTurn();
    void onHistoryReplayTurn();

    /** Perform some updates when the game is updated */
    void refresh();

    /** Perform work when the preferences were updated */
    void prefsUpdated();

    /**
     * Perform several updates when the game changes (title bar, status bar,
     * grey out some menu items, ...)
     */
    void updateForGame(const PublicGame *iGame);

    /// Update the status bar contents
    void updateStatusBar(const Dictionary *iDic);

private:
    /// Current dictionary
    const Dictionary *m_dic;

    /// Current game
    PublicGame *m_game;

    /// The UI file generated with Qt Designer
    Ui::MainWindow m_ui;

    /// Dialog for creating a new game
    NewGame *m_newGameDialog;

    /// Dialog for the preferences
    PrefsDialog *m_prefsDialog;

    /// Widget for the players
    PlayerTabWidget *m_playersWidget;

    /// Actions enabled or disabled depending on the game state
    QAction *m_actionGamePrint;
    QAction *m_actionGameSaveAs;
    QAction *m_actionHistoryPrevTurn;
    QAction *m_actionHistoryNextTurn;
    QAction *m_actionHistoryFirstTurn;
    QAction *m_actionHistoryLastTurn;
    QAction *m_actionHistoryReplayTurn;
    QAction *m_actionWindowsToolbar;

    static const char * m_windowName;

    /// Auxiliary windows
    //@{
    QAction *m_actionWindowsBag;
    AuxWindow *m_bagWindow;
    QAction *m_actionWindowsBoard;
    AuxWindow *m_boardWindow;
    QAction *m_actionWindowsHistory;
    AuxWindow *m_historyWindow;
    QAction *m_actionWindowsDicTools;
    AuxWindow *m_dicToolsWindow;
    //@}

    /// Label indicating the name of the current dictionary
    QLabel *m_dicNameLabel;

    /// Model for the coordinates of the word to play
    CoordModel m_coordModel;

    /// Save window state
    void writeSettings() const;
    /// Restore window state
    void readSettings();

    /// Helper function to easemenu creation
    QAction * addMenuAction(QMenu *menu, QString iText,
                            const QKeySequence &iShortcut,
                            QString iStatusTip, const char *iMember,
                            bool iCheckable = false, QIcon icon = QIcon());

    /// Create the menu bar and the actions
    void createMenu();

    /// Destroy the current game (if any) and the associated widgets
    void destroyCurrentGame();

};

#endif

