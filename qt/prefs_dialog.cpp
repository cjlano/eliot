/*****************************************************************************
 * Eliot
 * Copyright (C) 2008-2009 Olivier Teulière
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

#include <QtCore/QSettings>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtCore/QStringList>
#include <QtGui/QCompleter>
#include <QtGui/QFileSystemModel>

#include "prefs_dialog.h"
#include "game_exception.h"
#include "settings.h"


INIT_LOGGER(qt, PrefsDialog);


const QString PrefsDialog::kINTF_ALIGN_HISTORY = "Interface/AlignHistory";
const QString PrefsDialog::kINTF_DIC_PATH = "Interface/DicPath";
const QString PrefsDialog::kINTF_DEFINITIONS_SITE_URL = "Interface/DefinitionsSiteUrl";
const QString PrefsDialog::kINTF_SHOW_TILES_POINTS = "Interface/ShowTilesPoints";
const QString PrefsDialog::kINTF_WARN_REPLAY_TURN = "Interface/WarnReplayTurn";
const QString PrefsDialog::kINTF_SHOW_TOOLBAR = "Interface/ShowToolBar";
const QString PrefsDialog::kINTF_DEFAULT_AI_LEVEL = "Interface/DefaultAiLevel";
const QString PrefsDialog::kINTF_TIMER_TOTAL_DURATION = "Interface/TimerTotalDuration";
const QString PrefsDialog::kINTF_TIMER_ALERT_DURATION = "Interface/TimerAlertDuration";
const QString PrefsDialog::kARBIT_AUTO_MASTER = "Arbitration/AutoAssignMaster";
const QString PrefsDialog::kARBIT_LINK_7P1 = "Arbitration/LinkRackWith7P1";
const QString PrefsDialog::kDEFAULT_DEF_SITE = "http://fr.wiktionary.org/wiki/%w";


PrefsDialog::PrefsDialog(QWidget *iParent)
    : QDialog(iParent)
{
    setupUi(this);
    lineEditDefSite->setToolTip(_q("URL of the site used to display word definitions.\n"
                                   "In the URL, %w will be replaced with the word in lower case. Examples:\n"
                                   "\thttp://fr.wiktionary.org/wiki/%w\n"
                                   "\thttp://en.wiktionary.org/wiki/%w\n"
                                   "\thttp://images.google.com/images?q=%w"));
    spinBoxDefaultLevel->setToolTip(_q("Default level for Eliot, "
                                       "used when creating a new game.\n"
                                       "Accepted range: [0-100]"));
    spinBoxTimerTotal->setToolTip(_q("Total duration of the timer, in seconds.\n"
                                     "Changing this value will reset the timer."));
    spinBoxTimerAlert->setToolTip(_q("Number of remaining seconds when an alert is triggered.\n"
                                     "Use a value of -1 to disable the alert.\n"
                                     "Changing this value will reset the timer."));
    spinBoxTrainSearchLimit->setToolTip(_q("Maximum number of results returned by a search.\n"
                                           "The returned results will always be the best ones.\n"
                                           "Use 0 to disable the limit (warning: searches yielding many "
                                           "results could be very slow in this case!)."));
    checkBoxArbitAutoMaster->setToolTip(_q("If checked, a Master move will be selected "
                                           "by default when searching the results.\n"
                                           "It is still possible to change the Master move afterwards."));
    checkBoxArbitFillRack->setToolTip(_q("If checked, the rack will be completed with random letters. \n"
                                         "Uncheck this option if you prefer to choose the letters yourself."));
    spinBoxArbitSearchLimit->setToolTip(spinBoxTrainSearchLimit->toolTip());

    // Auto-completion on the dictionary path
    QCompleter *completer = new QCompleter(this);
    QFileSystemModel *model = new QFileSystemModel(completer);
    model->setRootPath(QDir::currentPath());
    completer->setModel(model);
    lineEditIntfDicPath->setCompleter(completer);

    try
    {
        // Interface settings
        QSettings qs;
        lineEditIntfDicPath->setText(qs.value(kINTF_DIC_PATH, "").toString());
        lineEditDefSite->setText(qs.value(kINTF_DEFINITIONS_SITE_URL,
                                          kDEFAULT_DEF_SITE).toString());
        checkBoxIntfAlignHistory->setChecked(qs.value(kINTF_ALIGN_HISTORY).toBool());
        bool showPoints = qs.value(kINTF_SHOW_TILES_POINTS, true).toBool();
        checkBoxIntfShowPoints->setChecked(showPoints);
        bool warnReplayTurn = qs.value(kINTF_WARN_REPLAY_TURN, true).toBool();
        checkBoxIntfWarnReplayTurn->setChecked(warnReplayTurn);
        int defaultAiLevel = qs.value(kINTF_DEFAULT_AI_LEVEL, 100).toInt();
        spinBoxDefaultLevel->setValue(defaultAiLevel);
        int timerTotal = qs.value(kINTF_TIMER_TOTAL_DURATION, 180).toInt();
        spinBoxTimerTotal->setValue(timerTotal);
        int timerAlert = qs.value(kINTF_TIMER_ALERT_DURATION, 30).toInt();
        spinBoxTimerAlert->setValue(timerAlert);

        // Duplicate settings
        checkBoxDuplRefuseInvalid->setChecked(Settings::Instance().getBool("duplicate.reject-invalid"));
        spinBoxDuplSoloPlayers->setValue(Settings::Instance().getInt("duplicate.solo-players"));
        spinBoxDuplSoloValue->setValue(Settings::Instance().getInt("duplicate.solo-value"));

        // Freegame settings
        checkBoxFreeRefuseInvalid->setChecked(Settings::Instance().getBool("freegame.reject-invalid"));

        // Training settings
        spinBoxTrainSearchLimit->setValue(Settings::Instance().getInt("training.search-limit"));

        // Arbitration settings
        bool autoAssignMaster = qs.value(kARBIT_AUTO_MASTER, false).toBool();
        checkBoxArbitAutoMaster->setChecked(autoAssignMaster);
        checkBoxArbitFillRack->setChecked(Settings::Instance().getBool("arbitration.fill-rack"));
        bool linkArbit7P1 = qs.value(kARBIT_LINK_7P1, false).toBool();
        checkBoxArbitLink7P1->setChecked(linkArbit7P1);
        spinBoxArbitSearchLimit->setValue(Settings::Instance().getInt("arbitration.search-limit"));
        spinBoxArbitDefPenalty->setValue(Settings::Instance().getInt("arbitration.default-penalty"));
    }
    catch (GameException &e)
    {
        QMessageBox::warning(this, _q("%1 error").arg(PACKAGE_NAME),
                             _q("Cannot load preferences: %1").arg(e.what()));
    }
}


void PrefsDialog::accept()
{
    updateSettings();
    try
    {
        Settings::Instance().save();
    }
    catch (GameException &e)
    {
        QMessageBox::warning(this, _q("%1 error").arg(PACKAGE_NAME),
                             _q("Cannot save preferences: %1").arg(e.what()));
    }
    QDialog::accept();
}


void PrefsDialog::updateSettings()
{
    bool shouldEmitUpdate = false;

    try
    {
        // Interface settings
        QSettings qs;
        qs.setValue(kINTF_DIC_PATH, lineEditIntfDicPath->text());
        qs.setValue(kINTF_DEFINITIONS_SITE_URL, lineEditDefSite->text());
        if (qs.value(kINTF_ALIGN_HISTORY, true).toBool() != checkBoxIntfAlignHistory->isChecked())
        {
            // We need to redraw the history widget
            shouldEmitUpdate = true;
            qs.setValue(kINTF_ALIGN_HISTORY, checkBoxIntfAlignHistory->isChecked());
        }
        if (qs.value(kINTF_SHOW_TILES_POINTS, true).toBool() != checkBoxIntfShowPoints->isChecked())
        {
            // We need to redraw the board
            shouldEmitUpdate = true;
            qs.setValue(kINTF_SHOW_TILES_POINTS, checkBoxIntfShowPoints->isChecked());
        }
        qs.setValue(kINTF_WARN_REPLAY_TURN, checkBoxIntfWarnReplayTurn->isChecked());
        if (qs.value(kINTF_DEFAULT_AI_LEVEL, 100).toInt() != spinBoxDefaultLevel->value())
        {
            // We need to change the default AI level
            shouldEmitUpdate = true;
            qs.setValue(kINTF_DEFAULT_AI_LEVEL, spinBoxDefaultLevel->value());
        }
        if (qs.value(kINTF_TIMER_TOTAL_DURATION, 180).toInt() != spinBoxTimerTotal->value())
        {
            // We need to change the default AI level
            shouldEmitUpdate = true;
            qs.setValue(kINTF_TIMER_TOTAL_DURATION, spinBoxTimerTotal->value());
        }
        if (qs.value(kINTF_TIMER_ALERT_DURATION, 30).toInt() != spinBoxTimerAlert->value())
        {
            // We need to change the default AI level
            shouldEmitUpdate = true;
            qs.setValue(kINTF_TIMER_ALERT_DURATION, spinBoxTimerAlert->value());
        }

        // Duplicate settings
        Settings::Instance().setBool("duplicate.reject-invalid",
                                     checkBoxDuplRefuseInvalid->isChecked());
        Settings::Instance().setInt("duplicate.solo-players",
                                    spinBoxDuplSoloPlayers->value());
        Settings::Instance().setInt("duplicate.solo-value",
                                    spinBoxDuplSoloValue->value());

        // Freegame settings
        Settings::Instance().setBool("freegame.reject-invalid",
                                     checkBoxFreeRefuseInvalid->isChecked());

        // Training settings
        Settings::Instance().setInt("training.search-limit",
                                    spinBoxTrainSearchLimit->value());

        // Arbitration settings
        qs.setValue(kARBIT_AUTO_MASTER, checkBoxArbitAutoMaster->isChecked());
        Settings::Instance().setBool("arbitration.fill-rack",
                                     checkBoxArbitFillRack->isChecked());
        if (qs.value(kARBIT_LINK_7P1, false).toBool() != checkBoxArbitLink7P1->isChecked())
        {
            // We need to (dis)connect the arbitration widget with the dictionary
            // tools window
            shouldEmitUpdate = true;
            qs.setValue(kARBIT_LINK_7P1, checkBoxArbitLink7P1->isChecked());
        }
        Settings::Instance().setInt("arbitration.search-limit",
                                    spinBoxArbitSearchLimit->value());
        Settings::Instance().setInt("arbitration.default-penalty",
                                    spinBoxArbitDefPenalty->value());
    }
    catch (GameException &e)
    {
        QMessageBox::warning(this, _q("%1 error").arg(PACKAGE_NAME),
                             _q("Cannot save preferences: %1").arg(e.what()));
    }

    if (shouldEmitUpdate)
        emit prefsUpdated();
}


void PrefsDialog::on_pushButtonIntfDicBrowse_clicked()
{
    QString fileName =
        QFileDialog::getOpenFileName(this, _q("Choose a dictionary"), "", "*.dawg");
    if (fileName != "")
        lineEditIntfDicPath->setText(fileName);
}

