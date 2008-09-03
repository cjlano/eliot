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
#include "encoding.h"
#include "header.h"
#include "game_factory.h"
#include "game.h"
#include "freegame.h"
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
#include "training_widget.h"
#include "aux_window.h"
#include "qtcommon.h"

#include "round.h"
#include "coord.h"


MainWindow::MainWindow(QWidget *iParent)
    : QMainWindow(iParent), m_dic(NULL), m_game(NULL), m_newGameDialog(NULL),
    m_prefsDialog(NULL), m_bagWindow(NULL), m_boardWindow(NULL),
    m_historyWindow(NULL), m_dicToolsWindow(NULL), m_dicNameLabel(NULL)
{
    m_ui.setupUi(this);
    QObject::connect(this, SIGNAL(gameChanged(const Game*)),
                     this, SLOT(updateForGame(const Game*)));

    // Status bar
    statusBar()->addWidget(new QLabel, 1);
    m_dicNameLabel = new QLabel;
    m_dicNameLabel->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    statusBar()->addPermanentWidget(m_dicNameLabel);
    QObject::connect(this, SIGNAL(dicChanged(const Dictionary*)),
                     this, SLOT(updateStatusBar(const Dictionary*)));

    // Board
    BoardWidget *boardWidget = new BoardWidget;
    QObject::connect(this, SIGNAL(gameChanged(const Game*)),
                     boardWidget, SLOT(setGame(const Game*)));
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

    // History
    HistoryTabWidget *historyTab = new HistoryTabWidget;
    QObject::connect(this, SIGNAL(gameChanged(const Game*)),
                     historyTab, SLOT(setGame(const Game*)));
    QObject::connect(this, SIGNAL(gameUpdated()),
                     historyTab, SLOT(refresh()));
    QHBoxLayout *hlayout2 = new QHBoxLayout;
    hlayout2->addWidget(historyTab);
    m_ui.groupBoxHistory->setLayout(hlayout2);

    // Players racks
    m_ui.groupBoxPlayers->hide();
    PlayerTabWidget *players = new PlayerTabWidget(NULL);
    m_ui.groupBoxPlayers->layout()->addWidget(players);
    QObject::connect(this, SIGNAL(gameChangedNonConst(Game*)),
                     players, SLOT(setGame(Game*)));
    QObject::connect(this, SIGNAL(gameUpdated()), players, SLOT(refresh()));

    QObject::connect(players, SIGNAL(gameUpdated()), this, SIGNAL(gameUpdated()));
    QObject::connect(players, SIGNAL(notifyProblem(QString)),
                     this, SLOT(displayErrorMsg(QString)));

    // Players score
    ScoreWidget *scores = new ScoreWidget;
    QObject::connect(this, SIGNAL(gameChanged(const Game*)),
                     scores, SLOT(setGame(const Game*)));
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
        catch (...)
        {
            displayErrorMsg(_q("Cannot load dictionary '%1' indicated in the preferences").arg(dicPath));
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


void MainWindow::updateForGame(const Game *iGame)
{
    if (iGame == NULL)
    {
        m_ui.action_GameSaveAs->setEnabled(false);
        m_ui.action_GamePrint->setEnabled(false);
        setWindowTitle(_q("No game") + " - Eliot");
    }
    else
    {
        m_ui.action_GameSaveAs->setEnabled(true);
        m_ui.action_GamePrint->setEnabled(true);
        if (iGame->getMode() == Game::kTRAINING)
        {
            setWindowTitle(_q("Training mode") + " - Eliot");
        }
        else if (iGame->getMode() == Game::kDUPLICATE)
        {
            setWindowTitle(_q("Duplicate game") + " - Eliot");
        }
        else
        {
            setWindowTitle(_q("Free game") + " - Eliot");
        }
    }
}


void MainWindow::updateStatusBar(const Dictionary *iDic)
{
    if (iDic == NULL)
        m_dicNameLabel->setText("No dictionary");
    else
        m_dicNameLabel->setText(qfw(m_dic->getHeader().getName()));
}


void MainWindow::displayErrorMsg(QString iMsg, QString iContext)
{
    if (iContext == "")
        iContext = PACKAGE_NAME;

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
    event->accept();
}


void MainWindow::on_action_GameNew_triggered()
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


void MainWindow::on_action_GameLoad_triggered()
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
        m_game = GameFactory::Instance()->load(qtl(fileName), *m_dic);
        if (m_game == NULL)
        {
            displayErrorMsg(_q("Error while loading the game"));
            return;
        }
        m_ui.groupBoxPlayers->show();
        emit gameChangedNonConst(m_game);
        emit gameChanged(m_game);
        emit gameUpdated();
        displayInfoMsg(_q("Game loaded"));
    }
}


