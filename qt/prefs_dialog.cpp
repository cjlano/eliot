/*****************************************************************************
 * Eliot
 * Copyright (C) 2008-2012 Olivier Teulière
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
const QString PrefsDialog::kINTF_SHOW_TOOLBAR = "Interface/ShowToolBar";
const QString PrefsDialog::kINTF_TIMER_TOTAL_DURATION = "Interface/TimerTotalDuration";
const QString PrefsDialog::kINTF_TIMER_ALERT_DURATION = "Interface/TimerAlertDuration";
const QString PrefsDialog::kINTF_TIMER_BEEPS = "Interface/TimerBeeps";
const QString PrefsDialog::kINTF_TIMER_AUTO_START = "Interface/TimerAutoStart";
const QString PrefsDialog::kINTF_CHECK_FOR_UPDATES = "Interface/CheckForUpdates";
const QString PrefsDialog::kARBIT_AUTO_MASTER = "Arbitration/AutoAssignMaster";
const QString PrefsDialog::kARBIT_LINK_7P1 = "Arbitration/LinkRackWith7P1";
const QString PrefsDialog::kDEFAULT_DEF_SITE = "http://fr.wiktionary.org/wiki/%w";

const QString PrefsDialog::kCONFO_START_GAME = "Confirmation/StartOverExisting";
const QString PrefsDialog::kCONFO_LOAD_GAME = "Confirmation/LoadOverExisting";
const QString PrefsDialog::kCONFO_LOAD_DIC = "Confirmation/StopGameForDic";
const QString PrefsDialog::kCONFO_QUIT_GAME = "Confirmation/QuitGame";
const QString PrefsDialog::kCONFO_REPLAY_TURN = "Confirmation/ReplayTurn";
const QString PrefsDialog::kCONFO_ARBIT_REPLACE_MASTER = "Confirmation/Arbitration/ReplaceMaster";
const QString PrefsDialog::kCONFO_ARBIT_LOW_MASTER = "Confirmation/Arbitration/LowMaster";
const QString PrefsDialog::kCONFO_ARBIT_MASTER_JOKERS = "Confirmation/Arbitration/MasterJokers";
const QString PrefsDialog::kCONFO_ARBIT_SUPPR_MOVE = "Confirmation/Arbitration/SuppressMove";
const QString PrefsDialog::kCONFO_ARBIT_REPLACE_MOVE = "Confirmation/Arbitration/ReplaceMove";
const QString PrefsDialog::kCONFO_ARBIT_CHANGE_RACK = "Confirmation/Arbitration/ChangeRack";
const QString PrefsDialog::kCONFO_ARBIT_INCOMPLETE_TURN = "Confirmation/Arbitration/IncompleteTurn";


PrefsDialog::PrefsDialog(QWidget *iParent)
    : QDialog(iParent)
{
    setupUi(this);

    QObject::connect(pushButtonIntfDicBrowse, SIGNAL(clicked()),
                     this, SLOT(browseDic()));

    // Display the first tab
    tabWidget->setCurrentIndex(0);

    lineEditDefSite->setToolTip(_q("URL of the site used to display word definitions.\n"
                                   "In the URL, %w will be replaced with the word in lower case. Examples:\n"
                                   "\thttp://fr.wiktionary.org/wiki/%w\n"
                                   "\thttp://en.wiktionary.org/wiki/%w\n"
                                   "\thttp://images.google.com/images?q=%w"));
    spinBoxTimerTotal->setToolTip(_q("Total duration of the timer, in seconds.\n"
                                     "Changing this value will reset the timer."));
    spinBoxTimerAlert->setToolTip(_q("Number of remaining seconds when an alert is triggered.\n"
                                     "Use a value of -1 to disable the alert.\n"
                                     "Changing this value will reset the timer."));
    checkBoxTimerBeeps->setToolTip(_q("If checked, a beep will be emitted when the timer\n"
                                      "reaches the alert level, and when it reaches 0."));
    checkBoxTimerAutoStart->setToolTip(_q("If checked, the timer will be reinitialized and restarted\n"
                                          "automatically every time that the main rack changes."));
    checkBoxIntfCheckUpdates->setToolTip(_q("If checked, Eliot will connect to the Internet from time to time\n"
                                            "(about once a week) to check if new versions are available.\n"
                                            "New versions are never installed automatically, you just get a notification."));
    spinBoxTrainSearchLimit->setToolTip(_q("Maximum number of results returned by a search.\n"
                                           "The returned results will always be the best ones.\n"
                                           "Use 0 to disable the limit (warning: searches yielding many "
                                           "results could be very slow in this case!)."));
    checkBoxArbitAutoMaster->setToolTip(_q("If checked, a master move will be selected "
                                           "by default when searching the results.\n"
                                           "It is still possible to change the master move afterwards."));
    checkBoxArbitFillRack->setToolTip(_q("If checked, the rack will be completed with random letters.\n"
                                         "Uncheck this option if you prefer to choose the letters yourself."));
    checkBoxArbitSoloAuto->setToolTip(_q("If checked, solos are given automatically, when appropriate.\n"
                                         "Uncheck this option if you prefer to do it manually."));
    spinBoxArbitSearchLimit->setToolTip(spinBoxTrainSearchLimit->toolTip());
    spinBoxArbitWarnLimit->setToolTip(_q("Maximal number of \"acceptable\" warnings.\n"
                                         "Any additional warning will give a penalty to the player."));
    checkBoxToppingElapsedPenalty->setToolTip(_q("If checked, the player gets, at each turn, a score penalty\n"
                                                 "equal to the elapsed time for the turn."));
    spinBoxToppingExpPenalty->setToolTip(_q("Number of points added to the player score when the timer expires.\n"
                                            "Set it to 0 if you don't want any penalty."));

    // Auto-completion on the dictionary path
    QCompleter *completer = new QCompleter(this);
    QFileSystemModel *model = new QFileSystemModel(completer);
    model->setRootPath(QDir::currentPath());
    completer->setModel(model);
    lineEditIntfDicPath->setCompleter(completer);

    // The "arbitration.solo-players" setting is meaningful only
    // when "arbitration.solo-auto" is true
    QObject::connect(checkBoxArbitSoloAuto, SIGNAL(toggled(bool)),
                     labelArbitSoloPlayers, SLOT(setEnabled(bool)));
    QObject::connect(checkBoxArbitSoloAuto, SIGNAL(toggled(bool)),
                     spinBoxArbitSoloPlayers, SLOT(setEnabled(bool)));

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
        int timerTotal = qs.value(kINTF_TIMER_TOTAL_DURATION, 180).toInt();
        spinBoxTimerTotal->setValue(timerTotal);
        int timerAlert = qs.value(kINTF_TIMER_ALERT_DURATION, 30).toInt();
        spinBoxTimerAlert->setValue(timerAlert);
        bool timerBeeps = qs.value(kINTF_TIMER_BEEPS, true).toBool();
        checkBoxTimerBeeps->setChecked(timerBeeps);
        bool timerAutoStart = qs.value(kINTF_TIMER_AUTO_START, false).toBool();
        checkBoxTimerAutoStart->setChecked(timerAutoStart);
        bool checkForUpdates = qs.value(kINTF_CHECK_FOR_UPDATES, true).toBool();
        checkBoxIntfCheckUpdates->setChecked(checkForUpdates);

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
        checkBoxArbitSoloAuto->setChecked(Settings::Instance().getBool("arbitration.solo-auto"));
        spinBoxArbitSoloPlayers->setValue(Settings::Instance().getInt("arbitration.solo-players"));
        spinBoxArbitSoloValue->setValue(Settings::Instance().getInt("arbitration.solo-value"));
        spinBoxArbitPenaltyValue->setValue(Settings::Instance().getInt("arbitration.penalty-value"));
        spinBoxArbitWarnLimit->setValue(Settings::Instance().getInt("arbitration.warnings-limit"));
        spinBoxArbitSearchLimit->setValue(Settings::Instance().getInt("arbitration.search-limit"));

        // Topping settings
        checkBoxToppingElapsedPenalty->setChecked(Settings::Instance().getBool("topping.elapsed-penalty"));
        spinBoxToppingExpPenalty->setValue(Settings::Instance().getInt("topping.timeout-penalty"));

        // Confirmations
        bool confoStartGame = qs.value(kCONFO_START_GAME, true).toBool();
        checkBoxConfoStartGame->setChecked(confoStartGame);
        bool confoLoadGame = qs.value(kCONFO_LOAD_GAME, true).toBool();
        checkBoxConfoLoadDic->setChecked(confoLoadGame);
        bool confoLoadDic = qs.value(kCONFO_LOAD_DIC, true).toBool();
        checkBoxConfoLoadGame->setChecked(confoLoadDic);
        bool confoQuitGame = qs.value(kCONFO_QUIT_GAME, true).toBool();
        checkBoxConfoQuitGame->setChecked(confoQuitGame);
        bool confoReplayTurn = qs.value(kCONFO_REPLAY_TURN, true).toBool();
        checkBoxConfoReplayTurn->setChecked(confoReplayTurn);
        bool confoArbitReplaceMaster = qs.value(kCONFO_ARBIT_REPLACE_MASTER, true).toBool();
        checkBoxConfoArbitReplaceMaster->setChecked(confoArbitReplaceMaster);
        bool confoArbitLowMaster = qs.value(kCONFO_ARBIT_LOW_MASTER, true).toBool();
        checkBoxConfoArbitLowMaster->setChecked(confoArbitLowMaster);
        bool confoArbitMasterJokers = qs.value(kCONFO_ARBIT_MASTER_JOKERS, true).toBool();
        checkBoxConfoArbitMasterJokers->setChecked(confoArbitMasterJokers);
        bool confoArbitSupprMove = qs.value(kCONFO_ARBIT_SUPPR_MOVE, true).toBool();
        checkBoxConfoArbitSupprMove->setChecked(confoArbitSupprMove);
        bool confoArbitReplaceMove = qs.value(kCONFO_ARBIT_REPLACE_MOVE, true).toBool();
        checkBoxConfoArbitReplaceMove->setChecked(confoArbitReplaceMove);
        bool confoArbitChangeRack = qs.value(kCONFO_ARBIT_CHANGE_RACK, true).toBool();
        checkBoxConfoArbitRack->setChecked(confoArbitChangeRack);
        bool confoArbitIncompleteTurn = qs.value(kCONFO_ARBIT_INCOMPLETE_TURN, true).toBool();
        checkBoxConfoArbitEndTurn->setChecked(confoArbitIncompleteTurn);
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
        qs.setValue(kINTF_TIMER_BEEPS, checkBoxTimerBeeps->isChecked());
        if (qs.value(kINTF_TIMER_AUTO_START, false).toBool() != checkBoxTimerAutoStart->isChecked())
        {
            // We need to update the "rack <--> timer" association
            shouldEmitUpdate = true;
            qs.setValue(kINTF_TIMER_AUTO_START, checkBoxTimerAutoStart->isChecked());
        }
        qs.setValue(kINTF_CHECK_FOR_UPDATES, checkBoxIntfCheckUpdates->isChecked());

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
        Settings::Instance().setBool("arbitration.solo-auto",
                                     checkBoxArbitSoloAuto->isChecked());
        Settings::Instance().setInt("arbitration.solo-players",
                                    spinBoxArbitSoloPlayers->value());
        Settings::Instance().setInt("arbitration.solo-value",
                                    spinBoxArbitSoloValue->value());
        Settings::Instance().setInt("arbitration.search-limit",
                                    spinBoxArbitSearchLimit->value());
        Settings::Instance().setInt("arbitration.penalty-value",
                                    spinBoxArbitPenaltyValue->value());
        Settings::Instance().setInt("arbitration.warnings-limit",
                                    spinBoxArbitWarnLimit->value());

        // Topping settings
        Settings::Instance().setBool("topping.elapsed-penalty",
                                     checkBoxToppingElapsedPenalty->isChecked());
        Settings::Instance().setInt("topping.timeout-penalty",
                                    spinBoxToppingExpPenalty->value());


        // Confirmations settings
        qs.setValue(kCONFO_START_GAME, checkBoxConfoStartGame->isChecked());
        qs.setValue(kCONFO_LOAD_GAME, checkBoxConfoLoadGame->isChecked());
        qs.setValue(kCONFO_LOAD_DIC, checkBoxConfoLoadDic->isChecked());
        qs.setValue(kCONFO_QUIT_GAME, checkBoxConfoQuitGame->isChecked());
        qs.setValue(kCONFO_REPLAY_TURN, checkBoxConfoReplayTurn->isChecked());
        qs.setValue(kCONFO_ARBIT_REPLACE_MASTER, checkBoxConfoArbitReplaceMaster->isChecked());
        qs.setValue(kCONFO_ARBIT_LOW_MASTER, checkBoxConfoArbitLowMaster->isChecked());
        qs.setValue(kCONFO_ARBIT_MASTER_JOKERS, checkBoxConfoArbitMasterJokers->isChecked());
        qs.setValue(kCONFO_ARBIT_SUPPR_MOVE, checkBoxConfoArbitSupprMove->isChecked());
        qs.setValue(kCONFO_ARBIT_REPLACE_MOVE, checkBoxConfoArbitReplaceMove->isChecked());
        qs.setValue(kCONFO_ARBIT_CHANGE_RACK, checkBoxConfoArbitRack->isChecked());
        qs.setValue(kCONFO_ARBIT_INCOMPLETE_TURN, checkBoxConfoArbitEndTurn->isChecked());
    }
    catch (GameException &e)
    {
        QMessageBox::warning(this, _q("%1 error").arg(PACKAGE_NAME),
                             _q("Cannot save preferences: %1").arg(e.what()));
    }

    if (shouldEmitUpdate)
        emit prefsUpdated();
}


void PrefsDialog::browseDic()
{
    QString fileName =
        QFileDialog::getOpenFileName(this, _q("Choose a dictionary"), "", "*.dawg");
    if (fileName != "")
        lineEditIntfDicPath->setText(fileName);
}

