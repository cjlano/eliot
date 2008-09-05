/*****************************************************************************
 * Eliot
 * Copyright (C) 2007 Olivier Teulière
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

#include <cstdlib>

#include "settings.h"
#include "game_exception.h"


Settings *Settings::m_instance = NULL;


Settings & Settings::Instance()
{
    if (m_instance == NULL)
    {
        m_instance = new Settings;
    }
    return *m_instance;
}


void Settings::Destroy()
{
    delete m_instance;
    m_instance = NULL;
}


Settings::Settings()
{
    // ============== General options ==============


    // ============== Training mode options ==============


    // ============== Duplicate mode options ==============

    // Minimum number of players in a duplicate game needed to apply a "solo" bonus
    // (16 is the ODS value)
    m_intHandler.addOption("duplicate-solo-players", 16);
    // Number of points granted for a solo (10 is the ODS value)
    m_intHandler.addOption("duplicate-solo-value", 10);

    // If true, Eliot complains when the player does something illegal
    // If false, the word is accepted (with a score of 0) and the player does
    // not get a second chance
    m_boolHandler.addOption("duplicate-reject-invalid", false);


    // ============== Freegame mode options ==============

    // If true, Eliot complains when the player does something illegal
    // If false, the word is accepted (with a score of 0) and the player does
    // not get a second chance.
    // Trying to change letters or to pass the turn in an incorrect way will
    // be rejected in any case.
    m_boolHandler.addOption("freegame-reject-invalid", false);
}


void Settings::setBool(const string &iName, bool iValue)
{
    m_boolHandler.setOption(iName, iValue);
}


bool Settings::getBool(const string &iName) const
{
    return m_boolHandler.getOption(iName);
}


void Settings::setInt(const string &iName, int iValue)
{
    m_intHandler.setOption(iName, iValue);
}


int Settings::getInt(const string &iName) const
{
    return m_intHandler.getOption(iName);
}


template <typename T>
void Settings::OptionsHandler<T>::addOption(const string &iName, const T &iValue)
{
    m_options[iName] = iValue;
}


template <typename T>
void Settings::OptionsHandler<T>::setOption(const string &iName, const T &iValue)
{
    typename map<string, T>::iterator it = m_options.find(iName);
    if (it == m_options.end())
    {
        throw GameException("No such option: " + iName);
    }
    it->second = iValue;
}


template <typename T>
const T& Settings::OptionsHandler<T>::getOption(const string &iName) const
{
    typename map<string, T>::const_iterator it = m_options.find(iName);
    if (it == m_options.end())
    {
        throw GameException("No such option: " + iName);
    }
    return it->second;
}

