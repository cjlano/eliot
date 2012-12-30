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

#include <QtGui/QCompleter>
#include <QtGui/QFileSystemModel>
#include <QtGui/QFileDialog>
#include <QtCore/QSettings>

#include "new_game.h"
#include "players_table_helper.h"
#include "qtcommon.h"
#include "prefs_dialog.h"
#include "game_factory.h"
#include "game.h"
#include "public_game.h"
#include "player.h"
#include "ai_percent.h"


INIT_LOGGER(qt, NewGame);

const char * NewGame::kHUMAN = _("Human");
const char * NewGame::kAI = _("Computer");


NewGame::NewGame(const Dictionary &iDic, QWidget *iParent)
    : QDialog(iParent), m_dic(iDic)
{
    setupUi(this);

    m_blackPalette = lineEditMaster->palette();
    m_redPalette = lineEditMaster->palette();
    m_redPalette.setColor(QPalette::Text, Qt::red);

    radioButtonDuplicate->setToolTip(_q(
            "In duplicate mode, all the players are always faced with the same board and the same rack,\n"
            "thus eliminating any \"luck\" (and tactics).\n"
            "Each player scores the points of the word (s)he found, but only\n"
            "the best move is played on the board.\n"
            "This mode allows an unlimited number of simultaneous players, and is therefore\n"
            "often used for official tournaments."));
    radioButtonFreeGame->setToolTip(_q(
            "This mode is the classical one, often played in family, where players play in turn,\n"
            "each with his own rack. Players are allowed to change letters, thus passing their turn.\n"
            "With only 2 players, some tactics can often be used, because the best move\n"
            "is not necessarily the one with the best score."));
    radioButtonTraining->setToolTip(_q(
            "In training mode, the player can set the rack freely and can see all the possible moves.\n"
            "There is no opponent, the goal is simply to make some progress."));
    radioButtonArbitration->setToolTip(_q(
            "The arbitration mode allows arbitrating a duplicate game, possibly with many players.\n"
            "The arbitrator can set the master move, and keep track of the players moves easily.\n"
            "This mode is ideal for arbitrating duplicate games in clubs or in tournaments."));
    radioButtonTopping->setToolTip(_q(
            "In topping mode, the goal is to find the top as quickly as possible. The player is allowed\n"
            "to try as many moves as possible until he finds the top, but there are penalties\n"
            "when the player takes too much time to find it (or doesn't fint it at all).\n"
            "This mode can be quite difficult, and is mostly intended for experienced players."));

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

    m_helper = new PlayersTableHelper(this, tablePlayers, pushButtonAdd);
    m_helper->addPopupRemoveAction();
    QAction *addToFavAction = new QAction(_q("Mark the selected player(s) as favorites"), this);
    addToFavAction->setStatusTip(_q("Add the selected player(s) to the list of favorite players"));
    QObject::connect(addToFavAction, SIGNAL(triggered()),
                     this, SLOT(addSelectedToFav()));
    m_helper->addPopupAction(addToFavAction);
    m_helper->setUpDown(buttonUp, buttonDown);

    // Initialize the model of the default players
    QList<PlayerDef> fav = PlayersTableHelper::getFavPlayers();
    Q_FOREACH(const PlayerDef &def, fav)
    {
        if (def.isDefault)
            m_helper->addPlayer(def);
    }

    // Default of the default :)
    if (m_helper->getRowCount() == 0)
    {
        m_helper->addPlayer(PlayerDef(_q("Player %1").arg(1), _q(kHUMAN), "", false));
        m_helper->addPlayer(PlayerDef(_q("Eliot"), _q(kAI), "100", false));
    }

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
    QObject::connect(checkBoxUseMaster, SIGNAL(toggled(bool)),
                     this, SLOT(enableOkButton()));
    QObject::connect(lineEditMaster, SIGNAL(textChanged(QString)),
                     this, SLOT(enableOkButton()));

    QObject::connect(radioButtonDuplicate, SIGNAL(toggled(bool)),
                     this, SLOT(enablePlayers(bool)));
    QObject::connect(radioButtonFreeGame, SIGNAL(toggled(bool)),
                     this, SLOT(enablePlayers(bool)));
    QObject::connect(radioButtonTraining, SIGNAL(toggled(bool)),
                     this, SLOT(enablePlayers(bool)));
    QObject::connect(radioButtonArbitration, SIGNAL(toggled(bool)),
                     this, SLOT(enablePlayers(bool)));
    QObject::connect(radioButtonTopping, SIGNAL(toggled(bool)),
                     this, SLOT(enablePlayers(bool)));

    QObject::connect(radioButtonFreeGame, SIGNAL(toggled(bool)),
                     this, SLOT(enableMasterControls()));

    QObject::connect(checkBoxJoker, SIGNAL(stateChanged(int)),
                     this, SLOT(onJokerChecked(int)));
    QObject::connect(checkBoxExplosive, SIGNAL(stateChanged(int)),
                     this, SLOT(onExplosiveChecked(int)));

    // Master games
    QObject::connect(checkBoxUseMaster, SIGNAL(toggled(bool)),
                     widgetMasterControls, SLOT(setEnabled(bool)));
    QObject::connect(buttonBrowseMaster, SIGNAL(clicked()),
                     this, SLOT(browseMasterGame()));
    QObject::connect(lineEditMaster, SIGNAL(textChanged(QString)),
                     this, SLOT(validateMasterGame(QString)));

    QObject::connect(buttonAddFav, SIGNAL(clicked()),
                     this, SLOT(addFavoritePlayers()));

    // Auto-completion on the master game path
    QCompleter *completer = new QCompleter(this);
    QFileSystemModel *model = new QFileSystemModel(this);
    model->setRootPath(QDir::currentPath());
    completer->setModel(model);
    lineEditMaster->setCompleter(completer);
}


