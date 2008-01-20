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

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QDockWidget>

#include "main_window.h"
#include "dic.h"
#include "encoding.h"
#include "header.h"
#include "game.h"
#include "freegame.h"
#include "player.h"
#include "debug.h"
#include "new_game.h"
#include "bag_widget.h"
#include "board_widget.h"
#include "score_widget.h"
#include "player_widget.h"
#include "history_widget.h"
#include "aux_window.h"
#include "qtcommon.h"

#include "round.h"
#include "coord.h"


MainWindow::MainWindow(QWidget *iParent)
    : QMainWindow(iParent), m_dic(NULL), m_game(NULL), m_newGame(NULL),
    m_bagWindow(NULL)
{
    m_ui.setupUi(this);

    // Cascading signals
    QObject::connect(this, SIGNAL(gameChanged(const Game*)),
                     this, SLOT(gameUpdated()));

    // Create the main window
    BoardWidget *boardWidget = new BoardWidget;
    QObject::connect(this, SIGNAL(boardChanged(const Board*)),
                     boardWidget, SLOT(setBoard(const Board*)));

    QDockWidget *dock = new QDockWidget;
    dock->setWidget(boardWidget);
    boardWidget->setWindowTitle(_q("Board"));

    ScoreWidget *scores = new ScoreWidget;
    QObject::connect(this, SIGNAL(gameChanged(const Game*)),
                     scores, SLOT(setGame(const Game*)));

    HistoryWidget *historyWidget = new HistoryWidget;
    QObject::connect(this, SIGNAL(historyChanged(const History*)),
                     historyWidget, SLOT(setHistory(const History*)));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(dock);
    hlayout->addWidget(scores);
    hlayout->addWidget(historyWidget);

    m_ui.groupBoxTest->setLayout(hlayout);

    // XXX: temporary, for testing purposes!
    try
    {
        m_dic = new Dictionary("/home/ipkiss/ods5.dawg");
    }
    catch (...)
    {
        // Ignore the error silently :)
    }
}


MainWindow::~MainWindow()
{
    delete m_bagWindow;
    delete m_game;
    delete m_dic;
}


void MainWindow::on_action_About_triggered()
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


void MainWindow::on_action_Bag_triggered()
{
    if (m_bagWindow == NULL)
    {
        // Create the bag window
        BagWidget *bagWidget =
            new BagWidget(NULL, m_game ? &m_game->getBag() : NULL);
        m_bagWindow = new AuxWindow(*bagWidget, m_ui.action_Bag);
        QObject::connect(this, SIGNAL(bagChanged(const Bag*)),
                         bagWidget, SLOT(setBag(const Bag*)));
        // XXX
        m_bagWindow->move(20, 20);
    }
    if (m_bagWindow->isVisible())
        m_bagWindow->hide();
    else
        m_bagWindow->show();
}


void MainWindow::on_action_ChooseDic_triggered()
{
    QString fileName =
        QFileDialog::getOpenFileName(this, _q("Choose a dictionary"), "", "*.dawg");
    if (!fileName.isEmpty())
    {
        try
        {
            Dictionary *dic = new Dictionary(qtl(fileName));
            delete m_dic;
            m_dic = dic;
            emit dicChanged(fileName, qfw(m_dic->getHeader().getName()));
        }
        catch (std::exception &e)
        {
            displayErrorMsg(e.what());
        }
    }
}


void MainWindow::on_action_New_Game_triggered()
{
    if (m_dic == NULL)
    {
        displayErrorMsg(_q("You have to select a dictionary first!"));
        return;
    }

    if (m_newGame == NULL)
        m_newGame = new NewGame(this);

    int res = m_newGame->exec();
    if (res == QDialog::Rejected)
        return;

    m_game = m_newGame->createGame(*m_dic);
    if (m_game == NULL)
        return;

    int nbTabs = m_ui.tabWidgetPlayers->count();
    for (int i = 0; i < nbTabs; ++i)
        m_ui.tabWidgetPlayers->removeTab(0);

    m_game->start();
    for (unsigned int i = 0; i < m_game->getNPlayers(); ++i)
    {
        PlayerWidget *r = new PlayerWidget(NULL, i, m_game);
        QObject::connect(this, SIGNAL(rackChanged()), r, SLOT(refresh()));
        QObject::connect(r, SIGNAL(playingWord(unsigned int, QString, QString)),
                         this, SLOT(playerPlays(unsigned int, QString, QString)));
        QObject::connect(r, SIGNAL(passing(unsigned int, QString)),
                         this, SLOT(playerPasses(unsigned int, QString)));
        m_ui.tabWidgetPlayers->addTab(r, qfw(m_game->getPlayer(i).getName()));
    }

    emit gameChanged(m_game);
}


void MainWindow::gameUpdated()
{
    emit bagChanged(&m_game->getBag());
    emit boardChanged(&m_game->getBoard());
    emit historyChanged(&m_game->getHistory());
    emit rackChanged();
}


void MainWindow::playerPlays(unsigned int p, QString iWord, QString iCoord)
{
    cerr << "Player " << p << " plays \"" << convertToMb(qtw(iWord))
        << "\" in " << convertToMb(qtw(iCoord)) << endl;
    int res = m_game->play(qtw(iCoord), qtw(iWord));
    if (res)
    {
        // XXX: tmp
        QString error;
        error.sprintf("Error: %d", res);
        displayErrorMsg(error, qfl("playing word"));
        return;
    }
    emit gameChanged(m_game);
}


void MainWindow::playerPasses(unsigned int p, QString iChangedLetters)
{
    if (iChangedLetters == "")
    {
        cerr << "Player " << p << " passes" << endl;
    }
    else
    {
        cerr << "Player " << p << " changes \""
            << convertToMb(qtw(iChangedLetters)) << "\"" << endl;
    }

    FreeGame *free = dynamic_cast<FreeGame*>(m_game);
    ASSERT(free != NULL,
           "MainWindow::playerPasses() called while not in a free game");

    int res = free->pass(qtw(iChangedLetters));
    if (res)
    {
        // XXX: tmp
        QString error;
        error.sprintf("Error: %d", res);
        displayErrorMsg(error, qfl("playing word"));
        return;
    }
    emit bagChanged(&m_game->getBag());
    emit boardChanged(&m_game->getBoard());
    emit historyChanged(&m_game->getHistory());
    emit rackChanged();
}


void MainWindow::displayErrorMsg(QString iMsg, QString iContext)
{
    if (iContext == "")
        iContext = PACKAGE_NAME;

    QMessageBox::warning(this, iContext, iMsg);
}

