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

#include <QtCore/QSettings>

#include "new_game.h"
#include "players_table_helper.h"
#include "qtcommon.h"
#include "prefs_dialog.h"
#include "game_factory.h"
#include "game.h"
#include "public_game.h"
#include "training.h"
#include "freegame.h"
#include "duplicate.h"
#include "player.h"
#include "ai_percent.h"


INIT_LOGGER(qt, NewGame);

const char * NewGame::kHUMAN = _("Human");
const char * NewGame::kAI = _("Computer");


NewGame::NewGame(QWidget *iParent)
    : QDialog(iParent)
{
    setupUi(this);
    lineEditName->setText(_q("Player %1").arg(2));

    checkBoxJoker->setToolTip(_q(
            "In a joker game, each rack contains a joker.\n"
            "When a word containing the joker is played on the grid, the joker is then replaced\n"
            "with the corresponding letter from the bag, and the joker stays in the rack.\n"
            "When the corresponding letter is not present in the bag, the joker is placed on the board.\n"
            "This variant, particularly interesting in Duplicate mode, is good to train using the joker."));
    checkBoxExplosive->setToolTip(_q(
            "An explosive game is a bit like a joker game, except that when the computer chooses the rack\n"
            "(containing a joker), it performs a search and finds the best word possible with the rack.\n"
            "Then, if possible, it replaces the joker in the rack with the letter allowing to play this best word.\n"
            "This variant, unlike the joker game, allows playing with a normal-looking rack, but it usually gives\n"
            "much higher scores than in a normal game."));
    checkBox7Among8->setToolTip(_q(
            "With this variant, the rack contains 8 letters instead of 7,\n"
            "but at most 7 can be played at the same time.\n"
            "This allows for more combinations during the game, and thus higher scores."));

    m_helper = new PlayersTableHelper(this, tablePlayers, NULL, pushButtonRemove);

    // Retrieve the default computer level
    QSettings qs;
    int defLevel = qs.value(PrefsDialog::kINTF_DEFAULT_AI_LEVEL, 100).toInt();
    // Ensure a valid range
    if (defLevel < 0)
        defLevel = 0;
    if (defLevel > 100)
        defLevel = 100;

    // Initialize the model of the default players
    m_helper->addRow(_q("Player %1").arg(1), _q(kHUMAN), "");
    m_helper->addRow(_q("Eliot"), _q(kAI), QString("%1").arg(defLevel));

    // Enable the Level spinbox only when the player is a computer
    QObject::connect(comboBoxType, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(enableLevelSpinBox(int)));
    // Enable the Ok button only if there are enough players for the
    // current mode
    QObject::connect(m_helper, SIGNAL(rowCountChanged()),
                     this, SLOT(enableOkButton()));
    QObject::connect(radioButtonDuplicate, SIGNAL(toggled(bool)),
                     this, SLOT(enableOkButton()));
    QObject::connect(radioButtonFreeGame, SIGNAL(toggled(bool)),
                     this, SLOT(enableOkButton()));
    QObject::connect(radioButtonTraining, SIGNAL(toggled(bool)),
                     this, SLOT(enableOkButton()));

    QObject::connect(radioButtonDuplicate, SIGNAL(toggled(bool)),
                     this, SLOT(enablePlayers(bool)));
    QObject::connect(radioButtonFreeGame, SIGNAL(toggled(bool)),
                     this, SLOT(enablePlayers(bool)));
    QObject::connect(radioButtonTraining, SIGNAL(toggled(bool)),
                     this, SLOT(enablePlayers(bool)));
}


PublicGame * NewGame::createGame(const Dictionary &iDic) const
{
    // Game parameters
    GameParams params(iDic);
    if (radioButtonTraining->isChecked())
        params.setMode(GameParams::kTRAINING);
    else if (radioButtonFreeGame->isChecked())
        params.setMode(GameParams::kFREEGAME);
    else
        params.setMode(GameParams::kDUPLICATE);

    if (checkBoxJoker->isChecked())
        params.addVariant(GameParams::kJOKER);
    if (checkBoxExplosive->isChecked())
        params.addVariant(GameParams::kEXPLOSIVE);
    if (checkBox7Among8->isChecked())
        params.addVariant(GameParams::k7AMONG8);

    // Create the game
    Game *tmpGame = GameFactory::Instance()->createGame(params);
    PublicGame *game = new PublicGame(*tmpGame);

    // Add the players
    if (!radioButtonTraining->isChecked())
    {
        const QList<PlayersTableHelper::PlayerDef> &players = m_helper->getPlayers(false);
        set<QString> allNames;
        for (int num = 0; num < players.size(); ++num)
        {
            QString name = players.at(num).name;
            if (name == "")
                name = _q("Player %1").arg(num + 1);
            // Ensure unicity of the players names
            if (allNames.find(name) != allNames.end())
            {
                int n = 2;
                while (allNames.find(name + QString(" (%1)").arg(n)) != allNames.end())
                {
                    ++n;
                }
                name += QString(" (%1)").arg(n);
            }
            allNames.insert(name);

            QString type = players.at(num).type;
            Player *player;
            if (type == _q(kHUMAN))
                player = new HumanPlayer;
            else
            {
                double level = players.at(num).level;
                player = new AIPercent(level / 100.);
            }
            player->setName(wfq(name));
            game->addPlayer(player);
        }
    }
    else
    {
        game->addPlayer(new HumanPlayer);
    }

    return game;
}


void NewGame::enableLevelSpinBox(int index)
{
    spinBoxLevel->setEnabled(index == 1);
}


void NewGame::enableOkButton()
{
    // Enable the "Ok" button:
    // - always in training mode
    // - if there is at least one player in duplicate mode
    // - if there are at least 2 players in free game mode
    bool disable =
        (radioButtonDuplicate->isChecked() && m_helper->getRowCount() < 1) ||
        (radioButtonFreeGame->isChecked() && m_helper->getRowCount() < 2);
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!disable);
}


void NewGame::enablePlayers(bool checked)
{
    // Testing the "checked" variable prevents from doing the work twice
    if (checked)
    {
        groupBoxPlayers->setEnabled(!radioButtonTraining->isChecked());
    }
}


void NewGame::on_pushButtonAdd_clicked()
{
    // Add a new row
    m_helper->addRow(lineEditName->displayText(),
                     comboBoxType->currentText(),
                     spinBoxLevel->isEnabled() ?
                        QString("%1").arg(spinBoxLevel->value()) : "");

    // Increment the player ID
    static int currPlayer = 2;
    if (lineEditName->displayText() == _q("Player %1").arg(currPlayer))
    {
        ++currPlayer;
        lineEditName->setText(_q("Player %1").arg(currPlayer));
    }
}


void NewGame::on_checkBoxJoker_stateChanged(int newState)
{
    if (newState == Qt::Checked)
        checkBoxExplosive->setChecked(false);
}


void NewGame::on_checkBoxExplosive_stateChanged(int newState)
{
    if (newState == Qt::Checked)
        checkBoxJoker->setChecked(false);
}

