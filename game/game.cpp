/*****************************************************************************
 * Eliot
 * Copyright (C) 1999-2008 Antoine Fraboulet & Olivier Teulière
 * Authors: Antoine Fraboulet <antoine.fraboulet @@ free.fr>
 *          Olivier Teulière <ipkiss @@ gmail.com>
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

#include <boost/foreach.hpp>

#if ENABLE_NLS
#   include <libintl.h>
#   define _(String) gettext(String)
#else
#   define _(String) String
#endif


#include "dic.h"
#include "tile.h"
#include "rack.h"
#include "round.h"
#include "pldrack.h"
#include "results.h"
#include "player.h"
#include "game.h"
#include "game_factory.h"
#include "turn.h"
#include "encoding.h"
#include "game_exception.h"
#include "turn_cmd.h"

#include "debug.h"

const unsigned int Game::RACK_SIZE =  7;
const int Game::BONUS_POINTS = 50;

Game::Game(const Dictionary &iDic):
    m_dic(iDic), m_bag(iDic)
{
    m_variant = kNONE;
    m_points = 0;
    m_currPlayer = 0;
    m_finished = false;
}


Game::~Game()
{
    BOOST_FOREACH(Player *p, m_players)
    {
        delete p;
    }
}


const Player& Game::getPlayer(unsigned int iNum) const
{
    ASSERT(iNum < m_players.size(), "Wrong player number");
    return *(m_players[iNum]);
}


int Game::back(unsigned int n)
{
    if (m_history.getSize() < n)
        throw GameException("Cannot go back that far");

    for (unsigned int i = 0; i < n+1; ++i)
    {
        m_navigation.prevTurn();
    }
    m_navigation.clearFuture();
#if 0
    for (unsigned int i = 0; i < n; i++)
    {
        prevPlayer();
        const Move &lastMove = m_history.getPreviousTurn().getMove();
        // Nothing to cancel if the move was not a valid round
        if (lastMove.getType() != Move::VALID_ROUND)
            continue;

        const Round &lastround = lastMove.getRound();
        // Remove the word from the board, and put its letters back
        // into the bag
        m_board.removeRound(m_dic, lastround);
        for (unsigned int j = 0; j < lastround.getWordLen(); j++)
        {
            if (lastround.isPlayedFromRack(j))
            {
                if (lastround.isJoker(j))
                    m_bag.replaceTile(Tile::Joker());
                else
                    m_bag.replaceTile(lastround.getTile(j));
            }
        }
        // Remove the points of this round
        m_points -= lastround.getPoints();
        // Remove the turns
        m_players[m_currPlayer]->removeLastTurn();
        m_history.removeLastTurn();
    }
#endif
    return 0;
}


void Game::shuffleRack()
{
    PlayedRack pld = getCurrentPlayer().getCurrentRack();
    pld.shuffle();
    m_players[currPlayer()]->setCurrentRack(pld);
}


void Game::realBag(Bag &ioBag) const
{
    // Copy the bag
    ioBag = m_bag;

    vector<Tile> tiles;

    // The real content of the bag depends on the game mode
    if (getMode() == kFREEGAME)
    {
        // In freegame mode, take the letters from all the racks
        BOOST_FOREACH(const Player *player, m_players)
        {
            player->getCurrentRack().getAllTiles(tiles);
            BOOST_FOREACH(const Tile &tile, tiles)
            {
                ioBag.takeTile(tile);
            }
        }
    }
    else
    {
        // In training or duplicate mode, take the rack of the current
        // player only
        getPlayer(m_currPlayer).getCurrentRack().getAllTiles(tiles);
        BOOST_FOREACH(const Tile &tile, tiles)
        {
            ioBag.takeTile(tile);
        }
    }
}


PlayedRack Game::helperSetRackRandom(const PlayedRack &iPld,
                                     bool iCheck, set_rack_mode mode) const
{
    // FIXME: RACK_MANUAL shouldn't be in the enum
    ASSERT(mode != RACK_MANUAL, "Invalid rack mode");

    // When iCheck is true, we must make sure that there are at least 2 vowels
    // and 2 consonants in the rack up to the 15th turn, and at least one of
    // each starting from the 16th turn.
    // So before trying to fill the rack, we'd better make sure there is a way
    // to complete the rack with these constraints...
    unsigned int min = 0;
    if (iCheck)
    {
        // 2 vowels and 2 consonants are needed up to the 15th turn
        if (m_history.getSize() < 15)
            min = 2;
        else
            min = 1;
    }

    // Make a copy of the given rack
    PlayedRack pld = iPld;
    int nold = pld.getNbOld();

    // Create a copy of the bag in which we can do everything we want,
    // and take from it the tiles of the players rack so that "bag"
    // contains the right number of tiles.
    Bag bag(m_dic);
    realBag(bag);
    if (mode == RACK_NEW && nold != 0)
    {
        // We may have removed too many letters from the bag (i.e. the 'new'
        // letters of the player)
        vector<Tile> tiles;
        pld.getNewTiles(tiles);
        BOOST_FOREACH(const Tile &tile, tiles)
        {
            bag.replaceTile(tile);
        }
        pld.resetNew();
    }
    else if ((mode == RACK_NEW && nold == 0) || mode == RACK_ALL)
    {
        // Replace all the tiles in the bag before choosing random ones
        vector<Tile> tiles;
        pld.getAllTiles(tiles);
        BOOST_FOREACH(const Tile &tile, tiles)
        {
            bag.replaceTile(tile);
        }
        // RACK_NEW with an empty rack is equivalent to RACK_ALL
        pld.reset();
        // Do not forget to update nold, for the RACK_ALL case
        nold = 0;
    }
    else
    {
        throw GameException("Not a random mode");
    }

    // Get the tiles remaining on the rack
    vector<Tile> tiles;
    pld.getOldTiles(tiles);
    ASSERT(tiles.size() < RACK_SIZE,
           "Cannot complete the rack, it is already complete");

    bool jokerAdded = false;
    // Are we dealing with a normal game or a joker game?
    if (m_variant == kJOKER)
    {
        // 1) Is there already a joker in the remaining letters of the rack?
        bool jokerFound = false;
        BOOST_FOREACH(const Tile &tile, tiles)
        {
            if (tile.isJoker())
            {
                jokerFound = true;
                break;
            }
        }

        // 2) If there was no joker, we add one if possible
        if (!jokerFound && bag.in(Tile::Joker()))
        {
            jokerAdded = true;
            pld.addNew(Tile::Joker());
            tiles.push_back(Tile::Joker());
        }

        // 3) Remove all the jokers from the bag, to avoid taking another one
        for (unsigned int i = 0; i < bag.in(Tile::Joker()); ++i)
        {
            bag.takeTile(Tile::Joker());
        }
    }

    // Count the needed consonants and vowels in the rack
    // (i.e. minimum required, minus what we already have in the rack)
    unsigned int neededVowels = min;
    unsigned int neededConsonants = min;
    BOOST_FOREACH(const Tile &tile, tiles)
    {
        if (neededVowels > 0 && tile.isVowel())
            neededVowels--;
        if (neededConsonants > 0 && tile.isConsonant())
            neededConsonants--;
    }

    // Nothing in the rack, nothing in the bag --> end of the (free)game
    if (bag.getNbTiles() == 0 && pld.getNbTiles() == 0)
    {
        throw EndGameException(_("The bag is empty"));
    }

    // Check whether it is possible to complete the rack properly
    if (bag.getNbVowels() < neededVowels ||
        bag.getNbConsonants() < neededConsonants)
    {
        throw EndGameException(_("Not enough vowels or consonants to complete the rack"));
    }
    // End of game condition
    if (iCheck)
    {
        // FIXME: redundant checks?
        if (bag.getNbVowels() < neededVowels ||
            bag.getNbConsonants() < neededConsonants ||
            (bag.getNbTiles() + tiles.size()) == 1)
        {
            throw EndGameException(_("Not enough vowels or consonants to complete the rack"));
        }
    }

    // Handle reject:
    // Now that the joker has been dealt with, we try to complete the rack
    // with truly random tiles. If it meets the requirements (i.e. if there
    // are at least "min" vowels and "min" consonants in the rack), fine.
    // Otherwise, we reject the rack completely, and we try again
    // to complete it, but this time we ensure by construction that the
    // requirements will be met.
    while (bag.getNbTiles() != 0 && pld.getNbTiles() < RACK_SIZE)
    {
        const Tile &l = bag.selectRandom();
        bag.takeTile(l);
        pld.addNew(l);
    }

    if (!pld.checkRack(min, min))
    {
        // Bad luck... we have to reject the rack
        vector<Tile> rejectedTiles;
        pld.getAllTiles(rejectedTiles);
        BOOST_FOREACH(const Tile &rejTile, rejectedTiles)
        {
            bag.replaceTile(rejTile);
        }
        pld.reset();
        // Do not mark the rack as rejected if it was empty
        if (nold > 0)
            pld.setReject();
        // Reset the number of required vowels and consonants
        neededVowels = min;
        neededConsonants = min;

        // Restore the joker if we are in a joker game
        if (jokerAdded)
            pld.addNew(Tile::Joker());

        // RACK_SIZE - tiles.size() is the number of letters to add to the rack
        if (neededVowels > RACK_SIZE - tiles.size() ||
            neededConsonants > RACK_SIZE - tiles.size())
        {
            // We cannot fill the rack with enough vowels or consonants!
            // Actually this should never happen, but it doesn't hurt to check...
            // FIXME: this test is not completely right, because it supposes no
            // letter can be at the same time a vowel and a consonant
            throw EndGameException("Not enough vowels or consonants to complete the rack");
        }

        // Get the required vowels and consonants first
        for (unsigned int i = 0; i < neededVowels; ++i)
        {
            const Tile &l = bag.selectRandomVowel();
            bag.takeTile(l);
            pld.addNew(l);
            // Handle the case where the vowel can also be considered
            // as a consonant
            if (l.isConsonant() && neededConsonants > 0)
                neededConsonants--;
        }
        for (unsigned int i = 0; i < neededConsonants; ++i)
        {
            const Tile &l = bag.selectRandomConsonant();
            bag.takeTile(l);
            pld.addNew(l);
        }

        // The difficult part is done:
        //  - we have handled joker games
        //  - we have handled the checks
        // Now complete the rack with truly random letters
        while (bag.getNbTiles() != 0 && pld.getNbTiles() < RACK_SIZE)
        {
            const Tile &l = bag.selectRandom();
            bag.takeTile(l);
            pld.addNew(l);
        }
    }

    // Shuffle the new tiles, to hide the order we imposed (joker first in a
    // joker game, then needed vowels, then needed consonants, and rest of the
    // rack)
    pld.shuffleNew();

    // Post-condition check. This should never fail, of course :)
    ASSERT(pld.checkRack(min, min), "helperSetRackRandom() is buggy!");

#if 0
    // Until now we didn't modify anything except local variables.
    // Let's "commit" the changes
    m_players[p]->setCurrentRack(pld);
#endif

    return pld;
}


bool Game::rackInBag(const Rack &iRack, const Bag &iBag) const
{
    BOOST_FOREACH(const Tile &t, m_dic.getAllTiles())
    {
        if (iRack.in(t) > iBag.in(t))
            return false;
    }
    return true;
}


int Game::helperSetRackManual(unsigned int p, bool iCheck, const wstring &iLetters)
{
    ASSERT(p < getNPlayers(), "Wrong player number");

    if (!m_dic.validateLetters(iLetters, L"+-"))
        return 3;

    PlayedRack pld;
    pld.setManual(iLetters);

    Rack rack;
    pld.getRack(rack);
    if (!rackInBag(rack, m_bag))
    {
        return 1;
    }

    if (iCheck)
    {
        int min;
        if (m_bag.getNbVowels() > 1 && m_bag.getNbConsonants() > 1
            && m_history.getSize() < 15)
            min = 2;
        else
            min = 1;
        if (!pld.checkRack(min, min))
        {
            return 2;
        }
    }

    m_players[p]->setCurrentRack(pld);

    return 0;
}

/*********************************************************
 *********************************************************/


