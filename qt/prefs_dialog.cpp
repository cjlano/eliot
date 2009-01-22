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

#include <QtCore/QSettings>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include "prefs_dialog.h"
#include "game_exception.h"
#include "settings.h"


const QString PrefsDialog::kINTF_ALIGN_HISTORY = "Interface/AlignHistory";
const QString PrefsDialog::kINTF_DIC_PATH = "Interface/DicPath";
const QString PrefsDialog::kINTF_SHOW_TILES_POINTS = "Interface/ShowTilesPoints";
const QString PrefsDialog::kINTF_WARN_REPLAY_TURN = "Interface/WarnReplayTurn";
const QString PrefsDialog::kINTF_SHOW_TOOLBAR = "Interface/ShowToolBar";
const QString PrefsDialog::kINTF_LINK_TRAINING_7P1 = "Interface/LinkTrainingRackWith7P1";


PrefsDialog::PrefsDialog(QWidget *iParent)
    : QDialog(iParent)
{
    setupUi(this);

    try
    {
        // Interface settings
        QSettings qs(ORGANIZATION, PACKAGE_NAME);
        lineEditIntfDicPath->setText(qs.value(kINTF_DIC_PATH, "").toString());
        checkBoxIntfAlignHistory->setChecked(qs.value(kINTF_ALIGN_HISTORY).toBool());
        bool showPoints = qs.value(kINTF_SHOW_TILES_POINTS, true).toBool();
        checkBoxIntfShowPoints->setChecked(showPoints);
        bool warnReplayTurn = qs.value(kINTF_WARN_REPLAY_TURN, true).toBool();
        checkBoxIntfWarnReplayTurn->setChecked(warnReplayTurn);
        bool linkTraining7P1 = qs.value(kINTF_LINK_TRAINING_7P1, false).toBool();
        checkBoxIntfLinkTraining7P1->setChecked(linkTraining7P1);

        // Duplicate settings
        checkBoxDuplRefuseInvalid->setChecked(Settings::Instance().getBool("duplicate.reject-invalid"));
        spinBoxDuplSoloPlayers->setValue(Settings::Instance().getInt("duplicate.solo-players"));
        spinBoxDuplSoloValue->setValue(Settings::Instance().getInt("duplicate.solo-value"));

        // Freegame settings
        checkBoxFreeRefuseInvalid->setChecked(Settings::Instance().getBool("freegame.reject-invalid"));

        // Training settings
        spinBoxTrainSearchLimit->setValue(Settings::Instance().getInt("training.search-limit"));
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
                             _q("Cannot save the preferences: %1").arg(e.what()));
    }
    QDialog::accept();
}


void PrefsDialog::updateSettings()
{
    bool shouldEmitUpdate = false;

    try
    {
        // Interface settings
        QSettings qs(ORGANIZATION, PACKAGE_NAME);
        qs.setValue(kINTF_DIC_PATH, lineEditIntfDicPath->text());
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
        if (qs.value(kINTF_LINK_TRAINING_7P1, false).toBool() != checkBoxIntfLinkTraining7P1->isChecked())
        {
            // We need to (dis)connect the training widget with the dictionary
            // tools window
            shouldEmitUpdate = true;
            qs.setValue(kINTF_LINK_TRAINING_7P1, checkBoxIntfLinkTraining7P1->isChecked());
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

