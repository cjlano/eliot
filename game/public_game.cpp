/*******************************************************************
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

#include "public_game.h"
#include "game_params.h"
#include "game.h"
#include "training.h"
#include "duplicate.h"
#include "arbitration.h"
#include "freegame.h"
#include "topping.h"
#include "game_factory.h"
#include "game_exception.h"
#include "xml_writer.h"
#include "player.h"
#include "pldrack.h"


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
    if (dynamic_cast<Arbitration*>(&m_game))
        return kARBITRATION;
    else if (dynamic_cast<Duplicate*>(&m_game))
        return kDUPLICATE;
    else if (dynamic_cast<FreeGame*>(&m_game))
        return kFREEGAME;
    else if (dynamic_cast<Topping*>(&m_game))
        return kTOPPING;
    else
        return kTRAINING;
}


const GameParams & PublicGame::getParams() const
{
    return m_game.getParams();
}


bool PublicGame::hasMasterGame() const
{
    return m_game.hasMasterGame();
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


const PlayedRack& PublicGame::getCurrentRack() const
{
    return m_game.getHistory().getCurrentRack();
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


void PublicGame::setPlayerName(unsigned iPlayerId, const wstring &iName)
{
    m_game.accessPlayer(iPlayerId).setName(iName);
}


void PublicGame::setPlayerTableNb(unsigned iPlayerId, unsigned iTableNb)
{
    m_game.accessPlayer(iPlayerId).setTableNb(iTableNb);
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


int PublicGame::checkPlayedWord(const wstring &iWord, const wstring &iCoord, Move &oMove) const
{
    return m_game.checkPlayedWord(iCoord, iWord, oMove, true, false);
}


int PublicGame::computePoints(const wstring &iWord, const wstring &iCoord) const
{
    Move move;
    int res = m_game.checkPlayedWord(iCoord, iWord, move);
    if (res > 0)
        return -res;
    return move.getScore();
}


void PublicGame::shuffleRack()
{
    m_game.shuffleRack();
}


void PublicGame::reorderRack(const PlayedRack &iRack)
{
    m_game.reorderRack(iRack);
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

template <typename T>
static T & getTypedGame(Game &iGame)
{
    T *typedGame = dynamic_cast<T*>(&iGame);
    if (typedGame == NULL)
    {
        throw GameException("Invalid game type");
    }
    return *typedGame;
}

/***************************/

void PublicGame::trainingSearch()
{
    getTypedGame<Training>(m_game).search();
}


const Results& PublicGame::trainingGetResults() const
{
    return getTypedGame<Training>(m_game).getResults();
}


int PublicGame::trainingPlayResult(unsigned int iResultIndex)
{
    return getTypedGame<Training>(m_game).playResult(iResultIndex);
}


void PublicGame::trainingSetRackRandom(bool iCheck, RackMode iRackMode)
{
    if (iRackMode == kRACK_NEW)
        getTypedGame<Training>(m_game).setRackRandom(iCheck, Game::RACK_NEW);
    else
        getTypedGame<Training>(m_game).setRackRandom(iCheck, Game::RACK_ALL);
}


void PublicGame::trainingSetRackManual(bool iCheck, const wstring &iLetters)
{
    getTypedGame<Training>(m_game).setRackManual(iCheck, iLetters);
}

/***************************/

void PublicGame::toppingPlay(const wstring &iWord, const wstring &iCoord, int iElapsed)
{
    getTypedGame<Topping>(m_game).tryWord(iWord, iCoord, iElapsed);
}


vector<Move> PublicGame::toppingGetTriedMoves() const
{
    return getTypedGame<Topping>(m_game).getTriedMoves();
}


Move PublicGame::toppingGetTopMove() const
{
    return getTypedGame<Topping>(m_game).getTopMove();
}

/***************************/

void PublicGame::duplicateSetPlayer(unsigned int p)
{
    getTypedGame<Duplicate>(m_game).setPlayer(p);
}

void PublicGame::duplicateSetMasterMove(const Move &iMove)
{
    getTypedGame<Duplicate>(m_game).setMasterMove(iMove);
}

const Move & PublicGame::duplicateGetMasterMove() const
{
    return getTypedGame<Duplicate>(m_game).getMasterMove();
}

/***************************/

int PublicGame::freeGamePass(const wstring &iToChange)
{
    return getTypedGame<FreeGame>(m_game).pass(iToChange);
}

/***************************/

void PublicGame::arbitrationSetRackRandom()
{
    getTypedGame<Arbitration>(m_game).setRackRandom();
}


void PublicGame::arbitrationSetRackManual(const wstring &iLetters)
{
    getTypedGame<Arbitration>(m_game).setRackManual(iLetters);
}


void PublicGame::arbitrationSearch(LimitResults &oResults)
{
    return getTypedGame<Arbitration>(m_game).search(oResults);
}


Move PublicGame::arbitrationCheckWord(const wstring &iWord,
                                      const wstring &iCoords) const
{
    return getTypedGame<Arbitration>(m_game).checkWord(iWord, iCoords);
}


void PublicGame::arbitrationToggleSolo(unsigned iPlayerId)
{
    Arbitration &game = getTypedGame<Arbitration>(m_game);
    if (game.getSolo(iPlayerId) != 0)
        game.removeSolo(iPlayerId);
    else
        game.setSolo(iPlayerId);
}


int PublicGame::arbitrationGetSolo(unsigned iPlayerId) const
{
    return getTypedGame<Arbitration>(m_game).getSolo(iPlayerId);
}


void PublicGame::arbitrationToggleWarning(unsigned iPlayerId)
{
    Arbitration &game = getTypedGame<Arbitration>(m_game);
    if (game.hasWarning(iPlayerId))
        game.removeWarning(iPlayerId);
    else
        game.addWarning(iPlayerId);
}


bool PublicGame::arbitrationHasWarning(unsigned iPlayerId) const
{
    return getTypedGame<Arbitration>(m_game).hasWarning(iPlayerId);
}


void PublicGame::arbitrationTogglePenalty(unsigned iPlayerId)
{
    Arbitration &game = getTypedGame<Arbitration>(m_game);
    if (game.getPenalty(iPlayerId) != 0)
        game.removePenalty(iPlayerId);
    else
        game.addPenalty(iPlayerId);
}


int PublicGame::arbitrationGetPenalty(unsigned iPlayerId) const
{
    return getTypedGame<Arbitration>(m_game).getPenalty(iPlayerId);
}


void PublicGame::arbitrationAssign(unsigned iPlayerId, const Move &iMove)
{
    getTypedGame<Arbitration>(m_game).assignMove(iPlayerId, iMove);
}


void PublicGame::arbitrationFinalizeTurn()
{
    getTypedGame<Arbitration>(m_game).finalizeTurn();
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