unsigned int Game::getNHumanPlayers() const
{
    unsigned int count = 0;
    BOOST_FOREACH(const Player *player, m_players)
    {
        count += (player->isHuman() ? 1 : 0);
    }
    return count;
}


void Game::addPlayer(Player *iPlayer)
{
    ASSERT(iPlayer != NULL, "Invalid player pointer in addPlayer()");

    // The ID of the player is its position in the m_players vector
    iPlayer->setId(getNPlayers());
    m_players.push_back(iPlayer);
}


void Game::prevPlayer()
{
    ASSERT(getNPlayers() != 0, "Expected at least one player");

    if (m_currPlayer == 0)
        m_currPlayer = getNPlayers() - 1;
    else
        m_currPlayer--;
}


void Game::nextPlayer()
{
    ASSERT(getNPlayers() != 0, "Expected at least one player");

    if (m_currPlayer == getNPlayers() - 1)
        m_currPlayer = 0;
    else
        m_currPlayer++;
}


int Game::checkPlayedWord(const wstring &iCoord,
                          const wstring &iWord, Round &oRound) const
{
    ASSERT(getNPlayers() != 0, "Expected at least one player");

    if (!m_dic.validateLetters(iWord))
        return 1;

    // Init the round with the given coordinates
    oRound.init();
    oRound.accessCoord().setFromString(iCoord);
    if (!oRound.getCoord().isValid())
    {
        return 2;
    }

    // Check the existence of the word
    if (!m_dic.searchWord(iWord))
    {
        return 3;
    }

    // Set the word
    // TODO: make this a Round_ function (Round_setwordfromchar for example)
    // or a Tiles_ function (to transform a char* into a vector<Tile>)
    // Adding a getter on the word could help too...
    vector<Tile> tiles;
    for (unsigned int i = 0; i < iWord.size(); i++)
    {
        tiles.push_back(Tile(iWord[i]));
    }
    oRound.setWord(tiles);
    for (unsigned int i = 0; i < iWord.size(); i++)
    {
        if (islower(iWord[i]))
            oRound.setJoker(i);
    }

    // Check the word position, compute its points,
    // and specify the origin of each letter (board or rack)
    int res = m_board.checkRound(oRound);
    if (res != 0)
        return res + 4;

    // Check that the word can be formed with the tiles in the rack:
    // we first create a copy of the rack, then we remove the tiles
    // one by one
    Rack rack;
    Player *player = m_players[m_currPlayer];
    player->getCurrentRack().getRack(rack);

    Tile t;
    for (unsigned int i = 0; i < oRound.getWordLen(); i++)
    {
        if (oRound.isPlayedFromRack(i))
        {
            if (oRound.isJoker(i))
                t = Tile::Joker();
            else
                t = oRound.getTile(i);

            if (!rack.in(t))
            {
                return 4;
            }
            rack.remove(t);
        }
    }

    return 0;
}

