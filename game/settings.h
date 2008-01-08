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

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <string>
#include <map>

using std::string;
using std::map;


/**
 * This class centralizes the various configuration options of Eliot.
 * It implements the Singleton pattern.
 *
 * Currently, there are few settings, and their initial value is hard-coded.
 * In a later phase, this class will be able to export/import settings
 * to/from a configuration file, and it should be possible to override
 * configuration settings with settings given on the command-line (TODO).
 * The boost::program_options library could be useful for this.
 *
 * This class will also be helpful for the "Settings" dialog box of the GUI.
 */
class Settings
{
public:
    /// Access to the singleton
    static Settings& Instance();
    /// Destroy the singleton cleanly
    static void Destroy();

    void setBool(const string &iName, bool iValue);
    bool getBool(const string &iName) const;

    void setInt(const string &iName, int iValue);
    int getInt(const string &iName) const;

private:

    /**
     * This nested class is simply there to handle storage and retrieval
     * for options of a particular type (and factorize code)
     */
    template <typename T>
    class OptionsHandler
    {
        public:
            /// Set the value of an option
            /**
             * If the option already exists, its value is replaced,
             * otherwise the option is created
             */
            void addOption(const string &iName, const T &iValue);

            /**
             * Change the value of an existing option.
             * An exception is thrown if the option doesn't exist yet
             */
            void setOption(const string &iName, const T &iValue);

            /**
             * Query the value of an option.
             * An exception is thrown if the option doesn't exist
             */
            const T& getOption(const string &iName) const;

        private:
            map<string, T> m_options;
    };


    /// Singleton instance
    static Settings *m_instance;
    Settings();

    /// The settings can be of various types
    OptionsHandler<bool> m_boolHandler;
    OptionsHandler<int> m_intHandler;
    // Add types as needed...

};

#endif

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
