/*****************************************************************************
 * Eliot
 * Copyright (C) 2008 Olivier Teulière
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

#include <string>
#include <exception>
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "logging.h"
#include "base_exception.h"
#include "stacktrace.h"
#include "main_window.h"
#ifdef WIN32
#   include <windows.h>
#endif
#ifdef __APPLE__
#   include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef HAVE_EXECINFO_H
#   include <signal.h>
#   include <execinfo.h>
#endif

using namespace std;


#ifdef HAVE_EXECINFO_H
static void bt_sighandler(int);
#endif

// Custom QApplication to catch and log exceptions properly
// See http://forum.qtfr.org/viewtopic.php?id=7615
class MyApplication : public QApplication
{
public:
    MyApplication(int argc, char **argv)
        : QApplication(argc, argv)
    {}

    virtual bool notify(QObject *receiver, QEvent *event)
    {
        try
        {
            return QApplication::notify(receiver, event);
        }
        catch (const BaseException &e)
        {
            LOG_ROOT_ERROR("Exception caught: " << e.what() << "\n" << e.getStackTrace());
            return false;
        }
    }
};

int main(int argc, char **argv)
{
#ifdef HAVE_EXECINFO_H
    // Install a custom signal handler to print a backtrace when crashing
    // See http://www.linuxjournal.com/article/6391 for inspiration
    signal(SIGSEGV, &bt_sighandler);
#endif

    // On Mac, running Eliot from the dock does not automatically set the LANG
    // variable, so we do it ourselves.
    // Note: The following block of code is copied from VLC, and slightly
    // modified by me (original author: Pierre d'Herbemont)
#if defined(ENABLE_NLS) && defined(__APPLE__)
    /* Check if $LANG is set. */
    if (NULL == getenv("LANG"))
    {
        // Retrieve the preferred language as chosen in System Preferences.app
        // (note that CFLocaleCopyCurrent() is not used because it returns the
        // preferred locale not language)
        CFArrayRef all_locales = CFLocaleCopyAvailableLocaleIdentifiers();
        CFArrayRef preferred_locales = CFBundleCopyLocalizationsForPreferences(all_locales, NULL);

        if (preferred_locales)
        {
            if (CFArrayGetCount(preferred_locales))
            {
                char psz_locale[50];
                CFStringRef user_language_string_ref = (CFStringRef) CFArrayGetValueAtIndex(preferred_locales, 0);
                CFStringGetCString(user_language_string_ref, psz_locale, sizeof(psz_locale), kCFStringEncodingUTF8);
                setenv("LANG", psz_locale, 1);
            }
            CFRelease(preferred_locales);
        }
        CFRelease(all_locales);
    }
#endif


#ifdef HAVE_SETLOCALE
    // Set locale via LC_ALL
    setlocale(LC_ALL, "");
#ifdef __APPLE__
    // FIXME: Ugly hack: we hardcode the encoding to UTF-8, because I don't
    // know how to retrieve it properly when LANG is not set
    setlocale(LC_CTYPE, "UTF-8");
#endif
#endif

    MyApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/images/eliot.xpm"));
    // Used for QSettings
    app.setApplicationName(PACKAGE_NAME);
    app.setOrganizationName("eliot");

#ifdef ENABLE_NLS
    // Set the message domain
#ifdef WIN32
    // Get the absolute path, as returned by GetFullPathName()
    char baseDir[MAX_PATH];
    GetFullPathName(argv[0], MAX_PATH, baseDir, NULL);
    char *pos = strrchr(baseDir, L'\\');
    if (pos)
        *pos = '\0';
    const string localeDir = baseDir + string("\\locale");
#elif defined(__APPLE__)
    const char *bundlePath = CFStringGetCStringPtr(CFURLCopyFileSystemPath(
            CFBundleCopyBundleURL(CFBundleGetMainBundle()), kCFURLPOSIXPathStyle), CFStringGetSystemEncoding());
    const string localeDir = string(bundlePath) + "/Contents/Resources/locale";
#else
    static const string localeDir = LOCALEDIR;
#endif
    bindtextdomain(PACKAGE, localeDir.c_str());
#ifdef __APPLE__
    // Force messages to UTF-8
    bind_textdomain_codeset(PACKAGE, "UTF-8");
#endif
    textdomain(PACKAGE);

    // Translations for Qt's own strings
    QTranslator translator;
    // Set the path for the translation file
#ifdef WIN32
    QString path = QString(localeDir.c_str()) + "\\qt4";
#else
    QString path = QString(QT4LOCALEDIR);
#endif
    QString lang = QLocale::system().name();
    translator.load(path + "/qt_" + lang);
    app.installTranslator(&translator);
#endif

    MainWindow qmain;
    qmain.show();
    return app.exec();
}

#ifdef HAVE_EXECINFO_H
static void bt_sighandler(int signum)
{
    LOG_ROOT_FATAL("Segmentation fault!");
    LOG_ROOT_FATAL("Backtrace:\n" << StackTrace::GetStack());

    // Restore the default handler to generate a nice core dump
    signal(signum, SIG_DFL);
    raise(signum);
}
#endif

