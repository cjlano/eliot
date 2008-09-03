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

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "main_window.h"

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
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    // Translations for Qt's own strings
    QTranslator translator;
    // Set the path for the translation file
#if !defined( WIN32 )
    QString path = QString(QT4LOCALEDIR);
#else
    QString path = QString(LOCALEDIR) + "/qt4/";
#endif
    QString lang = QLocale::system().name();
    translator.load(path + "qt_" + lang);
    app.installTranslator(&translator);
#endif

    MainWindow qmain;
    qmain.move(200, 200);
    qmain.show();
    return app.exec();
}
