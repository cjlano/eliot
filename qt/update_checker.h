/*****************************************************************************
 * Eliot
 * Copyright (C) 2013 Olivier Teulière
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

#ifndef UPDATE_CHECKER_H_
#define UPDATE_CHECKER_H_

#include <QtCore/QObject>
#include <QtCore/QString>

#include "logging.h"

class QNetworkReply;


/**
 * Utility class to check if a new version of Eliot is available
 */
class UpdateChecker: public QObject
{
    Q_OBJECT;
    DEFINE_LOGGER();

    /**
     * Structured representation of a version number
     */
    struct VersionNumber
    {
        int major;
        int minor;
        char letter; // ASCII value, or 0 if no letter is present
        QString suffix; // May be an empty string
    };

public:
    UpdateChecker(QObject *parent);

    void checkForUpdate();

signals:
    void notifyInfo(QString msg);

private slots:
    void updateCheckFinished(QNetworkReply*);

private:

    VersionNumber parseVersionNumber(QString iVersion) const;
    bool isNewer(QString iNewVersion) const;
    void showNewVersion(QString iVersion) const;

};

#endif

