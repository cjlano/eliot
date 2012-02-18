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

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <string>
#include <map>

#include "logging.h"

using std::string;
using std::map;

namespace libconfig
{
    class Config;
}


/**
 * This class centralizes the various configuration options of Eliot.
 * It implements the Singleton pattern.
 *
 * Currently, there are few settings, and their initial value is hard-coded.
 * It is possible to load/save the settings from/to a configuration file.
 * In a later phase, it should be possible to override configuration
 * settings with settings given on the command-line (TODO).
 */
class Settings
{
    DEFINE_LOGGER();
public:
    /// Access to the singleton
    static Settings& Instance();
    /// Destroy the singleton cleanly
    static void Destroy();

    ~Settings();

    /// Save the current value of the settings to a configuration file
    void save() const;

    void setBool(const string &iName, bool iValue);
    bool getBool(const string &iName) const;

    void setInt(const string &iName, int iValue);
    int getInt(const string &iName) const;

private:

    /// Singleton instance
    static Settings *m_instance;
    Settings();

    /// Name of the file used to store the settings
    string m_fileName;

    libconfig::Config *m_conf;

    template<class T>
    void setValue(const string &iName, T iValue);
};

#endif

