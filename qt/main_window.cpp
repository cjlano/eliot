/*****************************************************************************
 * Eliot
 * Copyright (C) 2008-2012 Olivier Teulière
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

#include "config.h"

//#include <iostream>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QDockWidget>
#include <QtGui/QCloseEvent>
#include <QtGui/QPrintDialog>
#include <QtGui/QPrinter>
#include <QtGui/QPainter>
#include <QtGui/QDesktopServices>
#include <QtCore/QSettings>
#include <QtCore/QUrl>

#include "main_window.h"
#include "dic.h"
#include "dic_exception.h"
#include "encoding.h"
#include "header.h"
#include "game_factory.h"
#include "game.h"
#include "public_game.h"
#include "player.h"
#include "history.h"
#include "turn.h"
#include "move.h"
#include "debug.h"
#include "new_game.h"
#include "prefs_dialog.h"
#include "bag_widget.h"
#include "board_widget.h"
#include "score_widget.h"
#include "player_widget.h"
#include "training_widget.h"
#include "history_widget.h"
#include "dic_tools_widget.h"
#include "dic_wizard.h"
#include "aux_window.h"
#include "qtcommon.h"

#include "round.h"
#include "coord.h"


INIT_LOGGER(qt, MainWindow);

const char *MainWindow::m_windowName = "MainWindow";

MainWindow::MainWindow(QWidget *iParent)
    : QMainWindow(iParent), m_dic(NULL), m_game(NULL),
    m_newGameDialog(NULL), m_prefsDialog(NULL),
    m_playersWidget(NULL), m_trainingWidget(NULL), m_scoresWidget(NULL),
    m_bagWindow(NULL), m_boardWindow(NULL),
    m_historyWindow(NULL), m_dicToolsWindow(NULL), m_dicNameLabel(NULL)
{
#ifdef DEBUG
    // Check that the string conversion routines are not buggy
    QtCommon::CheckConversions();
#endif

    LOG_DEBUG("Creating main window");
    m_ui.setupUi(this);
    createMenu();
    readSettings();

    // Initialize the random numbers generator
    // Note: This must be done _after_ creating the QMenuBar object,
    // because on Gnome QMenuBar calls gconftool2, which for some reason
    // calls srand() internally...
    // This could be disabled using QApplication::setDesktopSettingsAware(),
    // but we would lose the desktop integration...
    unsigned int val = time(NULL);
    srand(val);

    // Make it easier to reproduce bugs
    LOG_DEBUG("Rand seed: " << val);

    QObject::connect(this, SIGNAL(gameChangedNonConst(PublicGame*)),
                     this, SLOT(updateForGame(PublicGame*)));
    QObject::connect(this, SIGNAL(gameUpdated()),
                     this, SLOT(refresh()));

    // Status bar
    statusBar()->addWidget(new QLabel, 1);
    // First widget, not added yet
    m_lettersLabel = new QLabel;
    m_lettersLabel->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    // Second widget, not added yet
    m_turnLabel = new QLabel;
    m_turnLabel->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    // Third widget
    m_dicNameLabel = new QLabel;
    m_dicNameLabel->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    statusBar()->addPermanentWidget(m_dicNameLabel);
    QObject::connect(this, SIGNAL(dicChanged(const Dictionary*)),
                     this, SLOT(updateStatusBar(const Dictionary*)));

    // Board
    BoardWidget *boardWidget = new BoardWidget(m_coordModel);
    QObject::connect(this, SIGNAL(gameChanged(const PublicGame*)),
                     boardWidget, SLOT(setGame(const PublicGame*)));
    QObject::connect(this, SIGNAL(gameUpdated()),
                     boardWidget, SLOT(refresh()));

    QHBoxLayout *hlayout = new QHBoxLayout;
#if 0
    QDockWidget *dock = new QDockWidget;
    dock->setWidget(boardWidget);
    boardWidget->setWindowTitle(_q("Board"));

    hlayout->addWidget(dock);
#else
    hlayout->addWidget(boardWidget);
#endif

    m_ui.groupBoxTest->setLayout(hlayout);

#if 1
    // History
    HistoryTabWidget *historyTab = new HistoryTabWidget;
    QObject::connect(this, SIGNAL(gameChanged(const PublicGame*)),
                     historyTab, SLOT(setGame(const PublicGame*)));
    QObject::connect(this, SIGNAL(gameUpdated()),
                     historyTab, SLOT(refresh()));
    QObject::connect(historyTab, SIGNAL(requestDefinition(QString)),
                     this, SLOT(showDefinition(QString)));
    QHBoxLayout *hlayout2 = new QHBoxLayout;
    hlayout2->addWidget(historyTab);
    m_ui.groupBoxHistory->setLayout(hlayout2);
#else
    m_ui.groupBoxHistory->hide();
#endif

    // Hide the players group box
    m_ui.groupBoxPlayers->hide();

    emit gameChangedNonConst(NULL);
    emit gameChanged(NULL);

    // Load dictionary
    QSettings qs(ORGANIZATION, PACKAGE_NAME);
    QString dicPath = qs.value(PrefsDialog::kINTF_DIC_PATH, "").toString();
    if (dicPath != "")
    {
        LOG_INFO("Using dictionary " << lfq(dicPath));
        try
        {
            m_dic = new Dictionary(lfq(dicPath));
        }
        catch (DicException &e)
        {
            displayErrorMsg(_q("Cannot load dictionary '%1' indicated in the "
                               "preferences.\nReason: %2").arg(dicPath).arg(e.what()));
            return;
        }
    }
    emit dicChanged(m_dic);
}


MainWindow::~MainWindow()
{
    delete m_bagWindow;
    delete m_boardWindow;
    delete m_historyWindow;
    delete m_dicToolsWindow;
    delete m_game;
    delete m_dic;
}


void MainWindow::destroyCurrentGame()
{
    if (m_game == NULL)
        return;

    LOG_DEBUG("Destroying current game");

    // Some controls, like the board, can live when there is no game.
    // We only have to give them a NULL handler instead of the current one.
    emit gameChangedNonConst(NULL);
    emit gameChanged(NULL);

    m_ui.groupBoxPlayers->hide();

    delete m_game;
    m_game = NULL;

    LOG_DEBUG("Game destroyed");
}


void MainWindow::refresh()
{
    if (m_game != NULL)
    {
        const Bag &bag = m_game->getBag();
        m_lettersLabel->setText(_q("Consonants: %1 | Vowels: %2 | Jokers: %3")
                                .arg(bag.getNbConsonants())
                                .arg(bag.getNbVowels())
                                .arg(bag.in(Tile::Joker())));
        m_turnLabel->setText(_q("Turn %1/%2")
                             .arg(m_game->getCurrTurn())
                             .arg(m_game->getNbTurns()));
        bool isFirstTurn = m_game->isFirstTurn();
        bool isLastTurn = m_game->isLastTurn();
        m_actionHistoryFirstTurn->setEnabled(!isFirstTurn);
        m_actionHistoryPrevTurn->setEnabled(!isFirstTurn);
        m_actionHistoryNextTurn->setEnabled(!isLastTurn);
        m_actionHistoryLastTurn->setEnabled(!isLastTurn);
        m_actionHistoryReplayTurn->setEnabled(!isLastTurn);
        if (m_game->isFinished())
            displayInfoMsg(_q("End of the game"));
#ifdef DEBUG
        //m_game->printTurns();
#endif
    }
}


void MainWindow::linkTrainingAnd7P1()
{
    if (m_trainingWidget == NULL || m_dicToolsWindow == NULL)
        return;

    // Disconnect the training rack updates from the "Plus 1" tab of the
    // dictionary tools
    m_trainingWidget->disconnect(SIGNAL(rackUpdated(const QString&)));
    // Reconnect it only if needed
    if (m_dicToolsWindow != NULL)
    {
        QSettings qs(ORGANIZATION, PACKAGE_NAME);
        if (qs.value(PrefsDialog::kINTF_LINK_TRAINING_7P1, false).toBool())
        {
            QObject::connect(m_trainingWidget,
                             SIGNAL(rackUpdated(const QString&)),
                             &m_dicToolsWindow->getWidget(),
                             SLOT(setPlus1Rack(const QString&)));
        }
    }
}


void MainWindow::prefsUpdated()
{
    LOG_DEBUG("Preferences updated");
    // Refresh one signal/slot connection
    linkTrainingAnd7P1();

    // Refresh the default level for the Eliot player
    if (m_newGameDialog != NULL)
    {
        m_newGameDialog->refresh();
    }

    // Probably useless in most cases (currently only used for
    // the History alignment)
    emit gameUpdated();
}


void MainWindow::updateForGame(PublicGame *iGame)
{
    if (iGame == NULL)
    {
        m_actionGameSaveAs->setEnabled(false);
        m_actionGamePrint->setEnabled(false);
        m_actionHistoryFirstTurn->setEnabled(false);
        m_actionHistoryPrevTurn->setEnabled(false);
        m_actionHistoryNextTurn->setEnabled(false);
        m_actionHistoryLastTurn->setEnabled(false);
        m_actionHistoryReplayTurn->setEnabled(false);
        setWindowTitle(_q("No game") + " - Eliot");
        statusBar()->removeWidget(m_lettersLabel);
        statusBar()->removeWidget(m_turnLabel);

        // Destroy the players widget
        QtCommon::DestroyObject(m_playersWidget, this);
        m_playersWidget = NULL;

        // Destroy the training widget
        QtCommon::DestroyObject(m_trainingWidget, this);
        m_trainingWidget = NULL;

        // Destroy the scores widget
        QtCommon::DestroyObject(m_scoresWidget, this);
        m_scoresWidget = NULL;
    }
    else
    {
        m_actionGamePrint->setEnabled(true);
        m_actionGameSaveAs->setEnabled(true);
        statusBar()->addWidget(m_lettersLabel);
        m_lettersLabel->show();
        statusBar()->addWidget(m_turnLabel);
        m_turnLabel->show();

        if (iGame->getMode() == PublicGame::kTRAINING)
        {
            setWindowTitle(_q("Training mode") + " - Eliot");
            m_ui.groupBoxPlayers->setTitle(_q("Training"));

            // Training widget
            m_trainingWidget = new TrainingWidget(NULL, m_coordModel, iGame);
            m_ui.groupBoxPlayers->layout()->addWidget(m_trainingWidget);
            QObject::connect(m_trainingWidget, SIGNAL(gameUpdated()),
                             this, SIGNAL(gameUpdated()));
            QObject::connect(m_trainingWidget, SIGNAL(notifyInfo(QString)),
                             this, SLOT(displayInfoMsg(QString)));
            QObject::connect(m_trainingWidget, SIGNAL(notifyProblem(QString)),
                             this, SLOT(displayErrorMsg(QString)));
            QObject::connect(m_trainingWidget, SIGNAL(requestDefinition(QString)),
                             this, SLOT(showDefinition(QString)));
            QObject::connect(this, SIGNAL(gameUpdated()),
                             m_trainingWidget, SLOT(refresh()));
            // Connect with the dictionary tools only if needed
            linkTrainingAnd7P1();

            // Players score
            m_scoresWidget = new ScoreWidget(NULL, iGame);
            m_ui.groupBoxPlayers->layout()->addWidget(m_scoresWidget);
            QObject::connect(this, SIGNAL(gameUpdated()),
                             m_scoresWidget, SLOT(refresh()));
        }
        else
        {
            if (iGame->getMode() == PublicGame::kDUPLICATE)
                setWindowTitle(_q("Duplicate game") + " - Eliot");
            else
                setWindowTitle(_q("Free game") + " - Eliot");
            m_ui.groupBoxPlayers->setTitle(_q("Players"));

            // Players widget
            m_playersWidget = new PlayerTabWidget(m_coordModel, NULL);
            m_ui.groupBoxPlayers->layout()->addWidget(m_playersWidget);
            QObject::connect(m_playersWidget, SIGNAL(gameUpdated()),
                             this, SIGNAL(gameUpdated()));
            QObject::connect(m_playersWidget, SIGNAL(notifyInfo(QString)),
                             this, SLOT(displayInfoMsg(QString)));
            QObject::connect(m_playersWidget, SIGNAL(notifyProblem(QString)),
                             this, SLOT(displayErrorMsg(QString)));
            QObject::connect(m_playersWidget, SIGNAL(requestDefinition(QString)),
                             this, SLOT(showDefinition(QString)));
            QObject::connect(this, SIGNAL(gameUpdated()),
                             m_playersWidget, SLOT(refresh()));
            m_playersWidget->setGame(iGame);

            // Players score
            m_scoresWidget = new ScoreWidget(NULL, iGame);
            m_ui.groupBoxPlayers->layout()->addWidget(m_scoresWidget);
            QObject::connect(this, SIGNAL(gameUpdated()),
                             m_scoresWidget, SLOT(refresh()));
        }
    }
}


void MainWindow::updateStatusBar(const Dictionary *iDic)
{
    if (iDic == NULL)
        m_dicNameLabel->setText(_q("No dictionary"));
    else {
        QString dicName = qfw(m_dic->getHeader().getName());
        m_dicNameLabel->setText(_q("Dictionary: %1").arg(dicName));
        m_dicNameLabel->setToolTip("");
    }
}


void MainWindow::displayErrorMsg(QString iMsg, QString iContext)
{
    LOG_ERROR("Displayed error: " << lfq(iMsg));
    if (iContext == "")
        iContext = _q("Eliot - Error");

    QMessageBox::warning(this, iContext, iMsg);
}


void MainWindow::displayInfoMsg(QString iMsg)
{
    LOG_INFO("Displayed message: " << lfq(iMsg));
    statusBar()->showMessage(iMsg);
}


void MainWindow::showDefinition(QString iWord)
{
    QSettings qs(ORGANIZATION, PACKAGE_NAME);
    QString url = qs.value(PrefsDialog::kINTF_DEFINITIONS_SITE_URL,
                           PrefsDialog::kDEFAULT_DEF_SITE).toString();
    if (url == "")
    {
        displayErrorMsg(_q("No definitions site defined.\n"
                           "Please define one in the preferences."));
        return;
    }

    url = url.replace("%w", iWord.toLower());
    url = url.replace("%W", iWord.toUpper());
    bool res = QDesktopServices::openUrl(QUrl(url));
    if (!res)
    {
        LOG_ERROR("Could not open URL: " << lfq(url));
    }
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_game)
    {
        QString msg = _q("A game has been started.");
        if (!requestConfirmation(msg, _q("Do you really want to quit?")))
        {
            event->ignore();
            return;
        }
    }

    LOG_INFO("Exiting");

    // Make sure auxiliary windows don't survive after the main one
    if (m_bagWindow)
        m_bagWindow->close();
    if (m_boardWindow)
        m_boardWindow->close();
    if (m_historyWindow)
        m_historyWindow->close();
    if (m_dicToolsWindow)
        m_dicToolsWindow->close();
    writeSettings();
    event->accept();
}


void MainWindow::writeSettings() const
{
    QSettings settings(ORGANIZATION, PACKAGE_NAME);
    settings.beginGroup(m_windowName);
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    // Do not save this splitter when no game is present, because the players
    // group is hidden and the splitter is not in its normal place
    if (m_game)
        settings.setValue("splitterHoriz", m_ui.splitterHoriz->saveState());
    settings.setValue("splitterVert", m_ui.splitterVert->saveState());
    settings.endGroup();
}


void MainWindow::readSettings()
{
    QSettings settings(ORGANIZATION, PACKAGE_NAME);
    settings.beginGroup(m_windowName);
    QSize size = settings.value("size").toSize();
    if (size.isValid())
        resize(size);
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    m_ui.splitterHoriz->restoreState(settings.value("splitterHoriz").toByteArray());
    m_ui.splitterVert->restoreState(settings.value("splitterVert").toByteArray());
    settings.endGroup();
}


void MainWindow::changeDictionary(QString iFileName)
{
    if (!iFileName.isEmpty())
    {
        if (m_game)
        {
            QString msg = _q("Loading a dictionary will stop the current game.");
            if (!requestConfirmation(msg))
                return;
        }

        LOG_INFO("Loading new dictionary file: " << lfq(iFileName));

        destroyCurrentGame();

        try
        {
            Dictionary *dic = new Dictionary(lfq(iFileName));
            delete m_dic;
            m_dic = dic;
            emit dicChanged(m_dic);
            displayInfoMsg(_q("Loaded dictionary '%1'").arg(iFileName));

            // Save the location of the dictionary in the preferences
            QSettings qs(ORGANIZATION, PACKAGE_NAME);
            qs.setValue(PrefsDialog::kINTF_DIC_PATH, iFileName);
        }
        catch (std::exception &e)
        {
            displayErrorMsg(e.what());
        }
    }
}


bool MainWindow::requestConfirmation(QString msg, QString question)
{
    QMessageBox confirmationBox(QMessageBox::Question, _q("Eliot"), msg,
                                QMessageBox::Yes | QMessageBox::No, this);
    if (question != "")
        confirmationBox.setInformativeText(question);
    else
        confirmationBox.setInformativeText(_q("Do you want to continue?"));
    confirmationBox.setDefaultButton(QMessageBox::Yes);
    confirmationBox.setEscapeButton(QMessageBox::No);
    int res = confirmationBox.exec();
    return res == QMessageBox::Yes;
}


QAction * MainWindow::addMenuAction(QMenu *menu, QString iText,
                                    const QKeySequence &iShortcut,
                                    QString iStatusTip, const char *iMember,
                                    bool iCheckable, QIcon icon)
{
    QAction *action = new QAction(iText, this);
    action->setShortcut(iShortcut);
    action->setStatusTip(iStatusTip);
    action->setCheckable(iCheckable);
    action->setIcon(icon);
    QObject::connect(action, SIGNAL(triggered()), this, iMember);
    menu->addAction(action);
    return action;
}


void MainWindow::createMenu()
{
    // Decide whether to show the toolbar
    QSettings qs(ORGANIZATION, PACKAGE_NAME);
    bool showToolBar = qs.value(PrefsDialog::kINTF_SHOW_TOOLBAR, true).toBool();
    m_ui.toolBar->setVisible(showToolBar);

    QMenu *menuFile = new QMenu(m_ui.menubar);
    m_ui.menubar->addAction(menuFile->menuAction());
    menuFile->setTitle(_q("&Game"));
    addMenuAction(menuFile, _q("&New..."), _q("Ctrl+N"),
                  _q("Start a new game"), SLOT(onGameNew()));
    menuFile->addSeparator();
    addMenuAction(menuFile, _q("&Load..."), _q("Ctrl+O"),
                  _q("Load an existing game"), SLOT(onGameLoad()));
    m_actionGameSaveAs = addMenuAction(menuFile, _q("&Save as..."), _q("Ctrl+S"),
                  _q("Save the current game"), SLOT(onGameSaveAs()));
    menuFile->addSeparator();
    m_actionGamePrint = addMenuAction(menuFile, _q("&Print..."), _q("Ctrl+P"),
                  _q("Print the current game"), SLOT(onGamePrint()),
                  false, QIcon(":/images/printer.png"));
    menuFile->addSeparator();
    addMenuAction(menuFile, _q("&Quit"), _q("Ctrl+Q"),
                  _q("Quit Eliot"), SLOT(onGameQuit()),
                  false, QIcon(":/images/quit_16px.png"));

    QMenu *menuHistory = new QMenu(m_ui.menubar);
    m_ui.menubar->addAction(menuHistory->menuAction());
    menuHistory->setTitle(_q("&History"));
    m_actionHistoryFirstTurn = addMenuAction(menuHistory, _q("&First turn"), _q("Ctrl+Home"),
                  _q("Go to the first turn of the game"), SLOT(onHistoryFirstTurn()),
                  false, QIcon(":/images/go-first.png"));
    m_actionHistoryPrevTurn = addMenuAction(menuHistory, _q("&Previous turn"), _q("Ctrl+Left"),
                  _q("Go to the previous turn of the game"), SLOT(onHistoryPrevTurn()),
                  false, QIcon(":/images/go-previous.png"));
    m_actionHistoryNextTurn = addMenuAction(menuHistory, _q("&Next turn"), _q("Ctrl+Right"),
                  _q("Go to the next turn of the game"), SLOT(onHistoryNextTurn()),
                  false, QIcon(":/images/go-next.png"));
    m_actionHistoryLastTurn = addMenuAction(menuHistory, _q("&Last turn"), _q("Ctrl+End"),
                  _q("Go to the last turn of the game"), SLOT(onHistoryLastTurn()),
                  false, QIcon(":/images/go-last.png"));
    m_actionHistoryReplayTurn = addMenuAction(menuHistory, _q("&Replay turn"), _q("Ctrl+R"),
                  _q("Play the game from the current position, "
                     "replacing what was really played"), SLOT(onHistoryReplayTurn()),
                  false, QIcon(":/images/go-jump.png"));
    // Add actions to the toolbar
    m_ui.toolBar->addAction(m_actionHistoryFirstTurn);
    m_ui.toolBar->addAction(m_actionHistoryPrevTurn);
    m_ui.toolBar->addAction(m_actionHistoryNextTurn);
    m_ui.toolBar->addAction(m_actionHistoryLastTurn);
    m_ui.toolBar->addAction(m_actionHistoryReplayTurn);

    QMenu *menuSettings = new QMenu(m_ui.menubar);
    m_ui.menubar->addAction(menuSettings->menuAction());
    menuSettings->setTitle(_q("&Settings"));
    addMenuAction(menuSettings, _q("&Choose dictionary..."), _q("Ctrl+C"),
                  _q("Select a new dictionary"), SLOT(onSettingsChooseDic()));
    addMenuAction(menuSettings, _q("Create &new dictionary..."), QString(""),
                  _q("Start the wizard for creating a new dictionary "
                     "from an existing word list"), SLOT(onSettingsCreateDic()));
    addMenuAction(menuSettings, _q("&Preferences..."), _q("Ctrl+F"),
                  _q("Edit the preferences"), SLOT(onSettingsPreferences()),
                  false, QIcon(":/images/preferences.png"));

    QMenu *menuWindows = new QMenu(m_ui.menubar);
    m_ui.menubar->addAction(menuWindows->menuAction());
    menuWindows->setTitle(_q("&Windows"));
    m_actionWindowsToolbar = addMenuAction(menuWindows, _q("&Toolbar"), _q("Ctrl+T"),
                  _q("Show/hide the toolbar"), SLOT(onWindowsToolbar()), true);
    m_actionWindowsToolbar->setChecked(showToolBar);
    m_actionWindowsBag = addMenuAction(menuWindows, _q("&Bag"), _q("Ctrl+B"),
                  _q("Show/hide the remaining tiles in the bag"), SLOT(onWindowsBag()), true);
    m_actionWindowsBoard = addMenuAction(menuWindows, _q("&External board"), _q("Ctrl+E"),
                  _q("Show/hide the external board"), SLOT(onWindowsBoard()), true);
    m_actionWindowsHistory = addMenuAction(menuWindows, _q("&History"), _q("Ctrl+H"),
                  _q("Show/hide the game history"), SLOT(onWindowsHistory()),
                  true, QIcon(":/images/playlist_16px.png"));
    m_actionWindowsDicTools = addMenuAction(menuWindows, _q("&Dictionary tools"), _q("Ctrl+D"),
                  _q("Show/hide the dictionary tools"), SLOT(onWindowsDicTools()), true);

    QMenu *menuHelp = new QMenu(m_ui.menubar);
    m_ui.menubar->addAction(menuHelp->menuAction());
    menuHelp->setTitle(_q("Hel&p"));
    addMenuAction(menuHelp, _q("&About..."), QString(""),
                  _q("About Eliot"), SLOT(onHelpAbout()),
                  false, QIcon(":/images/info_16px.png"));
}


void MainWindow::onGameNew()
{
    LOG_DEBUG("Starting a new game (unconfirmed)");

    if (m_dic == NULL)
    {
        displayErrorMsg(_q("You have to select a dictionary (.dawg file) "
                           "before starting a game. This can be done in the "
                           "\"Settings\" menu."
                           "\n\nYou can download dictionary files on Eliot web site."));
        return;
    }

    if (m_newGameDialog == NULL)
        m_newGameDialog = new NewGame(this);

    int res = m_newGameDialog->exec();
    if (res == QDialog::Rejected)
        return;

    if (m_game)
    {
        QString msg = _q("Starting a new game will stop the current one.");
        if (!requestConfirmation(msg))
            return;
    }

    // Destroy the game and the associated controls
    destroyCurrentGame();

    // Create a new game
    m_game = m_newGameDialog->createGame(*m_dic);
    if (m_game == NULL)
        return;

    m_ui.groupBoxPlayers->show();

    displayInfoMsg(_q("Game started"));
    m_game->start();
    emit gameChangedNonConst(m_game);
    emit gameChanged(m_game);
    emit gameUpdated();
}


void MainWindow::onGameLoad()
{
    if (m_dic == NULL)
    {
        displayErrorMsg(_q("You have to select a dictionary first!"));
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this, _q("Load a game"));
    if (fileName != "")
    {
        try
        {
            PublicGame *tmpGame = PublicGame::load(lfq(fileName), *m_dic);
            destroyCurrentGame();
            m_game = tmpGame;
        }
        catch (std::exception &e)
        {
            displayErrorMsg(_q("Error while loading the game:\n") + e.what());
            return;
        }
        m_ui.groupBoxPlayers->show();
        emit gameChangedNonConst(m_game);
        emit gameChanged(m_game);
        emit gameUpdated();
        displayInfoMsg(_q("Game loaded"));
    }
}


void MainWindow::onGameSaveAs()
{
    if (m_game == NULL)
        return;

    QString fileName = QFileDialog::getSaveFileName(this, _q("Save a game"));
    if (fileName != "")
    {
        try
        {
            m_game->save(lfq(fileName));
            displayInfoMsg(_q("Game saved"));
        }
        catch (std::exception &e)
        {
            displayErrorMsg(_q("Error saving game: %1").arg(e.what()));
        }
    }
}


void MainWindow::onGamePrint()
{
    LOG_DEBUG("Printing game (unconfirmed)");
    if (m_game == NULL)
        return;

    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() == QDialog::Accepted)
    {
        LOG_INFO("Printing game");

        QPainter painter(&printer);
        const History &history = m_game->getHistory();

        // Printing parameters (XXX: these could be configurable by the users)
        // Number of pixels virtually present on the page width. The bigger
        // this number, the smaller the print result
        static const int TOTAL_WIDTH = 700;
        // Distance between 2 horizontal lines
        static const int LINE_HEIGHT = 16;
        // Font size, in pixels
        static const int FONT_SIZE = 10;
        // Width of the pen used to draw the grid lines
        static const int PEN_WIDTH = 1;
        // Offset of the text from the previous vertical line, in pixels
        static const int TEXT_OFFSET = 10;
        // Indicate whether the rack and the solution should be aligned
        static const bool SHOULD_ALIGN = false;
        // Columns widths
        static const int colWidths[] = { 30, 120, 120, 35, 35 };
        // Columns titles
        static const char *colTitles[] = { _("N."), _("RACK"), _("SOLUTION"), _("REF"), _("PTS") };

        static const unsigned int nbCols = sizeof(colWidths) / sizeof(int);
        const unsigned int nbRows = history.getSize() + (SHOULD_ALIGN ? 1 : 2);

        double scale = printer.pageRect().width() / double(TOTAL_WIDTH);
        painter.scale(scale, scale);

        QPen pen(painter.pen());
        pen.setWidth(PEN_WIDTH);
        painter.setPen(pen);

        QFont font;
        font.setPixelSize(FONT_SIZE);
        painter.setFont(font);

        int maxRight = 0;
        for (unsigned int i = 0; i < nbCols; ++i)
            maxRight += colWidths[i];
        int maxBottom = LINE_HEIGHT * (nbRows + 1);

        // Draw the horizontal lines
        for (unsigned int i = 0; i <= nbRows + 1; ++i)
            painter.drawLine(0, LINE_HEIGHT * i, maxRight, LINE_HEIGHT * i);

        // Draw the vertical lines
        painter.drawLine(0, 0, 0, maxBottom);
        int curWidth = 0;
        for (unsigned int i = 0; i < nbCols; ++i)
        {
            curWidth += colWidths[i];
            painter.drawLine(curWidth, 0, curWidth, maxBottom);
        }

        // Draw the titles
        QFontMetrics fm = painter.fontMetrics();
        int textHeight = fm.boundingRect('A').height();
        curWidth = 0;
        int curHeight = (LINE_HEIGHT + textHeight + 1) / 2;
        for (unsigned int i = 0; i < nbCols; ++i)
        {
            int textWidth = fm.width(colTitles[i]);
            painter.drawText(curWidth + (colWidths[i] - textWidth) / 2,
                             curHeight,  colTitles[i]);
            curWidth += colWidths[i];
        }

        // Draw the history of the game
        int score = 0;
        int nextHeight;
        if (SHOULD_ALIGN)
            nextHeight = curHeight;
        else
            nextHeight = curHeight + LINE_HEIGHT;
        for (unsigned int i = 0; i < history.getSize(); ++i)
        {
            const Turn &t = history.getTurn(i);
            const Move &m = t.getMove();

            curWidth = TEXT_OFFSET;
            curHeight += LINE_HEIGHT;
            nextHeight += LINE_HEIGHT;

            // Turn number
            painter.drawText(curWidth, curHeight, QString("%1").arg(i + 1));
            curWidth += colWidths[0];

            // Rack
            painter.drawText(curWidth, curHeight,
                             qfw(t.getPlayedRack().toString()));
            curWidth += colWidths[1];

            // Word and coordinates
            if (m.getType() == Move::VALID_ROUND)
            {
                const Round &r = m.getRound();
                painter.drawText(curWidth, nextHeight, qfw(r.getWord()));
                curWidth += colWidths[2];
                painter.drawText(curWidth, nextHeight,
                                 qfw(r.getCoord().toString()));
                curWidth += colWidths[3];
            }
            else if (m.getType() == Move::INVALID_WORD)
            {
                painter.drawText(curWidth, nextHeight,
                                 "<" + qfw(m.getBadWord()) + ">");
                curWidth += colWidths[2];
                painter.drawText(curWidth, nextHeight, qfw(m.getBadCoord()));
                curWidth += colWidths[3];
            }
            else if (m.getType() == Move::NO_MOVE)
            {
                painter.drawText(curWidth, nextHeight, _q("(NO MOVE)"));
                curWidth += colWidths[2];
                curWidth += colWidths[3];
            }
            else if (m.getType() == Move::PASS)
            {
                painter.drawText(curWidth, nextHeight, _q("(PASS)"));
                curWidth += colWidths[2];
                curWidth += colWidths[3];
            }
            else
            {
                painter.drawText(curWidth, nextHeight,
                                 "[-" + qfw(m.getChangedLetters()) + "]");
                curWidth += colWidths[2];
                curWidth += colWidths[3];
            }

            // Score
            painter.drawText(curWidth, nextHeight,
                             QString("%1").arg(m.getScore()));
            score += m.getScore();
        }

        // Total score
        nextHeight += LINE_HEIGHT;
        painter.drawText(curWidth, nextHeight, QString("%1").arg(score));

        LOG_INFO("Game printed");
    }
}


void MainWindow::onGameQuit()
{
    close();
}


void MainWindow::onSettingsPreferences()
{
    if (m_prefsDialog == NULL)
    {
        m_prefsDialog = new PrefsDialog(this);
        QObject::connect(m_prefsDialog, SIGNAL(prefsUpdated()),
                         this, SLOT(prefsUpdated()));
    }
    m_prefsDialog->exec();
}


void MainWindow::onSettingsChooseDic()
{
    QString fileName =
        QFileDialog::getOpenFileName(this, _q("Choose a dictionary"), "", "*.dawg");
    changeDictionary(fileName);
}


void MainWindow::onSettingsCreateDic()
{
    DicWizard *wizard = new DicWizard(this);
    wizard->setWindowTitle(_q("Dictionary creation wizard"));
    connect(wizard, SIGNAL(infoMsg(QString)),
            this, SLOT(displayInfoMsg(QString)));
    connect(wizard, SIGNAL(loadDictionary(QString)),
            this, SLOT(changeDictionary(QString)));
    wizard->show();
}


void MainWindow::onWindowsToolbar()
{
    if (m_ui.toolBar->isVisible())
        m_ui.toolBar->hide();
    else
        m_ui.toolBar->show();
    QSettings qs(ORGANIZATION, PACKAGE_NAME);
    qs.setValue(PrefsDialog::kINTF_SHOW_TOOLBAR, m_ui.toolBar->isVisible());
}


void MainWindow::onWindowsBag()
{
    if (m_bagWindow == NULL)
    {
        // Create the window
        BagWidget *bag = new BagWidget(NULL);
        bag->setGame(m_game);
        m_bagWindow = new AuxWindow(*bag, _q("Bag"), "BagWindow",
                                    m_actionWindowsBag);
        QObject::connect(this, SIGNAL(gameChanged(const PublicGame*)),
                         bag, SLOT(setGame(const PublicGame*)));
        QObject::connect(this, SIGNAL(gameUpdated()),
                         bag, SLOT(refresh()));
    }
    m_bagWindow->toggleVisibility();
}


void MainWindow::onWindowsBoard()
{
    if (m_boardWindow == NULL)
    {
        // Create the window
        BoardWidget *board = new BoardWidget(m_coordModel, NULL);
        board->setGame(m_game);
        m_boardWindow = new AuxWindow(*board, _q("Board"), "BoardWindow",
                                      m_actionWindowsBoard);
        QObject::connect(this, SIGNAL(gameChanged(const PublicGame*)),
                         board, SLOT(setGame(const PublicGame*)));
        QObject::connect(this, SIGNAL(gameUpdated()),
                         board, SLOT(refresh()));
    }
    m_boardWindow->toggleVisibility();
}


void MainWindow::onWindowsHistory()
{
    if (m_historyWindow == NULL)
    {
        // Create the window
        HistoryTabWidget *history = new HistoryTabWidget(NULL);
        history->setGame(m_game);
        m_historyWindow = new AuxWindow(*history, _q("History"), "HistoryWindow",
                                        m_actionWindowsHistory);
        QObject::connect(this, SIGNAL(gameChanged(const PublicGame*)),
                         history, SLOT(setGame(const PublicGame*)));
        QObject::connect(this, SIGNAL(gameUpdated()),
                         history, SLOT(refresh()));
    }
    m_historyWindow->toggleVisibility();
}


void MainWindow::onWindowsDicTools()
{
    if (m_dicToolsWindow == NULL)
    {
        // Create the window
        DicToolsWidget *dicTools = new DicToolsWidget(NULL);
        m_dicToolsWindow = new AuxWindow(*dicTools, _q("Dictionary tools"), "DicTools",
                                    m_actionWindowsDicTools);
        QObject::connect(this, SIGNAL(dicChanged(const Dictionary*)),
                         dicTools, SLOT(setDic(const Dictionary*)));
        QObject::connect(dicTools, SIGNAL(requestDefinition(QString)),
                         this, SLOT(showDefinition(QString)));
        // Link the training rack with the "Plus 1" one
        linkTrainingAnd7P1();
        // Fake a dictionary selection
        dicTools->setDic(m_dic);
        dicTools->setFocus();
    }
    m_dicToolsWindow->toggleVisibility();
}


void MainWindow::onHelpAbout()
{
    QString msg;
    msg.sprintf("Eliot %s\n\n", VERSION);
    msg += _q( \
        "Copyright (C) 1999-2011 - Antoine Fraboulet & Olivier Teuliere\n\n" \
        "This program is free software; you can redistribute it and/or " \
        "modify it under the terms of the GNU General Public License as " \
        "published by the Free Software Foundation; either version 2 of " \
        "the License, or (at your option) any later version.");
    msg += "\n\n";
    msg += _q("Web site: http://www.nongnu.org/eliot/en/");
    // QMessageBox::about() doesn't add the nice information icon, so we create
    // the box manually (not much work...)
    QMessageBox aboutBox(QMessageBox::Information, _q("About Eliot"),
                         msg, QMessageBox::Ok, this);
    aboutBox.exec();
}


void MainWindow::onHistoryFirstTurn()
{
    if (m_game == NULL)
        return;

    m_game->firstTurn();
    emit gameUpdated();
    unsigned int currTurn = m_game->getCurrTurn();
    unsigned int nbTurns = m_game->getNbTurns();
    displayInfoMsg(_q("Turn %1/%2").arg(currTurn).arg(nbTurns));
}


void MainWindow::onHistoryPrevTurn()
{
    if (m_game == NULL)
        return;

    m_game->prevTurn();
    emit gameUpdated();
    unsigned int currTurn = m_game->getCurrTurn();
    unsigned int nbTurns = m_game->getNbTurns();
    displayInfoMsg(_q("Turn %1/%2").arg(currTurn).arg(nbTurns));
}


void MainWindow::onHistoryNextTurn()
{
    if (m_game == NULL)
        return;

    m_game->nextTurn();
    emit gameUpdated();
    unsigned int currTurn = m_game->getCurrTurn();
    unsigned int nbTurns = m_game->getNbTurns();
    displayInfoMsg(_q("Turn %1/%2").arg(currTurn).arg(nbTurns));
}


void MainWindow::onHistoryLastTurn()
{
    if (m_game == NULL)
        return;

    m_game->lastTurn();
    emit gameUpdated();
    unsigned int currTurn = m_game->getCurrTurn();
    unsigned int nbTurns = m_game->getNbTurns();
    displayInfoMsg(_q("Turn %1/%2").arg(currTurn).arg(nbTurns));
}


void MainWindow::onHistoryReplayTurn()
{
    if (m_game == NULL)
        return;

    QSettings settings(ORGANIZATION, PACKAGE_NAME);
    bool warn = settings.value(PrefsDialog::kINTF_WARN_REPLAY_TURN, true).toBool();
    if (warn) {
        // Ask for a confirmation, because this may lead to data loss
        QString msg = _q("Replaying this turn will modify the game history "
                         "by deleting the turns after the displayed one (i.e. "
                         "turns \"in the future\").");
        if (!requestConfirmation(msg))
            return;
    }

    unsigned int currTurn = m_game->getCurrTurn();
    m_game->clearFuture();
    emit gameUpdated();
    displayInfoMsg(_q("Replaying from turn %1").arg(currTurn));
}

