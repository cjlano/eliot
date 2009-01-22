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

#include "config.h"

#include <iostream>
#include <fstream>
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QDockWidget>
#include <QtGui/QCloseEvent>
#include <QtGui/QPrintDialog>
#include <QtGui/QPrinter>
#include <QtGui/QPainter>
#include <QtCore/QSettings>

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
#include "history_widget.h"
#include "dic_tools_widget.h"
#include "aux_window.h"
#include "qtcommon.h"

#include "round.h"
#include "coord.h"


const char *MainWindow::m_windowName = "MainWindow";

MainWindow::MainWindow(QWidget *iParent)
    : QMainWindow(iParent), m_dic(NULL), m_game(NULL), m_newGameDialog(NULL),
    m_prefsDialog(NULL), m_bagWindow(NULL), m_boardWindow(NULL),
    m_historyWindow(NULL), m_dicToolsWindow(NULL), m_dicNameLabel(NULL)
{
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
#ifdef DEBUG
    // Make it easier to reproduce bugs
    cout << "Rand seed: " << val << endl;
#endif

    QObject::connect(this, SIGNAL(gameChanged(const PublicGame*)),
                     this, SLOT(updateForGame(const PublicGame*)));
    QObject::connect(this, SIGNAL(gameUpdated()),
                     this, SLOT(refresh()));
    refresh();

    // Status bar
    statusBar()->addWidget(new QLabel, 1);
    m_dicNameLabel = new QLabel;
    m_dicNameLabel->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    statusBar()->addPermanentWidget(m_dicNameLabel);
    QObject::connect(this, SIGNAL(dicChanged(const Dictionary*)),
                     this, SLOT(updateStatusBar(const Dictionary*)));

    // Board
    BoardWidget *boardWidget = new BoardWidget;
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
    QHBoxLayout *hlayout2 = new QHBoxLayout;
    hlayout2->addWidget(historyTab);
    m_ui.groupBoxHistory->setLayout(hlayout2);
#else
    m_ui.groupBoxHistory->hide();
#endif

    // Players racks
    m_ui.groupBoxPlayers->hide();
    m_playersWidget = new PlayerTabWidget(NULL);
    m_ui.groupBoxPlayers->layout()->addWidget(m_playersWidget);
    QObject::connect(this, SIGNAL(gameChangedNonConst(PublicGame*)),
                     m_playersWidget, SLOT(setGame(PublicGame*)));
    QObject::connect(this, SIGNAL(gameUpdated()), m_playersWidget, SLOT(refresh()));

    QObject::connect(m_playersWidget, SIGNAL(gameUpdated()), this, SIGNAL(gameUpdated()));
    QObject::connect(m_playersWidget, SIGNAL(notifyProblem(QString)),
                     this, SLOT(displayErrorMsg(QString)));
    QObject::connect(m_playersWidget, SIGNAL(notifyInfo(QString)),
                     this, SLOT(displayInfoMsg(QString)));

    // Players score
    ScoreWidget *scores = new ScoreWidget;
    QObject::connect(this, SIGNAL(gameChanged(const PublicGame*)),
                     scores, SLOT(setGame(const PublicGame*)));
    QObject::connect(this, SIGNAL(gameUpdated()),
                     scores, SLOT(refresh()));
    m_ui.groupBoxPlayers->layout()->addWidget(scores);

    emit gameChangedNonConst(NULL);
    emit gameChanged(NULL);

    // Load dictionary
    QSettings qs(ORGANIZATION, PACKAGE_NAME);
    QString dicPath = qs.value(PrefsDialog::kINTF_DIC_PATH, "").toString();
    if (dicPath != "")
    {
        try
        {
            m_dic = new Dictionary(qtl(dicPath));
        }
        catch (DicException &e)
        {
            displayErrorMsg(_q("Cannot load dictionary '%1' indicated in the "
                               "preferences.\nReason: %2").arg(dicPath).arg(e.what()));
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

    // Some controls, like the board, can live when there is no game.
    // We only have to give them a NULL handler instead of the current one.
    emit gameChangedNonConst(NULL);
    emit gameChanged(NULL);

    m_ui.groupBoxPlayers->hide();

    delete m_game;
    m_game = NULL;
}


void MainWindow::refresh()
{
    if (m_game != NULL)
    {
        bool isFirstTurn = m_game->isFirstTurn();
        bool isLastTurn = m_game->isLastTurn();
        m_actionHistoryFirstTurn->setEnabled(!isFirstTurn);
        m_actionHistoryPrevTurn->setEnabled(!isFirstTurn);
        m_actionHistoryNextTurn->setEnabled(!isLastTurn);
        m_actionHistoryLastTurn->setEnabled(!isLastTurn);
        m_actionHistoryReplayTurn->setEnabled(!isLastTurn);
#ifdef DEBUG
        cout << endl << endl;
        m_game->printTurns();
#endif
    }
}


void MainWindow::prefsUpdated()
{
    // Disconnect the training rack updates from the "Plus 1" tab of the
    // dictionary tools
    m_playersWidget->disconnect(SIGNAL(trainingRackUpdated(const QString&)));
    // Reconnect it only if needed
    if (m_dicToolsWindow != NULL)
    {
        QSettings qs(ORGANIZATION, PACKAGE_NAME);
        if (qs.value(PrefsDialog::kINTF_LINK_TRAINING_7P1, false).toBool())
        {
            QObject::connect(m_playersWidget,
                             SIGNAL(trainingRackUpdated(const QString&)),
                             &m_dicToolsWindow->getWidget(),
                             SLOT(setPlus1Rack(const QString&)));
        }
    }

    // Probably useless in most cases (currently only used for
    // the History alignment)
    emit gameUpdated();
}


void MainWindow::updateForGame(const PublicGame *iGame)
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
    }
    else
    {
        m_actionGamePrint->setEnabled(true);
        if (iGame->getMode() == PublicGame::kTRAINING)
        {
            m_actionGameSaveAs->setEnabled(true);
            setWindowTitle(_q("Training mode") + " - Eliot");
        }
        else if (iGame->getMode() == PublicGame::kDUPLICATE)
        {
            m_actionGameSaveAs->setEnabled(false);
            setWindowTitle(_q("Duplicate game") + " - Eliot");
        }
        else
        {
            m_actionGameSaveAs->setEnabled(false);
            setWindowTitle(_q("Free game") + " - Eliot");
        }
    }
}


