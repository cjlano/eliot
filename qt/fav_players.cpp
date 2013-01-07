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

#include <fstream>

#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QTableWidget>
#include <QtGui/QFileDialog>

#include "fav_players.h"
#include "players_table_helper.h"
#include "qtcommon.h"

#include "csv_helper.h"
#include "base_exception.h"


using namespace std;

INIT_LOGGER(qt, FavPlayersDialog);


FavPlayersDialog::FavPlayersDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(_q("Favorite players"));
    resize(480, 550);
    setMinimumSize(400, 200);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    QLabel *label = new QLabel(_q("The favorite players listed below can be "
                                  "used in the \"New game\" dialog, to add "
                                  "players quickly. Those marked as \"Default\" "
                                  "will appear there directly (useful if you "
                                  "often play with the same players)."));
    label->setWordWrap(true);
    vLayout->addWidget(label);
    QLabel *label2 = new QLabel(_q("To add or remove a player, use the buttons "
                                   "at the bottom. You can edit the existing "
                                   "players directly in the table, by "
                                   "double-clicking on them."));
    label2->setWordWrap(true);
    vLayout->addWidget(label2);
    QTableWidget *tableFav = new QTableWidget;
    vLayout->addWidget(tableFav);

    QHBoxLayout *hLayout = new QHBoxLayout;
    vLayout->addLayout(hLayout);

    QPushButton *buttonAdd = new QPushButton(_q("Add player"));
    hLayout->addWidget(buttonAdd);
    QPushButton *buttonRemove = new QPushButton(_q("Remove player"));
    hLayout->addWidget(buttonRemove);
    hLayout->addStretch();
    QPushButton *buttonMoveUp = new QPushButton(QIcon(":/images/go-up.png"), "");
    buttonMoveUp->setToolTip(_q("Move selection upwards"));
    hLayout->addWidget(buttonMoveUp);
    QPushButton *buttonMoveDown = new QPushButton(QIcon(":/images/go-down.png"), "");
    buttonMoveDown->setToolTip(_q("Move selection downwards"));
    hLayout->addWidget(buttonMoveDown);
    hLayout->addStretch();
    QPushButton *buttonImport = new QPushButton(_q("CSV Import..."));
    hLayout->addWidget(buttonImport);
    QObject::connect(buttonImport, SIGNAL(clicked()),
                     this, SLOT(importPlayers()));
    QPushButton *buttonExport = new QPushButton(_q("CSV Export..."));
    hLayout->addWidget(buttonExport);
    QObject::connect(buttonExport, SIGNAL(clicked()),
                     this, SLOT(exportPlayers()));

    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    vLayout->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    m_helper = new PlayersTableHelper(this, tableFav, buttonAdd, buttonRemove, true);
    m_helper->addPopupRemoveAction();
    m_helper->setUpDown(buttonMoveUp, buttonMoveDown);
    m_helper->addPlayers(m_helper->getFavPlayers());
}


void FavPlayersDialog::accept()
{
    // Save the favourite players in case of validation
    m_helper->saveFavPlayers(m_helper->getPlayers(false));

    QDialog::accept();
}


void FavPlayersDialog::importPlayers()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            _q("Choose a CSV file containing favorite players to import"),
            _q("fav_players.csv"),
            QString("%1;;%2").arg(_q("CSV files (*.csv)")).arg("All files (*)"));
    if (fileName.isEmpty())
        return;

    ifstream file(lfq(fileName).c_str());
    if (!file.is_open())
    {
        emit notifyProblem(_q("Cannot open file '%1' for reading").arg(fileName));
        return;
    }

    try
    {
        const vector<CsvHelper::DataRow> &csvData = CsvHelper::readStream(file);
        file.close();

        QList<PlayerDef> players;
        Q_FOREACH(const CsvHelper::DataRow &row, csvData)
        {
            if (row.size() < 4)
                throw BaseException(_("Invalid file (not enough values)"));
            // The player is not a default one if the corresponding flag is incorrect
            PlayerDef def(qfl(row[0]), qfl(row[1]), qfl(row[2]), row[3] == "1");
            players.push_back(def);
        }
        m_helper->addPlayers(players);
    }
    catch (const std::exception &e)
    {
        emit notifyProblem(_q("Cannot import favorite players: %1")
                           .arg(e.what()));
    }
}


void FavPlayersDialog::exportPlayers()
{
    QString fileName = QFileDialog::getSaveFileName(this,
            _q("Choose a CSV file to save the favorite players"),
            _q("fav_players.csv"),
            QString("%1;;%2").arg(_q("CSV files (*.csv)")).arg("All files (*)"));
    if (fileName.isEmpty())
        return;

    ofstream file(lfq(fileName).c_str());
    if (!file.is_open())
    {
        emit notifyProblem(_q("Cannot open file '%1' for writing").arg(fileName));
        return;
    }

    try
    {
        const QList<PlayerDef> &players = m_helper->getPlayers(false);
        vector<CsvHelper::DataRow> csvData;
        Q_FOREACH(const PlayerDef &def, players)
        {
            CsvHelper::DataRow dataRow;
            dataRow.push_back(lfq(def.name));
            dataRow.push_back(lfq(def.type));
            dataRow.push_back(lfq(def.level));
            dataRow.push_back(def.isDefault ? "1" : "0");
            csvData.push_back(dataRow);
        }

        CsvHelper::writeStream(file, csvData);
        file.close();
    }
    catch (const std::exception &e)
    {
        emit notifyProblem(_q("Cannot export favorite players: %1")
                           .arg(e.what()));
    }
}

