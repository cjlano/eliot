/*****************************************************************************
 * Eliot
 * Copyright (C) 2011-2012 Olivier Teulière
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

#ifndef GAME_PARAMS_H_
#define GAME_PARAMS_H_

#include "game_exception.h"
#include "board_layout.h"

class Dictionary;


/**
 * GameParams groups various game characteristics.
 * These attributes are defined when the game is created,
 * and cannot change during the game.
 */
class GameParams
{
 public:

    /// Game mode
    enum GameMode
    {
        kTRAINING,
        kFREEGAME,
        kDUPLICATE,
        kARBITRATION,
        kTOPPING,
    };

    /**
     * Game variants: they slightly modifies the rules of the game.
     * Several variants can be combined, but the Joker
     * and Explosive ones are incompatible.
     */
    enum GameVariant
    {
        kJOKER = 1,     // Joker game
        kEXPLOSIVE = 2, // "Explosive" game
        k7AMONG8 = 4,   // Play up to 7 letters from a rack containing 8
    };

    /**
     * The dictionary does not belong to this class
     * (it won't be destroyed by ~GameParams())
     */
    GameParams(const Dictionary &iDic, GameMode iMode = kTRAINING)
        : m_dic(iDic), m_mode(iMode), m_variants(0)
    {
        // Set default values
        m_rackSize = 7;
        m_lettersToPlay = 7;
        m_bonusPoints = 50;
    }

    // Setters
    void setMode(GameMode iMode) { m_mode = iMode; }
    void addVariant(GameVariant iVariant)
    {
        m_variants |= iVariant;
        // Sanity check
        if (hasVariant(kJOKER) && hasVariant(kEXPLOSIVE))
            throw GameException("Incompatible variants: Joker and Explosive");

        if (hasVariant(k7AMONG8))
            m_rackSize = 8;
    }

    void setBoardLayout(const BoardLayout &iLayout) { m_boardLayout = iLayout; }

    // Getters
    const Dictionary & getDic() const { return m_dic; }
    GameMode getMode() const { return m_mode; }
    bool hasVariant(GameVariant iVariant) const { return m_variants & iVariant; }
    int getRackSize() const { return m_rackSize; }
    int getLettersToPlay() const { return m_lettersToPlay; }
    int getBonusPoints() const { return m_bonusPoints; }

    const BoardLayout & getBoardLayout() const { return m_boardLayout; }

 private:
    BoardLayout m_boardLayout;
    const Dictionary &m_dic;
    GameMode m_mode;
    unsigned int m_variants;
    int m_rackSize;
    int m_lettersToPlay;
    int m_bonusPoints;
};

#endif

