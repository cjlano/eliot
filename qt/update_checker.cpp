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

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <QtGui/QMessageBox>

#include <QtCore/QSettings>
#include <QtCore/QDate>
#include <QtCore/QVariant>

#include "config.h"

#include "update_checker.h"
#include "prefs_dialog.h"
#include "qtcommon.h"


using namespace std;

INIT_LOGGER(qt, UpdateChecker);

#define SETTING_KEY "Interface/NextUpdateCheck"
//#define URL "http://www.nongnu.org/eliot/latest-version"
#define URL "http://localhost/latest-version"


UpdateChecker::UpdateChecker(QObject *parent)
    : QObject(parent)
{
}


void UpdateChecker::checkForUpdate()
{
    QSettings qs;

    // Do nothing if the updates are deactivated in the preferences
    if (!qs.value(PrefsDialog::kINTF_CHECK_FOR_UPDATES, true).toBool())
        return;

    QString dateStr = qs.value(SETTING_KEY, "").toString();
    QDate nextCheckDate = QDate::fromString(dateStr, Qt::ISODate);

    // If the next date for the check is in the future, nothing to do
    if (nextCheckDate.isValid() && nextCheckDate > QDate::currentDate())
        return;

    emit notifyInfo(_q("Checking for updates..."));

    QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);
    QObject::connect(networkManager, SIGNAL(finished(QNetworkReply*)),
                     this, SLOT(updateCheckFinished(QNetworkReply*)));
    networkManager->get(QNetworkRequest(QUrl(URL)));
}


void UpdateChecker::updateCheckFinished(QNetworkReply *iReply)
{
    // Check at most once a week
    const int nbDaysToWait = 7;

    // Save the new check date
    QDate nextCheckDate = QDate::currentDate().addDays(nbDaysToWait);
    QSettings qs;
    qs.setValue(SETTING_KEY, nextCheckDate.toString(Qt::ISODate));

    if (iReply->error() == QNetworkReply::NoError)
    {
        LOG_INFO("Update file retrieved successfully");
        QByteArray data = iReply->readAll().trimmed();
        bool newer = isNewer(data);

        if (newer)
        {
            LOG_INFO("New version available: " << lfq(data));
            showNewVersion(data);
        }
        else
            emit notifyInfo(_q("Update check completed, no new version available"));
    }
    else
    {
        LOG_ERROR("Could not retrieve update file: " << lfq(iReply->errorString()));
        emit notifyInfo(_q("Update check failed. Please check your internet connection"));
    }

    // Delete the reply
    iReply->deleteLater();
}


UpdateChecker::VersionNumber UpdateChecker::parseVersionNumber(QString iVersion) const
{
    VersionNumber vn;

    // A version number has the following form: 1.12a-git (where 'a' is an
    // optional letter, and -git is optional as well)
    // Regexp to the rescue!
    QRegExp re("^(\\d+)\\.(\\d+)([a-z])?(-git.*)?$");
    re.setPatternSyntax(QRegExp::RegExp2);
    if (re.indexIn(iVersion) == -1)
    {
        LOG_ERROR("Error parsing version number: " << lfq(iVersion));
        vn.major = -1;
        return vn;
    }
    vn.major = re.cap(1).toInt();
    vn.minor = re.cap(2).toInt();
    vn.letter = 0;
    if (re.pos(3) != -1)
        vn.letter = re.cap(3)[0].toAscii();
    vn.suffix = re.cap(4);

    LOG_DEBUG("Parsed version number: " << vn.major << "." << vn.minor <<
              (vn.letter ? string(1, vn.letter) : "") << lfq(vn.suffix) <<
              " (from '" << lfq(iVersion) << "')");
    return vn;
}


bool UpdateChecker::isNewer(QString iNewVersion) const
{
    // Before doing a lot of work, check the most common case first
    if (iNewVersion == PACKAGE_VERSION)
        return false;

    // Parse the 2 version strings
    const VersionNumber &currVN = parseVersionNumber(PACKAGE_VERSION);
    const VersionNumber &newVN = parseVersionNumber(iNewVersion);
    if (currVN.major == -1 || newVN.major == -1)
        return false;

    // We have the following order:
    //  - 1.* < 2.*
    //  - 1.12* < 1.12a* < 1.12b* < ... < 1.13*
    //  - 1.12 < 1.13-git < 1.13

    // Compare the major numbers
    if (newVN.major > currVN.major)
        return true;
    if (newVN.major < currVN.major)
        return false;

    // Compare the minor numbers
    if (newVN.minor > currVN.minor)
        return true;
    if (newVN.minor < currVN.minor)
        return false;

    // Compare the letters
    if (newVN.letter != 0 || currVN.letter != 0)
    {
        if (newVN.letter == 0 || currVN.letter == 0)
            return currVN.letter == 0;
        if (newVN.letter > currVN.letter)
            return true;
        if (newVN.letter < currVN.letter)
            return false;
    }

    // Compare the suffixes
    if (newVN.suffix != "" || currVN.suffix != "")
    {
        if (newVN.suffix == "" || currVN.suffix == "")
            return newVN.suffix == "";
    }

    // If we reach this point, the 2 version numbers have (different) suffixes.
    // This is not expected, so we are conservative...
    LOG_WARN("Cannot compare version numbers: " <<
             PACKAGE_VERSION << " and " << lfq(iNewVersion));
    return false;
}


void UpdateChecker::showNewVersion(QString iVersion) const
{
    const QString url = _q("http://www.nongnu.org/eliot/en/");
    // TRANSLATORS: Here %1 represents a version number.
    QString msg = _q("Eliot %1 is available.").arg(iVersion);
    msg += "<br>" + _q("You can download it from %1.")
        .arg(QString("<a href=\"%1\">%2</a>").arg(url).arg(url));
    msg += "<br><br>" + _q("This message will be displayed at most once a week.");
    QMessageBox infoBox(QMessageBox::Information, _q("New version available"),
                         msg, QMessageBox::Ok);
    infoBox.setTextFormat(Qt::RichText);
    infoBox.exec();
}


