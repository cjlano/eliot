/*****************************************************************************
 * Eliot
 * Copyright (C) 1999-2007 Antoine Fraboulet & Olivier Teulière
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

#include "dic.h"
#include "tile.h"
#include "rack.h"
#include "round.h"
#include "pldrack.h"
#include "results.h"
#include "player.h"
#include "ai_percent.h"
#include "game.h"
#include "game_factory.h"
#include "turn.h"
#include "encoding.h"

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
    for (unsigned int i = 0; i < getNPlayers(); i++)
    {
        delete m_players[i];
    }
}


const Player& Game::getPlayer(unsigned int iNum) const
{
    ASSERT(iNum < m_players.size(), "Wrong player number");
    return *(m_players[iNum]);
}


void Game::helperPlayMove(unsigned int iPlayerId, const Move &iMove)
{
    // History of the game
    m_history.setCurrentRack(getPlayer(iPlayerId).getLastRack());
    m_history.playMove(iPlayerId, m_history.getSize(), iMove);

    // Points
    debug("    helper: %d points\n", iMove.getScore());
    m_points += iMove.getScore();

    // For moves corresponding to a valid round, we have much more
    // work to do...
    if (iMove.getType() == Move::VALID_ROUND)
    {
        helperPlayRound(iPlayerId, iMove.getRound());
    }
}



void Game::helperPlayRound(unsigned int iPlayerId, const Round &iRound)
{
    // Before updating the bag and the board, if we are playing a "joker game",
    // we replace in the round the joker by the letter it represents
    // This is currently done by a succession of ugly hacks :-/
    if (m_variant == kJOKER)
    {
        for (unsigned int i = 0; i < iRound.getWordLen(); i++)
        {
            if (iRound.isPlayedFromRack(i) && iRound.isJoker(i))
            {
                // Is the represented letter still available in the bag?
                // XXX: this way to get the represented letter sucks...
                Tile t(towupper(iRound.getTile(i).toChar()));
                Bag bag(m_dic);
                realBag(bag);
                // FIXME: realBag() does not give us a real bag in this
                // particular case! This is because Player::endTurn() is called
                // before Game::helperPlayRound(), which means that the rack
                // of the player is updated, while the word is not actually
                // played on the board yet. Since realBag() relies on
                // Player::getCurrentRack(), it doesn't remove the letters of
                // the current player, which are in fact available through
                // Player::getLastRack().
                // That's why we have to replace the letters of the current
                // rack and remove the ones from the previous rack...
                // There is a big design problem here, but i am unsure what is
                // the best way to fix it.
                vector<Tile> tiles;
                getPlayer(iPlayerId).getCurrentRack().getAllTiles(tiles);
                for (unsigned int j = 0; j < tiles.size(); j++)
                {
                    bag.replaceTile(tiles[j]);
                }
                getPlayer(iPlayerId).getLastRack().getAllTiles(tiles);
                for (unsigned int j = 0; j < tiles.size(); j++)
                {
                    bag.takeTile(tiles[j]);
                }

                if (bag.in(t))
                {
                    // FIXME: A const_cast sucks too...
                    const_cast<Round&>(iRound).setTile(i, t);
                    // FIXME: This shouldn't be necessary either, this is only
                    // needed because of the stupid way of handling jokers in
                    // rounds
                    const_cast<Round&>(iRound).setJoker(i, false);
                }

                // In a joker game we should have only 1 joker in the rack
                break;
            }
        }
    }

    // Update the bag
    // We remove tiles from the bag only when they are played
    // on the board. When going back in the game, we must only
    // replace played tiles.
    // We test a rack when it is set but tiles are left in the bag.
    for (unsigned int i = 0; i < iRound.getWordLen(); i++)
    {
        if (iRound.isPlayedFromRack(i))
        {
            if (iRound.isJoker(i))
            {
                m_bag.takeTile(Tile::Joker());
            }
            else
            {
                m_bag.takeTile(iRound.getTile(i));
            }
        }
    }

    // Update the board
    m_board.addRound(m_dic, iRound);
}


int Game::back(unsigned int n)
{
    debug("Game::back %d\n",n);
    // TODO: throw an exception
    if (m_history.getSize() < n)
        return 1;

    for (unsigned int i = 0; i < n; i++)
    {
        prevPlayer();
        const Move &lastMove = m_history.getPreviousTurn().getMove();
        // Nothing to cancel if the move was not a valid round
        if (lastMove.getType() != Move::VALID_ROUND)
            continue;

        const Round &lastround = lastMove.getRound();
        debug("Game::back last round %s\n",
              convertToMb(lastround.toString()).c_str());
        /* Remove the word from the board, and put its letters back
         * into the bag */
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
        /* Remove the points of this round */
        m_points -= lastround.getPoints();
        /* Remove the turns */
        m_players[m_currPlayer]->removeLastTurn();
        m_history.removeLastTurn();
    }
    return 0;
}