void MainWindow::on_action_GameSaveAs_triggered()
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


void MainWindow::on_action_GamePrint_triggered()
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


void MainWindow::on_action_SettingsPreferences_triggered()
{
    if (m_prefsDialog == NULL)
    {
        m_prefsDialog = new PrefsDialog(this);
        QObject::connect(m_prefsDialog, SIGNAL(gameUpdated()),
                         this, SIGNAL(gameUpdated()));
    }
    m_prefsDialog->exec();
}


void MainWindow::on_action_SettingsChooseDic_triggered()
{
    if (m_game)
    {
        int res = QMessageBox::question(this, _q("Stop current game?"),
                                        _q("Loading a dictionary will stop the current game. Do you want to continue?"),
                                        QMessageBox::Yes | QMessageBox::Default,
                                        QMessageBox::No | QMessageBox::Escape);
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
            QString dicPath = qs.value(PrefsDialog::kINTF_DIC_PATH, "").toString();
            qs.setValue(PrefsDialog::kINTF_DIC_PATH, fileName);
        }
        catch (std::exception &e)
        {
            displayErrorMsg(e.what());
        }
    }
}


void MainWindow::on_action_WindowsBag_triggered()
{
    if (m_bagWindow == NULL)
    {
        // Create the window
        BagWidget *bag = new BagWidget(NULL);
        bag->setGame(m_game);
        m_bagWindow = new AuxWindow(*bag, _q("Bag"), "BagWindow",
                                    m_ui.action_WindowsBag);
        QObject::connect(this, SIGNAL(gameChanged(const Game*)),
                         bag, SLOT(setGame(const Game*)));
        QObject::connect(this, SIGNAL(gameUpdated()),
                         bag, SLOT(refresh()));
    }
    m_bagWindow->toggleVisibility();
}


void MainWindow::on_action_WindowsBoard_triggered()
{
    if (m_boardWindow == NULL)
    {
        // Create the window
        BoardWidget *board = new BoardWidget(NULL);
        board->setGame(m_game);
        m_boardWindow = new AuxWindow(*board, _q("Board"), "BoardWindow",
                                      m_ui.action_WindowsBoard);
        QObject::connect(this, SIGNAL(gameChanged(const Game*)),
                         board, SLOT(setGame(const Game*)));
        QObject::connect(this, SIGNAL(gameUpdated()),
                         board, SLOT(refresh()));
    }
    m_boardWindow->toggleVisibility();
}


void MainWindow::on_action_WindowsHistory_triggered()
{
    if (m_historyWindow == NULL)
    {
        // Create the window
        HistoryTabWidget *history = new HistoryTabWidget(NULL);
        history->setGame(m_game);
        m_historyWindow = new AuxWindow(*history, _q("History"), "HistoryWindow",
                                        m_ui.action_WindowsHistory);
        QObject::connect(this, SIGNAL(gameChanged(const Game*)),
                         history, SLOT(setGame(const Game*)));
        QObject::connect(this, SIGNAL(gameUpdated()),
                         history, SLOT(refresh()));
    }
    m_historyWindow->toggleVisibility();
}


void MainWindow::on_action_WindowsDicTools_triggered()
{
    if (m_dicToolsWindow == NULL)
    {
        // Create the window
        DicToolsWidget *dicTools = new DicToolsWidget(NULL);
        m_dicToolsWindow = new AuxWindow(*dicTools, _q("Dictionary tools"), "DicTools",
                                    m_ui.action_WindowsDicTools);
        QObject::connect(this, SIGNAL(dicChanged(const Dictionary*)),
                         dicTools, SLOT(setDic(const Dictionary*)));
        // Fake a dictionary selection
        dicTools->setDic(m_dic);
        dicTools->setFocus();
    }
    m_dicToolsWindow->toggleVisibility();
}


void MainWindow::on_action_HelpAbout_triggered()
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
    QMessageBox *aboutBox = new QMessageBox(QMessageBox::Information,
                                            _q("About Eliot"), msg, QMessageBox::Ok, this);
    aboutBox->exec();
}