void MainWindow::updateStatusBar(const Dictionary *iDic)
{
    if (iDic == NULL)
        m_dicNameLabel->setText("No dictionary");
    else {
        QString dicName = qfw(m_dic->getHeader().getName());
        m_dicNameLabel->setText(_q("Dictionary: %1").arg(dicName));
        m_dicNameLabel->setToolTip("");
    }
}


void MainWindow::displayErrorMsg(QString iMsg, QString iContext)
{
    if (iContext == "")
        iContext = _q("Eliot - Error");

    QMessageBox::warning(this, iContext, iMsg);
}


void MainWindow::displayInfoMsg(QString iMsg)
{
    statusBar()->showMessage(iMsg);
}


void MainWindow::closeEvent(QCloseEvent *event)
{
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
                  _q("Quit Eliot"), SLOT(close()),
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
    menuHelp->setTitle(_q("&Help"));
    addMenuAction(menuHelp, _q("&About..."), QString(""),
                  _q("About Eliot"), SLOT(onHelpAbout()),
                  false, QIcon(":/images/info_16px.png"));
}


void MainWindow::onGameNew()
{
    if (m_dic == NULL)
    {
        displayErrorMsg(_q("You have to select a dictionary first!"));
        return;
    }

    if (m_newGameDialog == NULL)
        m_newGameDialog = new NewGame(this);

    int res = m_newGameDialog->exec();
    if (res == QDialog::Rejected)
        return;

    // Destroy the game and the associated controls
    destroyCurrentGame();

    // Create a new game
    m_game = m_newGameDialog->createGame(*m_dic);
    if (m_game == NULL)
        return;

    m_ui.groupBoxPlayers->show();

    m_game->start();
    emit gameChangedNonConst(m_game);
    emit gameChanged(m_game);
    emit gameUpdated();
    displayInfoMsg(_q("Game started"));
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
        destroyCurrentGame();
        Game *tmpGame = GameFactory::Instance()->load(qtl(fileName), *m_dic);
        if (tmpGame == NULL)
        {
            displayErrorMsg(_q("Error while loading the game"));
            return;
        }
        m_game = new PublicGame(*tmpGame);
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
        ofstream fout(qtl(fileName));
        m_game->save(fout);
        displayInfoMsg(_q("Game saved"));
    }
}


