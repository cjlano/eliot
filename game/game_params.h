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


/**
 * GameParams groups various game characteristics.
 * These attributes are defined when the game is created,
 * and cannot change during the game.
 */
class GameParams
{
 public:
    /// Game variant: it slightly modifies the rules of the game
    enum GameVariant
    {
        kNONE,      // Normal game rules
        kJOKER,     // Joker game
        kEXPLOSIVE, // "Explosive" game
        k7AMONG8,   // Play up to 7 letters from a rack containing 8
    };

    GameParams(GameVariant iVariant = kNONE)
        : m_variant(iVariant)
    {
        // Set default values
        m_rackSize = (iVariant == k7AMONG8) ? 8 : 7;
        m_lettersToPlay = 7;
        m_bonusPoints = 50;
    }

    // Getters
    GameVariant getVariant() const { return m_variant; }
    int getRackSize() const { return m_rackSize; }
    int getLettersToPlay() const { return m_lettersToPlay; }
    int getBonusPoints() const { return m_bonusPoints; }

 private:
    GameVariant m_variant;
    int m_rackSize;
    int m_lettersToPlay;
    int m_bonusPoints;
};

#endif

