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
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "main_window.h"
#ifdef WIN32
#   include <windows.h>
#endif

using std::string;


int main(int argc, char **argv)
{
#ifdef HAVE_SETLOCALE
    // Set locale via LC_ALL
    setlocale(LC_ALL, "");
#endif

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/images/eliot.xpm"));

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
#else
    static const string localeDir = LOCALEDIR;
#endif
    bindtextdomain(PACKAGE, localeDir.c_str());
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
