/*******************************************************************
 * Eliot
 * Copyright (C) 2008-2009 Olivier Teulière
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

#include "public_game.h"
#include "game_params.h"
#include "game.h"
#include "training.h"
#include "duplicate.h"
#include "freegame.h"
#include "game_factory.h"
#include "game_exception.h"
#include "xml_writer.h"


PublicGame::PublicGame(Game &iGame)
    : m_game(iGame)
{
}


PublicGame::~PublicGame()
{
    delete &m_game;
}


PublicGame::GameMode PublicGame::getMode() const
{
    if (dynamic_cast<Duplicate*>(&m_game))
        return kDUPLICATE;
    else if (dynamic_cast<FreeGame*>(&m_game))
        return kFREEGAME;
    else
        return kTRAINING;
}


const GameParams & PublicGame::getParams() const
{
    return m_game.getParams();
}


const Dictionary & PublicGame::getDic() const
{
    return m_game.getDic();
}


const Board& PublicGame::getBoard() const
{
    return m_game.getBoard();
}


const Bag& PublicGame::getBag() const
{
    return m_game.getBag();
}


const History& PublicGame::getHistory() const
{
    return m_game.getHistory();
}


void PublicGame::addPlayer(Player *iPlayer)
{
    m_game.addPlayer(iPlayer);
}


const Player& PublicGame::getPlayer(unsigned int iNum) const
{
    return m_game.getPlayer(iNum);
}


const Player& PublicGame::getCurrentPlayer() const
{
    return m_game.getCurrentPlayer();
}


unsigned int PublicGame::getNbPlayers() const
{
    return m_game.getNPlayers();
}


unsigned int PublicGame::getNbHumanPlayers() const
{
    return m_game.getNHumanPlayers();
}


bool PublicGame::hasPlayed(unsigned int player) const
{
    return m_game.hasPlayed(player);
}

void PublicGame::start()
{
    m_game.start();
}


bool PublicGame::isFinished() const
{
    return m_game.isFinished();
}


int PublicGame::play(const wstring &iWord, const wstring &iCoord)
{
    return m_game.play(iCoord, iWord);
}


int PublicGame::computePoints(const wstring &iWord, const wstring &iCoord) const
{
    Round round;
    int res = m_game.checkPlayedWord(iCoord, iWord, round);
    if (res > 0)
        return -res;
    return round.getPoints();
}


void PublicGame::shuffleRack()
{
    m_game.shuffleRack();
}


void PublicGame::setTestRound(const Round &iRound)
{
    m_game.accessBoard().testRound(iRound);
}


void PublicGame::removeTestRound()
{
    m_game.accessBoard().removeTestRound();
}

/***************************/

static Training & getTrainingGame(Game &iGame)
{
    Training *trGame = dynamic_cast<Training *>(&iGame);
    if (trGame == NULL)
    {
        throw GameException("Invalid game type");
    }
    return *trGame;
}

void PublicGame::trainingSearch()
{
    getTrainingGame(m_game).search();
}


const Results& PublicGame::trainingGetResults() const
{
    return getTrainingGame(m_game).getResults();
}


int PublicGame::trainingPlayResult(unsigned int iResultIndex)
{
    return getTrainingGame(m_game).playResult(iResultIndex);
}


void PublicGame::trainingSetRackRandom(bool iCheck, RackMode iRackMode)
{
    if (iRackMode == kRACK_NEW)
        getTrainingGame(m_game).setRackRandom(iCheck, Game::RACK_NEW);
    else
        getTrainingGame(m_game).setRackRandom(iCheck, Game::RACK_ALL);
}


void PublicGame::trainingSetRackManual(bool iCheck, const wstring &iLetters)
{
    getTrainingGame(m_game).setRackManual(iCheck, iLetters);
}

/***************************/

static Duplicate & getDuplicateGame(Game &iGame)
{
    Duplicate *dupGame = dynamic_cast<Duplicate *>(&iGame);
    if (dupGame == NULL)
    {
        throw GameException("Invalid game type");
    }
    return *dupGame;
}


void PublicGame::duplicateSetPlayer(unsigned int p)
{
    getDuplicateGame(m_game).setPlayer(p);
}

/***************************/

static FreeGame & getFreeGameGame(Game &iGame)
{
    FreeGame *frGame = dynamic_cast<FreeGame *>(&iGame);
    if (frGame == NULL)
    {
        throw GameException("Invalid game type");
    }
    return *frGame;
}


int PublicGame::freeGamePass(const wstring &iToChange)
{
    return getFreeGameGame(m_game).pass(iToChange);
}

/***************************/

PublicGame *PublicGame::load(const string &iFileName, const Dictionary &iDic)
{
    Game *game = GameFactory::Instance()->load(iFileName, iDic);
    return new PublicGame(*game);
}


void PublicGame::save(const string &iFileName) const
{
    XmlWriter::write(m_game, iFileName);
}

/***************************/

unsigned int PublicGame::getCurrTurn() const
{
    // +1 to have a 1-based index (more user-friendly)
    return m_game.getNavigation().getCurrTurn() + 1;
}


unsigned int PublicGame::getNbTurns() const
{
    return m_game.getNavigation().getNbTurns();
}


bool PublicGame::isFirstTurn() const
{
    return m_game.getNavigation().isFirstTurn();
}


bool PublicGame::isLastTurn() const
{
    return m_game.getNavigation().isLastTurn();
}


void PublicGame::firstTurn()
{
    m_game.accessNavigation().firstTurn();
}


void PublicGame::prevTurn()
{
    m_game.accessNavigation().prevTurn();
}


void PublicGame::nextTurn()
{
    m_game.accessNavigation().nextTurn();
}


void PublicGame::lastTurn()
{
    m_game.accessNavigation().lastTurn();
}


void PublicGame::clearFuture()
{
    m_game.accessNavigation().clearFuture();
}


void PublicGame::printTurns() const
{
    m_game.getNavigation().print();
}

