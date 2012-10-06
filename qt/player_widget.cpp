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

#include <QtGui/QHBoxLayout>
#include <QtCore/QStringList>

#include "player_widget.h"
#include "play_word_mediator.h"
#include "validator_factory.h"
#include "qtcommon.h"
#include "public_game.h"
#include "player.h"
#include "pldrack.h"
#include "coord.h"
#include "coord_model.h"
#include "dic.h"
#include "debug.h"

#include "encoding.h"


INIT_LOGGER(qt, PlayerWidget);


PlayerWidget::PlayerWidget(QWidget *parent, CoordModel &iCoordModel,
                           unsigned int iPlayerNb, PublicGame *iGame)
    : QWidget(parent), m_game(iGame), m_player(iPlayerNb)
{
    setupUi(this);

    // Use the mediator
    m_mediator = new PlayWordMediator(this, *lineEditPlay, *lineEditCoords,
                                      *lineEditPoints, *pushButtonPlay,
                                      iCoordModel, iGame);
    QObject::connect(m_mediator, SIGNAL(gameUpdated()),
                     this, SIGNAL(gameUpdated()));
    QObject::connect(m_mediator, SIGNAL(notifyProblem(QString)),
                     this, SIGNAL(notifyProblem(QString)));

    lineEditRack->setReadOnly(true);

    if (m_game)
    {
        // Do not allow messing with AI players
        if (!m_game->getPlayer(m_player).isHuman())
            setEnabled(false);
    }
    if (m_game == NULL || m_game->getMode() != PublicGame::kFREEGAME)
    {
        // Hide the freegame-specific controls
        labelChange->hide();
        lineEditChange->hide();
        pushButtonChange->hide();
        pushButtonPass->hide();
    }
    else
    {
        QValidator * val = ValidatorFactory::newChangeValidator(this, *lineEditRack, m_game->getDic());
        lineEditChange->setValidator(val);
    }

    refresh();
}


QSize PlayerWidget::sizeHint() const
{
    return QSize(100, 100);
}


void PlayerWidget::refresh()
{
    if (m_game == NULL)
    {
        lineEditRack->clear();
        return;
    }

    const PlayedRack &pld = m_game->getPlayer(m_player).getCurrentRack();
    lineEditRack->setText(qfw(pld.toString(PlayedRack::RACK_EXTRA)));
    lineEditPlay->clear();
    lineEditCoords->clear();
    lineEditChange->clear();

    if (!m_game->isLastTurn())
    {
        // Do not allow entering a move when displaying an old turn
        setEnabled(false);
    }
    else
    {
        // Do not allow messing with AI players or with players
        // who already played
        setEnabled(!m_game->hasPlayed(m_player) &&
                   m_game->getPlayer(m_player).isHuman());
    }
}


void PlayerWidget::on_pushButtonShuffle_clicked()
{
    m_game->shuffleRack();
    emit gameUpdated();
}


void PlayerWidget::on_lineEditChange_textChanged()
{
    pushButtonChange->setEnabled(lineEditChange->hasAcceptableInput() &&
                                 lineEditChange->text() != "");
    // Force the letters to be in upper case
    lineEditChange->setText(lineEditChange->text().toUpper());
}


void PlayerWidget::on_lineEditChange_returnPressed()
{
    ASSERT(m_game->getMode() == PublicGame::kFREEGAME,
           "Trying to change letters while not in free game mode");

    pass(lineEditChange->text());
}


void PlayerWidget::on_pushButtonPass_clicked()
{
    ASSERT(m_game->getMode() == PublicGame::kFREEGAME,
           "Trying to pass while not in free game mode");

    pass("");
}


void PlayerWidget::pass(QString inputLetters)
{
    // Convert the input string into an internal one
    const wstring &letters =
        m_game->getDic().convertFromInput(wfq(inputLetters));

    // Pass the turn (and possibly change letters)
    int res = m_game->freeGamePass(letters);
    if (res == 0)
        emit gameUpdated();
    else
    {
        QString msg;
        if (inputLetters == "")
            msg = _q("Cannot pass turn:\n%1");
        else
            msg = _q("Cannot change letters '%1':\n%2").arg(inputLetters);
        if (res == 1)
            msg = msg.arg(_q("Changing letters is not allowed when there are less than 7 tiles left in the bag"));
        else if (res == 2)
            msg = msg.arg(_q("The rack of the current player does not contain all the listed letters"));
        else if (res == 3)
            msg = msg.arg(_q("The game is already finished!"));
        else if (res == 4)
            msg = msg.arg(_q("Some letters are invalid for the current dictionary"));
        else
            msg = msg.arg(_q("Unknown error"));
        emit notifyProblem(msg);
    }
}



PlayerTabWidget::PlayerTabWidget(CoordModel &iCoordModel, QWidget *parent)
    : QTabWidget(parent), m_coordModel(iCoordModel)
{
    QObject::connect(this, SIGNAL(currentChanged(int)),
                     this, SLOT(changeCurrentPlayer(int)));
}


void PlayerTabWidget::setGame(PublicGame *iGame)
{
    m_game = iGame;

    // Remove all the tabs
    int nbTabs = count();
    for (int i = 0; i < nbTabs; ++i)
    {
        setCurrentIndex(0);
        // Cut all the connections with the page (needed because removeTab()
        // doesn't really destroy the widget)
        disconnect(currentWidget());
        removeTab(0);
    }

    if (iGame != NULL)
    {
        // Add one tab per player
        for (unsigned int i = 0; i < iGame->getNbPlayers(); ++i)
        {
            const Player &player = iGame->getPlayer(i);
            PlayerWidget *p = new PlayerWidget(NULL, m_coordModel, i, iGame);
            QObject::connect(this, SIGNAL(refreshSignal()), p, SLOT(refresh()));
            // Forward signals to the outside
            QObject::connect(p, SIGNAL(notifyProblem(QString)),
                             this, SIGNAL(notifyProblem(QString)));
            QObject::connect(p, SIGNAL(gameUpdated()),
                             this, SIGNAL(gameUpdated()));
            addTab(p, qfw(player.getName()));
            // Switching to a tab corresponding to an AI player
            // is forbidden
            if (!player.isHuman())
                setTabEnabled(i, false);
        }
        setCurrentIndex(iGame->getCurrentPlayer().getId());
    }
}


void PlayerTabWidget::refresh()
{
    if (m_game)
        setCurrentIndex(m_game->getCurrentPlayer().getId());
    emit refreshSignal();
}


void PlayerTabWidget::changeCurrentPlayer(int p)
{
    // This method is triggered somehow when creating a Duplicate game
    // after a FreeGame one. The next line avoids crashing in this case...
    if (m_game == NULL)
        return;

    m_coordModel.clear();

    // Change the active player when the active tab changes
    // (only in duplicate mode)
    if (m_game->getMode() == PublicGame::kDUPLICATE &&
        widget(p)->isEnabled())
    {
        m_game->duplicateSetPlayer(p);
    }
}

