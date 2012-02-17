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

#include "config.h"

#include <QtGui/QTableWidget>
#include <QtCore/QSettings>

#include "fav_players_dialog.h"
#include "players_table_helper.h"


INIT_LOGGER(qt, FavPlayersDialog);


FavPlayersDialog::FavPlayersDialog(QWidget *iParent)
    : QDialog(iParent)
{
    setupUi(this);

    m_helper = new PlayersTableHelper(this, tablePlayers, NULL, buttonRemove);
    m_helper->addPopupAction(m_helper->getRemoveAction());

    QObject::connect(buttonAdd, SIGNAL(clicked()),
                     this, SLOT(addRow()));

    m_helper->addPlayers(m_helper->getFavPlayers());
}


void FavPlayersDialog::addRow()
{
    m_helper->addRow(_q("New player"), _q(PlayersTableHelper::kHUMAN), "");

    // Give focus to the newly created cell containing the player name,
    // to allow fast edition
    tablePlayers->setFocus();
    tablePlayers->setCurrentCell(tablePlayers->rowCount() - 1, 0,
                                 QItemSelectionModel::ClearAndSelect |
                                 QItemSelectionModel::Current |
                                 QItemSelectionModel::Rows);
}


void FavPlayersDialog::accept()
{
    m_helper->saveFavPlayers(m_helper->getPlayers(false));

    QDialog::accept();
}