void MainWindow::onGamePrint()
{
    if (m_game == NULL)
        return;

    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() == QDialog::Accepted)
    {
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
    }
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
    if (m_game)
    {
        QString msg = _q("Loading a dictionary will stop the current game.");
        QMessageBox confirmationBox(QMessageBox::Question, _q("Eliot"), msg,
                                    QMessageBox::Yes | QMessageBox::No, this);
        confirmationBox.setInformativeText(_q("Do you want to continue?"));
        confirmationBox.setDefaultButton(QMessageBox::Yes);
        confirmationBox.setEscapeButton(QMessageBox::No);
        int res = confirmationBox.exec();
        if (res == QMessageBox::No)
            return;
    }

    QString fileName =
        QFileDialog::getOpenFileName(this, _q("Choose a dictionary"), "", "*.dawg");
    if (!fileName.isEmpty())
    {
        destroyCurrentGame();

        try
        {
            Dictionary *dic = new Dictionary(qtl(fileName));
            delete m_dic;
            m_dic = dic;
            emit dicChanged(m_dic);
            displayInfoMsg(QString("Loaded dictionary '%1'").arg(fileName));

            // Save the location of the dictionary in the preferences
            QSettings qs(ORGANIZATION, PACKAGE_NAME);
            qs.setValue(PrefsDialog::kINTF_DIC_PATH, fileName);
        }
        catch (std::exception &e)
        {
            displayErrorMsg(e.what());
        }
    }
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
        BoardWidget *board = new BoardWidget(NULL);
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
        // Link the training rack with the "Plus 1" one
        QSettings qs(ORGANIZATION, PACKAGE_NAME);
        if (qs.value(PrefsDialog::kINTF_LINK_TRAINING_7P1, false).toBool())
        {
            QObject::connect(m_playersWidget,
                             SIGNAL(trainingRackUpdated(const QString&)),
                             dicTools, SLOT(setPlus1Rack(const QString&)));
        }
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
        "Copyright (C) 1999-2008 - Antoine Fraboulet & Olivier Teuliere\n\n" \
        "This program is free software; you can redistribute it and/or " \
        "modify it under the terms of the GNU General Public License as " \
        "published by the Free Software Foundation; either version 2 of " \
        "the License, or (at your option) any later version.");
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
        QMessageBox confirmationBox(QMessageBox::Question, _q("Eliot"), msg,
                                    QMessageBox::Yes | QMessageBox::No, this);
        confirmationBox.setInformativeText(_q("Do you want to continue?"));
        confirmationBox.setDefaultButton(QMessageBox::Yes);
        confirmationBox.setEscapeButton(QMessageBox::No);
        int ret = confirmationBox.exec();
        if (ret != QMessageBox::Yes)
            return;
    }

    unsigned int currTurn = m_game->getCurrTurn();
    m_game->clearFuture();
    emit gameUpdated();
    displayInfoMsg(_q("Replaying from turn %1").arg(currTurn));
}

