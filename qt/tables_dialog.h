/*****************************************************************************
 * Eliot
 * Copyright (C) 2012 Olivier Teulière
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

#ifndef TABLES_DIALOG_H_
#define TABLES_DIALOG_H_

#include <QDialog>

#include "ui/tables_dialog.ui.h"
#include "logging.h"


class PublicGame;


class TablesDialog: public QDialog, private Ui::TablesDialog
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    explicit TablesDialog(QWidget *iParent, PublicGame &iGame);

signals:
    void notifyProblem(QString iMsg);

public slots:
    /// Update the players table numbers when the user selects "OK"
    virtual void accept();

private:
    PublicGame *m_game;

    // Fill the table with the players in the current game
    void fillTable();
};

#endif

