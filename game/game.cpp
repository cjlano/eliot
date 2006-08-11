/*****************************************************************************
 * Copyright (C) 1999-2005 Eliot
 * Authors: Antoine Fraboulet <antoine.fraboulet@free.fr>
 *          Olivier Teuliere  <ipkiss@via.ecp.fr>
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
#include "dic_search.h"
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


const int Game::RACK_SIZE = 7;


Game::Game(const Dictionary &iDic):
    m_dic(&iDic)
{
    m_variant = kNONE;
    m_points = 0;
    m_currPlayer = -1;
    m_finished = false;
}


Game::~Game()
{
    for (int i = 0; i < getNPlayers(); i++)
    {
        delete m_players[i];
    }
}


const Player& Game::getPlayer(int iNum) const
{
    ASSERT(0 <= iNum && iNum < (int)m_players.size(), "Wrong player number");
    return *(m_players[iNum]);
}


/* This function plays a round on the board */
int Game::helperPlayRound(const Round &iRound)
{
    /*
     * We remove tiles from the bag only when they are played
     * on the board. When going back in the game, we must only
     * replace played tiles.
     * We test a rack when it is set but tiles are left in the bag.
     */

    // History of the game
    m_history.setCurrentRack(getCurrentPlayer().getLastRack());
    m_history.playRound(m_currPlayer, m_history.getSize(),  iRound);

    debug("    helper: %d points\n",iRound.getPoints());
    m_points += iRound.getPoints();

    // Before updating the bag and the board, if we are playing a "joker game",
    // we replace in the round the joker by the letter it represents
    // This is currently done by a succession of ugly hacks :-/
    if (m_variant == kJOKER)
    {
        for (int i = 0; i < iRound.getWordLen(); i++)
        {
            if (iRound.isPlayedFromRack(i) && iRound.isJoker(i))
            {
                // Is the represented letter still available in the bag?
                // FIXME: this way to get the represented letter sucks...
                Tile t(toupper(iRound.getTile(i).toChar()));
                Bag bag;
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
                getPlayer(m_currPlayer).getCurrentRack().getAllTiles(tiles);
                for (unsigned int j = 0; j < tiles.size(); j++)
                {
                    bag.replaceTile(tiles[j]);
                }
                getPlayer(m_currPlayer).getLastRack().getAllTiles(tiles);
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
            }
        }
    }

    // Update the bag and the board
    for (int i = 0; i < iRound.getWordLen(); i++)
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
    m_board.addRound(*m_dic, iRound);

    return 0;
}


int Game::back(int n)
{
    int i, j;
    Player *player;

    if (n < 0)
    {
        debug("Game::back negative argument\n");
        n = -n;
    }
    debug("Game::back %d\n",n);
    for (i = 0; i < n; i++)
    {
        if (m_history.getSize() > 0)
        {
            prevPlayer();
            player = m_players[m_currPlayer];
            const Round &lastround = m_history.getPreviousTurn().getRound();
            debug("Game::back last round %s\n",
                  convertToMb(lastround.toString()).c_str());
            /* Remove the word from the board, and put its letters back
             * into the bag */
            m_board.removeRound(*m_dic, lastround);
            for (j = 0; j < lastround.getWordLen(); j++)
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
            player->addPoints(- lastround.getPoints());
            m_points -= lastround.getPoints();
            /* Remove the turns */
            player->removeLastTurn();
            m_history.removeLastTurn();
        }
        else
        {
            return 1;
        }
    }
    return 0;
}

/**
 * The realBag is the current bag minus all the racks
 * present in the game. It represents the actual 
 * letters that are left in the bag.
 */
