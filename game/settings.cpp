/*****************************************************************************
 * Eliot
 * Copyright (C) 2007-2012 Olivier Teulière
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

#include "config.h"

#include <cstdlib>
#ifdef HAVE_LIBCONFIG
#   define LIBCONFIG_STATIC
#   include <libconfig.h++>
#endif
#ifdef WIN32
#   include <windows.h>
#   include <shlobj.h>
#else
#   if defined(HAVE_SYS_STAT_H) && defined(HAVE_SYS_TYPES_H)
#       include <sys/stat.h>
#       include <sys/types.h>
#   endif
#endif

#include "settings.h"
#include "game_exception.h"

using namespace libconfig;


INIT_LOGGER(game, Settings);


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


// Return true if the given path exists and is a directory)
static bool is_directory(const string &path)
{
#ifdef WIN32
    DWORD attrib = GetFileAttributes(path.c_str());
    return (attrib != INVALID_FILE_ATTRIBUTES &&
            (attrib & FILE_ATTRIBUTE_DIRECTORY));
#else
#if defined(HAVE_SYS_STAT_H) && defined(HAVE_SYS_TYPES_H)
    struct stat sb;
    int res = stat(path.c_str(), &sb);
    return res == 0 && S_ISDIR(sb.st_mode);
#else
    // Unimplemented
    return false;
#endif
#endif
}

// Create a directory.
// Return true in case of success, false otherwise.
static bool my_mkdir(const string &dir)
{
#ifdef WIN32
    // The value '248' comes from MSDN
    char tmp[248];
    snprintf(tmp, sizeof(tmp), "%s", dir.c_str());
    return CreateDirectory(tmp, NULL);
#else
#if defined(HAVE_SYS_STAT_H) && defined(HAVE_SYS_TYPES_H)
    // Create the directory with mode 0700
    return mkdir(dir.c_str(), S_IRWXU) == 0;
#else
    // Unimplemented
    return false;
#endif
#endif
}

// Create a directory, like mkdir -p
// We ignore potential errors...
static void full_mkdir(const string &dir)
{
    // Remove trailing '/'
    string copy = dir;
    string::size_type pos = dir.find_last_not_of('/');
    if (pos != string::npos && pos != dir.size() - 1)
        copy.erase(pos + 1, dir.size() - 1 - pos);

    // Create intermediate directories
    pos = 0;
    while ((pos = copy.find('/', pos)) != string::npos)
    {
        // Ignore potential errors...
        my_mkdir(copy.substr(0, pos));
        ++pos;
    }

    // Create the final directory
    my_mkdir(copy);
}


string Settings::GetConfigFileDir()
{
    string dirName;
#ifdef WIN32
    char szPath[MAX_PATH];
    // Get the AppData directory
    if (SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE,
                        NULL, 0, szPath) == S_OK)
    {
        dirName = szPath + string("/eliot");
    }
#else
    // Follow the XDG Base Directory Specification (from freedesktop.org)
    // XXX: In fact we don't follow it to the letter, because the location
    // of the config file could be different when reading and writing.
    // But in the case of Eliot it's not very important (we don't try to
    // merge config files)...
    const char *configDir = getenv("XDG_CONFIG_HOME");
    if (configDir != NULL)
        dirName = configDir;
    else
    {
        // Fallback to the default value: $HOME/.config
        configDir = getenv("HOME");
        if (configDir)
            dirName = configDir + string("/.config");
    }
    dirName += "/eliot";
#endif

    if (dirName != "")
        dirName += "/";

    // Try to create the directory if it doesn't exist.
    // If the directory cannot be created, saving the
    // configuration file will definitely fail...
    if (!is_directory(dirName))
    {
        full_mkdir(dirName);
    }

    return dirName;
}


namespace
{
#ifdef HAVE_LIBCONFIG
    template<typename T>
    void copySetting(const Config &srcConf, Config &dstConf, const char *path)
    {
        if (srcConf.exists(path))
        {
            T t;
            srcConf.lookupValue(path, t);
            dstConf.lookup(path) = t;
        }
    }
#endif
}


Settings::Settings()
{
#ifdef HAVE_LIBCONFIG
    m_fileName = GetConfigFileDir() + "eliot.cfg";
    m_conf = new Config;

    // ============== General options ==============

    // ============== Training mode options ==============
    Setting &training = m_conf->getRoot().add("training", Setting::TypeGroup);

    // Number of search results kept in a search
    training.add("search-limit", Setting::TypeInt) = 100;

    // ============== Duplicate mode options ==============
    Setting &dupli = m_conf->getRoot().add("duplicate", Setting::TypeGroup);

    // Minimum number of players in a duplicate game needed to apply a "solo" bonus
    // (16 is the ODS value)
    dupli.add("solo-players", Setting::TypeInt) = 16;
    // Number of points granted for a solo (10 is the ODS value)
    dupli.add("solo-value", Setting::TypeInt) = 10;

    // If true, Eliot complains when the player does something illegal
    // If false, the word is accepted (with a score of 0) and the player does
    // not get a second chance
    dupli.add("reject-invalid", Setting::TypeBoolean) = true;

    // ============== Freegame mode options ==============
    Setting &freegame = m_conf->getRoot().add("freegame", Setting::TypeGroup);

    // If true, Eliot complains when the player does something illegal
    // If false, the word is accepted (with a score of 0) and the player does
    // not get a second chance.
    // Trying to change letters or to pass the turn in an incorrect way will
    // be rejected in any case.
    freegame.add("reject-invalid", Setting::TypeBoolean) = true;

    // ============== Arbitration mode options ==============
    Setting &arbitration = m_conf->getRoot().add("arbitration", Setting::TypeGroup);

    // If true, a random rack is defined, otherwise the rack is left untouched
    arbitration.add("fill-rack", Setting::TypeBoolean) = true;

    // If true, solos are automatically given when appropriate
    // If false, the arbitrator has full control (but must do everything manually)
    arbitration.add("solo-auto", Setting::TypeBoolean) = true;
    // Minimum number of players in a duplicate game needed to apply a "solo" bonus
    // (16 is the ODS value)
    arbitration.add("solo-players", Setting::TypeInt) = 16;

    // Number of points granted for a solo (10 is the ODS value)
    arbitration.add("solo-value", Setting::TypeInt) = 10;

    // Default value of a penalty
    arbitration.add("penalty-value", Setting::TypeInt) = 5;

    // Maximum number of warnings before getting penalties
    arbitration.add("warnings-limit", Setting::TypeInt) = 3;

    // Number of search results kept in a search
    arbitration.add("search-limit", Setting::TypeInt) = 100;

    // Try to read the values from the configuration file
    try
    {
        // We cannot call readFile() on m_conf, as it removes the previous
        // settings. So we create a temporary config, and copy the settings
        // one by one...
        Config tmpConf;
        tmpConf.readFile(m_fileName.c_str());
        copySetting<int>(tmpConf, *m_conf, "training.search-limit");
        copySetting<int>(tmpConf, *m_conf, "duplicate.solo-players");
        copySetting<int>(tmpConf, *m_conf, "duplicate.solo-value");
        copySetting<bool>(tmpConf, *m_conf, "duplicate.reject-invalid");
        copySetting<bool>(tmpConf, *m_conf, "freegame.reject-invalid");
        copySetting<bool>(tmpConf, *m_conf, "arbitration.fill-rack");
        copySetting<int>(tmpConf, *m_conf, "arbitration.search-limit");
        copySetting<bool>(tmpConf, *m_conf, "arbitration.solo-auto");
        copySetting<int>(tmpConf, *m_conf, "arbitration.solo-players");
        copySetting<int>(tmpConf, *m_conf, "arbitration.solo-value");
        copySetting<int>(tmpConf, *m_conf, "arbitration.penalty-value");
        copySetting<int>(tmpConf, *m_conf, "arbitration.warnings-limit");
    }
    catch (const std::exception &e)
    {
        // Only log the exception
        LOG_ERROR("Error reading config file: " << e.what());
    }
#endif
}


Settings::~Settings()
{
#ifdef HAVE_LIBCONFIG
    delete m_conf;
#endif
}


void Settings::save() const
{
#ifdef HAVE_LIBCONFIG
    try
    {
        m_conf->writeFile(m_fileName.c_str());
    }
    catch (FileIOException &e)
    {
        throw GameException("The configuration file cannot be written (" +
                            m_fileName + ")");
    }
#endif
}


void Settings::setBool(const string &iName, bool iValue)
{
    setValue<bool>(iName, iValue);
}


bool Settings::getBool(const string &iName) const
{
#ifdef HAVE_LIBCONFIG
    try
    {
        return m_conf->lookup(iName);
    }
    catch (SettingNotFoundException &e)
    {
        throw GameException("No such option: " + iName);
    }
#else
    // Dummy implementation
    return true;
#endif
}


void Settings::setInt(const string &iName, int iValue)
{
    setValue<int>(iName, iValue);
}


int Settings::getInt(const string &iName) const
{
#ifdef HAVE_LIBCONFIG
    try
    {
        return m_conf->lookup(iName);
    }
    catch (SettingNotFoundException &e)
    {
        throw GameException("No such option: " + iName);
    }
#else
    // Dummy implementation
    if (iName == "training.search-limit")
        return 100;
    else if (iName == "duplicate.solo-players")
        return 16;
    else if (iName == "duplicate.solo-value")
        return 10;
    else if (iName == "arbitration.search-limit")
        return 100;
    else if (iName == "arbitration.solo-players")
        return 16;
    else if (iName == "arbitration.solo-value")
        return 5;
    else if (iName == "arbitration.penalty-value")
        return 5;
    else if (iName == "arbitration.warnings-limit")
        return 3;
    return 0;
#endif
}


template<class T>
void Settings::setValue(const string &iName, T iValue)
{
#ifdef HAVE_LIBCONFIG
    try
    {
        m_conf->lookup(iName) = iValue;
    }
    catch (SettingNotFoundException &e)
    {
#ifdef DEBUG
        throw GameException("No such option: " + iName);
#endif
    }
#endif
}

