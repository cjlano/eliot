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

#ifndef PREFS_DIALOG_H_
#define PREFS_DIALOG_H_

#include <QtGui/QDialog>
#include <QtCore/QString>

#include <ui/prefs_dialog.ui.h>


class PrefsDialog: public QDialog, private Ui::PrefsDialog
{
    Q_OBJECT;

public:
    explicit PrefsDialog(QWidget *iParent = 0);

    static const QString kINTF_ALIGN_HISTORY;
    static const QString kINTF_DIC_PATH;
    static const QString kINTF_SHOW_TILES_POINTS;
    static const QString kINTF_WARN_REPLAY_TURN;
    static const QString kINTF_SHOW_TOOLBAR;
    static const QString kINTF_LINK_TRAINING_7P1;

public slots:
    /// Update the settings when the user selects "OK"
    virtual void accept();

signals:
    void prefsUpdated();

private slots:
    void on_pushButtonIntfDicBrowse_clicked();

private:
    void updateSettings();

};

#endif

