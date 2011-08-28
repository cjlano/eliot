/*****************************************************************************
 * Eliot
 * Copyright (C) 2011 Olivier Teulière
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
        kDUPLICATE
    };

    /**
     * Game variants: they slightly modifies the rules of the game.
     * Note that the Joker and Explosive variants are incompatible.
     */
    static const unsigned int kNO_VARIANT = 0;        // Normal game rules
    static const unsigned int kJOKER_VARIANT = 1;     // Joker game
    static const unsigned int kEXPLOSIVE_VARIANT = 2; // "Explosive" game
    static const unsigned int k7AMONG8_VARIANT = 4;   // Play up to 7 letters from a rack containing 8

    /**
     * The dictionary does not belong to this class
     * (it won't be destroyed by ~GameParams())
     */
    GameParams(const Dictionary &iDic,
               GameMode iMode, unsigned int iVariants = 0)
        : m_dic(iDic), m_mode(iMode), m_variants(iVariants)
    {
        // Set default values
        m_rackSize = hasVariant(k7AMONG8_VARIANT) ? 8 : 7;
        m_lettersToPlay = 7;
        m_bonusPoints = 50;

        // Sanity check
        if (hasVariant(kJOKER_VARIANT) && hasVariant(kEXPLOSIVE_VARIANT))
            throw GameException("Incompatible variants: Joker and Explosive");
    }

    // Getters
    const Dictionary & getDic() const { return m_dic; }
    GameMode getMode() const { return m_mode; }
    bool hasVariant(unsigned int iVariant) const { return m_variants & iVariant; }
    int getRackSize() const { return m_rackSize; }
    int getLettersToPlay() const { return m_lettersToPlay; }
    int getBonusPoints() const { return m_bonusPoints; }

 private:
    const Dictionary &m_dic;
    GameMode m_mode;
    unsigned int m_variants;
    int m_rackSize;
    int m_lettersToPlay;
    int m_bonusPoints;
};

#endif