PublicGame * NewGame::createGame() const
{
    // Game parameters
    GameParams params(m_dic);
    if (radioButtonTraining->isChecked())
        params.setMode(GameParams::kTRAINING);
    else if (radioButtonFreeGame->isChecked())
        params.setMode(GameParams::kFREEGAME);
    else if (radioButtonDuplicate->isChecked())
        params.setMode(GameParams::kDUPLICATE);
    else if (radioButtonArbitration->isChecked())
        params.setMode(GameParams::kARBITRATION);
    else
        params.setMode(GameParams::kTOPPING);

    if (checkBoxJoker->isChecked())
        params.addVariant(GameParams::kJOKER);
    if (checkBoxExplosive->isChecked())
        params.addVariant(GameParams::kEXPLOSIVE);
    if (checkBox7Among8->isChecked())
        params.addVariant(GameParams::k7AMONG8);

    // Load the master game if needed
    Game *masterGame = NULL;
    if (checkBoxUseMaster->isChecked())
    {
        const QString &path = lineEditMaster->text();
        try
        {
            masterGame = GameFactory::Instance()->load(lfq(path), m_dic);
        }
        catch (const GameException &e)
        {
            // Should not happen, since we have already done validation.
            // But the user may have changed the file itself after the validation...
            emit notifyProblem("Error loading master game: " + qfl(e.what()));
            // Give up loading the game
            return NULL;
        }
    }

    // Create the game
    Game *tmpGame = GameFactory::Instance()->createGame(params, masterGame);
    PublicGame *game = new PublicGame(*tmpGame);

    // Add the players
    if (!radioButtonTraining->isChecked() &&
        !radioButtonTopping->isChecked())
    {
        const QList<PlayerDef> &players = m_helper->getPlayers(false);
        set<QString> allNames;
        for (int num = 0; num < players.size(); ++num)
        {
            QString name = players.at(num).name;
            if (name == "")
                name = _q("Player %1").arg(num + 1);
            // Ensure uniqueness of the players names
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
                double level = players.at(num).level.toInt();
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


void NewGame::enableOkButton()
{
    // Enable the "Ok" button:
    // - always in training mode or duplicate mode
    // - if there is at least one player in duplicate mode
    // - if there are at least 2 players in free game mode
    bool disable =
        (radioButtonDuplicate->isChecked() && m_helper->getRowCount() < 1) ||
        (radioButtonFreeGame->isChecked() && m_helper->getRowCount() < 2);
    // The "Ok" button is also disabled when a master game is used,
    // but no ivalid master game is specified
    disable = disable || (checkBoxUseMaster->isChecked() && !isMasterGameValid(lineEditMaster->text()));

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!disable);
}


void NewGame::enableMasterControls()
{
    // Do not allow master games in free game mode: it would
    // work only when loading another free game, with the same
    // number of players. And this is probably not useful anyway.
    groupBoxMaster->setEnabled(!radioButtonFreeGame->isChecked());
}


void NewGame::enablePlayers(bool checked)
{
    // Testing the "checked" variable prevents from doing the work twice
    if (checked)
    {
        groupBoxPlayers->setEnabled(!radioButtonTraining->isChecked() &&
                                    !radioButtonTopping->isChecked());
    }
}


void NewGame::addSelectedToFav()
{
    QList<PlayerDef> fav = PlayersTableHelper::getFavPlayers();
    const QList<PlayerDef> &selected = m_helper->getPlayers(true);
    Q_FOREACH(const PlayerDef &def, selected)
    {
        fav.push_back(def);
    }
    m_helper->saveFavPlayers(fav);
}


void NewGame::addFavoritePlayers()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle(_q("Select the players to add"));
    dialog->resize(400, 500);
    dialog->setLayout(new QVBoxLayout);
    QTableWidget *tableFav = new QTableWidget;
    dialog->layout()->addWidget(tableFav);
    QDialogButtonBox *buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    dialog->layout()->addWidget(buttonBox);
    connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

    PlayersTableHelper *helper = new PlayersTableHelper(dialog, tableFav);
    helper->addPlayers(PlayersTableHelper::getFavPlayers());

    if (dialog->exec() == QDialog::Accepted)
    {
        m_helper->addPlayers(helper->getPlayers(true));
    }
}


void NewGame::onJokerChecked(int newState)
{
    // The joker and explosive variants are incompatible
    if (newState == Qt::Checked)
        checkBoxExplosive->setChecked(false);
}


void NewGame::onExplosiveChecked(int newState)
{
    // The joker and explosive variants are incompatible
    if (newState == Qt::Checked)
        checkBoxJoker->setChecked(false);
}


void NewGame::browseMasterGame()
{
    QString fileName = QFileDialog::getOpenFileName(this, _q("Load a game"));
    if (fileName != "")
    {
        lineEditMaster->setText(fileName);
    }
}


void NewGame::validateMasterGame(QString iPath)
{
    // Validate the given path
    bool isValid = isMasterGameValid(iPath);

    // Update the path colour
    lineEditMaster->setPalette(isValid ? m_blackPalette : m_redPalette);
}


bool NewGame::isMasterGameValid(QString iPath) const
{
    bool isValid = false;
    try
    {
        Game *game = GameFactory::Instance()->load(lfq(iPath), m_dic);
        // Free games are not accepted as master games
        isValid = game->getParams().getMode() != GameParams::kFREEGAME;
        delete game;
    }
    catch (const GameException &e)
    {
        // Ignore the exception
    }
    return isValid;
}