void Game::realBag(Bag &ioBag) const
{
    vector<Tile> tiles;

    /* Copy the bag */
    ioBag = m_bag;

    /* The real content of the bag depends on the game mode */
    if (getMode() == kFREEGAME)
    {
        /* In freegame mode, take the letters from all the racks */
        for (int i = 0; i < getNPlayers(); i++)
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


int Game::helperSetRackRandom(int p, bool iCheck, set_rack_mode mode)
{
    ASSERT(0 <= p && p < getNPlayers(), "Wrong player number");

    int nold, min;

    // Make a copy of the current player's rack
    PlayedRack pld = getPlayer(p).getCurrentRack();
    nold = pld.nOld();

    // Create a copy of the bag in which we can do everything we want,
    // and take from it the tiles of the players rack so that "bag"
    // contains the right number of tiles.
    Bag bag;
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

    // Nothing in the rack, nothing in the bag --> end of the game
    if (bag.nTiles() == 0 && pld.nTiles() == 0)
    {
        return 1;
    }

    // When iCheck is true, we must make sure that there are at least 2 vowels
    // and 2 consonants in the rack up to the 15th turn, and at least one of
    // them from the 16th turn.
    // So before trying to fill the rack, we'd better make sure there is a way
    // to complete the rack with these constraints...
    min = 0;
    if (iCheck)
    {
        int oldc, oldv;

        if (bag.nVowels() == 0 || bag.nConsonants() == 0)
        {
            return 1;
        }
        // 2 vowels and 2 consonants are needed up to the 15th turn
        if (bag.nVowels() > 1 && bag.nConsonants() > 1
            && m_history.getSize() < 15)
            min = 2;
        else
            min = 1;

        // Count the remaining consonants and vowels in the rack
        vector<Tile> tiles;
        pld.getOldTiles(tiles);
        oldc = 0;
        oldv = 0;
        for (unsigned int i = 0; i < tiles.size(); i++)
        {
            if (tiles[i].isConsonant())
                oldc++;
            if (tiles[i].isVowel())
                oldv++;
        }

        // RACK_SIZE - nold is the number of letters to add
        if (min > oldc + RACK_SIZE - nold ||
            min > oldv + RACK_SIZE - nold)
        {
            // We cannot fill the rack with enough vowels or consonants!
            return 3;
        }
    }

    // Are we dealing with a normal game or a joker game?
    if (m_variant == kJOKER)
    {
        // 1) Is there already a joker in the remaining letters of the rack?
        bool jokerFound = false;
        vector<Tile> tiles;
        pld.getOldTiles(tiles);
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
            bag.takeTile(Tile::Joker());
            pld.addNew(Tile::Joker());
        }

        // 3) Complete the rack normally... but without any joker!
        Tile l;
        while (bag.nTiles() != 0 && pld.nTiles() != RACK_SIZE)
        {
            l = bag.selectRandom();
            if (!l.isJoker())
            {
                bag.takeTile(l);
                pld.addNew(l);
            }
        }
    }
    else // Normal game
    {
        // Get new tiles from the bag
        Tile l;
        while (bag.nTiles() != 0 && pld.nTiles() != RACK_SIZE)
        {
            l = bag.selectRandom();
            bag.takeTile(l);
            pld.addNew(l);
        }
    }

    if (iCheck && !pld.checkRack(min,min))
        return 2;

    m_players[p]->setCurrentRack(pld);

    return 0;
}


/**
 * Check if the players rack can be obtained from the bag.
 * Since letters are removed from the bag only when the
 * round is played we need to check that ALL the racks 
 * are in the bag simultaneously.
 *
 * FIXME: since we do not check for all racks it works
 * for training and duplicate but it won't work for
 * freegames.
 */
bool Game::rackInBag(const Rack &iRack, const Bag &iBag) const
{
    const list<Tile>& allTiles = Tile::getAllTiles();
    list<Tile>::const_iterator it;
    for (it = allTiles.begin(); it != allTiles.end(); it++)
    {
        if (iRack.in(*it) > iBag.in(*it))
            return false;
    }
    return true;
}

/**
 * Set the rack of the player p manually.
 */
int Game::helperSetRackManual(int p, bool iCheck, const wstring &iLetters)
{
    int min, ret;

    PlayedRack pld = getPlayer(p).getCurrentRack();
    pld.reset();

    if ((ret = pld.setManual(iLetters)) > 0)
    {
        return 1; /* add new tests */
    }

    Rack rack;
    pld.getRack(rack);
    if (!rackInBag(rack, m_bag))
    {
        pld.reset();
        return 1;
    }

    if (iCheck)
    {
        if (m_bag.nVowels() > 1 && m_bag.nConsonants() > 1
            && m_history.getSize() < 15)
            min = 2;
        else
            min = 1;
        if (!pld.checkRack(min,min))
            return 2;
    }

    m_players[p]->setCurrentRack(pld);

    return 0;
}

/*********************************************************
 *********************************************************/


int Game::getNHumanPlayers() const
{
    int count = 0;
    for (int i = 0; i < getNPlayers(); i++)
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
    m_players.push_back(new AIPercent(getNPlayers(), 0));
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


/*
 * This function checks whether it is legal to play the given word at the
 * given coordinates. If so, the function fills a Round object, also given as
 * a parameter.
 * Possible return values:
 *  0: correct word, the Round can be used by the caller
 *  1: no dictionary set
 *  2: invalid coordinates (unreadable or out of the board)
 *  3: word not present in the dictionary
 *  4: not enough letters in the rack to play the word
 *  5: word is part of a longer one
 *  6: word overwriting an existing letter
 *  7: invalid crosscheck, or word going out of the board
 *  8: word already present on the board (no new letter from the rack)
 *  9: isolated word (not connected to the rest)
 * 10: first word not horizontal
 * 11: first word not covering the H8 square
 */
int Game::checkPlayedWord(const wstring &iCoord,
                          const wstring &iWord, Round &oRound)
{
    ASSERT(getNPlayers() != 0, "Expected at least one player");

    int res;
    vector<Tile> tiles;
    Tile t;

    /* Init the round with the given coordinates */
    oRound.init();
    oRound.accessCoord().setFromString(iCoord);
    if (!oRound.getCoord().isValid())
    {
        debug("game: incorrect coordinates\n");
        return 2;
    }
    
    /* Check the existence of the word */
    if (Dic_search_word(*m_dic, iWord.c_str()) == 0)
    {
        return 3;
    }

    /* Set the word */
    // TODO: make this a Round_ function (Round_setwordfromchar for example)
    // or a Tiles_ function (to transform a char* into a vector<Tile>)
    // Adding a getter on the word could help too...
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

    /* Check the word position, compute its points,
     * and specify the origin of each letter (board or rack) */
    res = m_board.checkRound(oRound, m_history.getSize() == 0);
    if (res != 0)
        return res + 4;

    /* Check that the word can be formed with the tiles in the rack:
     * we first create a copy of the rack, then we remove the tiles
     * one by one */
    Rack rack;
    Player *player = m_players[m_currPlayer];
    player->getCurrentRack().getRack(rack);

    for (int i = 0; i < oRound.getWordLen(); i++)
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
