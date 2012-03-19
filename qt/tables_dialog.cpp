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

#include "tables_dialog.h"
#include "public_game.h"
#include "qtcommon.h"

#include "player.h"


INIT_LOGGER(qt, TablesDialog);


TablesDialog::TablesDialog(QWidget *parent, PublicGame &iGame)
    : QDialog(parent), m_game(&iGame)
{
    setupUi(this);

    fillTable();
}


void TablesDialog::fillTable()
{
    unsigned nbPlayers = m_game->getNbPlayers();
    tablePlayers->setRowCount(nbPlayers);

    for (unsigned i = 0; i < nbPlayers; ++i)
    {
        const Player &player = m_game->getPlayer(i);

        // Display the player data
        tablePlayers->setRowHeight(i, 24);
        tablePlayers->setItem(i, 0, new QTableWidgetItem(qfw(player.getName())));
        QTableWidgetItem *item = new QTableWidgetItem(qfw(player.getName()));
        item->setData(Qt::DisplayRole, QVariant((int)player.getTableNb()));
        // Also save the player ID as user data (hidden)
        item->setData(Qt::UserRole, QVariant(player.getId()));
        tablePlayers->setItem(i, 1, item);
    }
}


void TablesDialog::accept()
{
    // Check table numbers uniqueness
    QSet<unsigned> tableNbSet;
    for (int i = 0; i < tablePlayers->rowCount(); ++i)
    {
        unsigned tableNb = tablePlayers->item(i, 1)->data(Qt::DisplayRole).toUInt();
        if (tableNbSet.contains(tableNb))
        {
            emit notifyProblem(_q("The table numbers must be unique, but "
                                  "\"%1\" appears several times.").arg(tableNb));
            return;
        }
        tableNbSet.insert(tableNb);
    }

    // Save data
    for (int i = 0; i < tablePlayers->rowCount(); ++i)
    {
        // Retrieve the player id
        unsigned id = tablePlayers->item(i, 1)->data(Qt::UserRole).toUInt();

        // Update the player name
        QString name = tablePlayers->item(i, 0)->data(Qt::DisplayRole).toString();
        m_game->setPlayerName(id, wfq(name));
        // Update the table number
        unsigned tableNb = tablePlayers->item(i, 1)->data(Qt::DisplayRole).toUInt();
        m_game->setPlayerTableNb(id, tableNb);
    }

    QDialog::accept();
}