void Game::realBag(Bag &ioBag) const
{
    vector<Tile> tiles;

    /* Copy the bag */
    ioBag = m_bag;

    /* The real content of the bag depends on the game mode */
    if (getMode() == kFREEGAME)
    {
        /* In freegame mode, take the letters from all the racks */
        for (unsigned int i = 0; i < getNPlayers(); i++)
        {
            getPlayer(i).getCurrentRack().getAllTiles(tiles);
            for (unsigned int j = 0; j < tiles.size(); j++)
            {
                ioBag.takeTile(tiles[j]);
            }
        }
    }
    else
    {
        /* In training or duplicate mode, take the rack of the current
         * player only */
        getPlayer(m_currPlayer).getCurrentRack().getAllTiles(tiles);
        for (unsigned int j = 0; j < tiles.size(); j++)
        {
            ioBag.takeTile(tiles[j]);
        }
    }
}


int Game::helperSetRackRandom(unsigned int p, bool iCheck, set_rack_mode mode)
{
    ASSERT(p < getNPlayers(), "Wrong player number");
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

    // Make a copy of the current player's rack
    PlayedRack pld = getPlayer(p).getCurrentRack();
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
        for (unsigned int i = 0; i < tiles.size(); i++)
        {
            bag.replaceTile(tiles[i]);
        }
        pld.resetNew();
    }
    else if (mode == RACK_NEW && nold == 0 || mode == RACK_ALL)
    {
        // Replace all the tiles in the bag before choosing random ones
        vector<Tile> tiles;
        pld.getAllTiles(tiles);
        for (unsigned int i = 0; i < tiles.size(); i++)
        {
            bag.replaceTile(tiles[i]);
        }
        // RACK_NEW with an empty rack is equivalent to RACK_ALL
        pld.reset();
        // Do not forget to update nold, for the RACK_ALL case
        nold = 0;
    }
    else
    {
        debug("Game::helperSetRackRandom not a random mode\n");
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
        for (unsigned int i = 0; i < tiles.size(); i++)
        {
            if (tiles[i].isJoker())
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
    for (unsigned int i = 0; i < tiles.size(); ++i)
    {
        if (neededVowels > 0 && tiles[i].isVowel())
            neededVowels--;
        if (neededConsonants > 0 && tiles[i].isConsonant())
            neededConsonants--;
    }

    // Nothing in the rack, nothing in the bag --> end of the (free)game
    if (bag.getNbTiles() == 0 && pld.getNbTiles() == 0)
    {
        return 1;
    }

    // Check whether it is possible to complete the rack properly
    if (bag.getNbVowels() < neededVowels ||
        bag.getNbConsonants() < neededConsonants)
    {
        return 1;
    }
    // End of game condition
    if (iCheck)
    {
        if (bag.getNbVowels() < neededVowels ||
            bag.getNbConsonants() < neededConsonants ||
            (bag.getNbTiles() + tiles.size()) == 1)
        {
            return 1;
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
        Tile l = bag.selectRandom();
        bag.takeTile(l);
        pld.addNew(l);
    }

    if (!pld.checkRack(min, min))
    {
        // Bad luck... we have to reject the rack
        vector<Tile> rejectedTiles;
        pld.getAllTiles(rejectedTiles);
        for (unsigned int i = 0; i < rejectedTiles.size(); i++)
        {
            bag.replaceTile(rejectedTiles[i]);
        }
        pld.reset();
        // Do not mark the rack as rejected if it was empty
        if (nold > 0)
            pld.setReject();

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
            return 3;
        }

        // Get the required vowels and consonants first
        for (unsigned int i = 0; i < neededVowels; ++i)
        {
            Tile l = bag.selectRandomVowel();
            bag.takeTile(l);
            pld.addNew(l);
            // Handle the case where the vowel can also be considered
            // as a consonant
            if (l.isConsonant() && neededConsonants > 0)
                neededConsonants--;
        }
        for (unsigned int i = 0; i < neededConsonants; ++i)
        {
            Tile l = bag.selectRandomConsonant();
            bag.takeTile(l);
            pld.addNew(l);
        }

        // The difficult part is done:
        //  - we have handled joker games
        //  - we have handled the checks
        // Now complete the rack with truly random letters
        while (bag.getNbTiles() != 0 && pld.getNbTiles() < RACK_SIZE)
        {
            Tile l = bag.selectRandom();
            bag.takeTile(l);
            pld.addNew(l);
        }
    }

    // Shuffle the new tiles, to hide the order we imposed (joker first in a
    // joker game, then needed vowels, then needed consonants, and rest of the
    // rack)
    pld.shuffleNew();

    // Post-condition check. This should never fail, of course :)
    ASSERT(pld.checkRack(min, min), "helperSetRackRandom() is buggy!")

    // Until now we didn't modify anything except local variables.
    // Let's "commit" the changes
    m_players[p]->setCurrentRack(pld);

    return 0;
}


bool Game::rackInBag(const Rack &iRack, const Bag &iBag) const
{
    const vector<Tile>& allTiles = m_dic.getAllTiles();
    vector<Tile>::const_iterator it;
    for (it = allTiles.begin(); it != allTiles.end(); it++)
    {
        if (iRack.in(*it) > iBag.in(*it))
            return false;
    }
    return true;
}


int Game::helperSetRackManual(unsigned int p, bool iCheck, const wstring &iLetters)
{
    ASSERT(p < getNPlayers(), "Wrong player number");

    if (!m_dic.validateLetters(iLetters, L"+"))
        return 3;

    PlayedRack pld;
    pld.setManual(iLetters);

    Rack rack;
    pld.getRack(rack);
    if (!rackInBag(rack, m_bag))
    {
        pld.reset();
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
            return 2;
    }

    m_players[p]->setCurrentRack(pld);

    return 0;
}

/*********************************************************
 *********************************************************/


unsigned int Game::getNHumanPlayers() const
{
    unsigned int count = 0;
    for (unsigned int i = 0; i < getNPlayers(); i++)
        count += (getPlayer(i).isHuman() ? 1 : 0);
    return count;
}


void Game::addHumanPlayer()
{
    // The ID of the player is its position in the m_players vector
    m_players.push_back(new HumanPlayer(getNPlayers()));
}


void Game::addAIPlayer()
{
    // TODO: allow other percentages, and even other types of AI
    m_players.push_back(new AIPercent(getNPlayers(), 1));
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
                          const wstring &iWord, Round &oRound)
{
    ASSERT(getNPlayers() != 0, "Expected at least one player");

    if (!m_dic.validateLetters(iWord))
        return 1;

    // Init the round with the given coordinates
    oRound.init();
    oRound.accessCoord().setFromString(iCoord);
    if (!oRound.getCoord().isValid())
    {
        debug("game: incorrect coordinates\n");
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
    int res = m_board.checkRound(oRound, m_history.getSize() == 0);
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

/****************************************************************/
/****************************************************************/

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
