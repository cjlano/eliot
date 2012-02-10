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

#include <QtGui/QKeyEvent>
#include <QtGui/QComboBox>
#include <QtGui/QSpinBox>
#include <QtCore/QSettings>

#include "new_game.h"
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

    // Initialize the model of the default players
    addRow(_q("Player %1").arg(1), _q(kHUMAN), "");
    addRow(_q("Eliot"), _q(kAI), "100");

    // Change the default AI level
    refresh();

    PlayersTypeDelegate *typeDelegate = new PlayersTypeDelegate(this);
    tablePlayers->setItemDelegateForColumn(1, typeDelegate);
    PlayersLevelDelegate *levelDelegate = new PlayersLevelDelegate(this);
    tablePlayers->setItemDelegateForColumn(2, levelDelegate);
    // Improve the header
    QHeaderView *header = tablePlayers->horizontalHeader();
    header->setDefaultAlignment(Qt::AlignLeft);
    header->resizeSection(0, 200);
    header->resizeSection(1, 100);
    header->resizeSection(2, 50);

    // Enable the Level spinbox only when the player is a computer
    QObject::connect(comboBoxType, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(enableLevelSpinBox(int)));
    // Enable the Remove button only when there is a selection in the tree
    QObject::connect(tablePlayers, SIGNAL(itemSelectionChanged()),
                     this, SLOT(enableRemoveButton()));
    // Enable the Ok button only if there are enough players for the
    // current mode
    QObject::connect(tablePlayers->model(),
                     SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                     this, SLOT(enableOkButton()));
    QObject::connect(tablePlayers->model(),
                     SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
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

    // Install a custom event filter, to remove the selection when the
    // "Delete" key is pressed
    PlayersEventFilter *filter = new PlayersEventFilter(this);
    tablePlayers->installEventFilter(filter);
    QObject::connect(filter, SIGNAL(deletePressed()),
                     this, SLOT(on_pushButtonRemove_clicked()));
}


void NewGame::refresh()
{
    // Retrieve the default computer level
    QSettings qs;
    int defLevel = qs.value(PrefsDialog::kINTF_DEFAULT_AI_LEVEL, 100).toInt();
    // Ensure a valid range
    if (defLevel < 0)
        defLevel = 0;
    if (defLevel > 100)
        defLevel = 100;

    // Update the level of the "Eliot" player only
    for (int num = 0; num < tablePlayers->rowCount(); ++num)
    {
        QString name = tablePlayers->item(num, 0)->text();
        QString type = tablePlayers->item(num, 1)->text();
        if (name == _q("Eliot") && type == _q(kAI))
        {
            tablePlayers->item(num, 2)->setText(QString("%1").arg(defLevel));
        }
    }
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
        set<QString> allNames;
        for (int num = 0; num < tablePlayers->rowCount(); ++num)
        {
            QString name = tablePlayers->item(num, 0)->text();
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

            QString type = tablePlayers->item(num, 1)->text();
            Player *player;
            if (type == _q(kHUMAN))
                player = new HumanPlayer;
            else
            {
                double level = tablePlayers->item(num, 2)->text().toInt();
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
        (radioButtonDuplicate->isChecked() && tablePlayers->rowCount() < 1) ||
        (radioButtonFreeGame->isChecked() && tablePlayers->rowCount() < 2);
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!disable);
}


void NewGame::enableRemoveButton()
{
    // Enable the "Remove" button iff at least one line in the table
    // is selected
    pushButtonRemove->setEnabled(!tablePlayers->selectedItems().isEmpty());
}


void NewGame::enablePlayers(bool checked)
{
    // Testing the "checked" variable prevents from doing the work twice
    if (checked)
    {
        groupBoxPlayers->setEnabled(!radioButtonTraining->isChecked());
    }
}


void NewGame::addRow(QString iName, QString iType, QString iLevel)
{
    const int row = tablePlayers->rowCount();
    tablePlayers->setRowCount(row + 1);
    tablePlayers->setRowHeight(row, 24);
    tablePlayers->setItem(row, 0, new QTableWidgetItem(iName));
    tablePlayers->setItem(row, 1, new QTableWidgetItem(iType));
    tablePlayers->setItem(row, 2, new QTableWidgetItem(iLevel));
}


void NewGame::on_pushButtonAdd_clicked()
{
    // Add a new row
    addRow(lineEditName->displayText(),
           comboBoxType->currentText(),
           spinBoxLevel->isEnabled() ? QString("%1").arg(spinBoxLevel->value()) : "");

    // Increment the player ID
    static int currPlayer = 2;
    if (lineEditName->displayText() == _q("Player %1").arg(currPlayer))
    {
        ++currPlayer;
        lineEditName->setText(_q("Player %1").arg(currPlayer));
    }
}


void NewGame::on_pushButtonRemove_clicked()
{
    QList<QTableWidgetItem *> selectedItems = tablePlayers->selectedItems();
    if (!selectedItems.isEmpty())
        tablePlayers->removeRow(selectedItems.first()->row());
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


PlayersTypeDelegate::PlayersTypeDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}


QWidget *PlayersTypeDelegate::createEditor(QWidget *parent,
                                           const QStyleOptionViewItem &,
                                           const QModelIndex &) const
{
    QComboBox *editor = new QComboBox(parent);
    editor->addItem(_q(NewGame::kHUMAN));
    editor->addItem(_q(NewGame::kAI));
    return editor;
}


void PlayersTypeDelegate::setEditorData(QWidget *editor,
                                        const QModelIndex &index) const
{
    QComboBox *combo = static_cast<QComboBox*>(editor);
    QString text = index.model()->data(index, Qt::DisplayRole).toString();
    combo->setCurrentIndex(combo->findText(text));
}


void PlayersTypeDelegate::setModelData(QWidget *editor,
                                       QAbstractItemModel *model,
                                       const QModelIndex &index) const
{
    QComboBox *combo = static_cast<QComboBox*>(editor);
    model->setData(index, combo->currentText());
    // Adapt the level to the chosen type of player
    QModelIndex levelIndex = model->index(index.row(), 2);
    if (combo->currentText() == _q(NewGame::kHUMAN))
        model->setData(levelIndex, QVariant());
    else
        model->setData(levelIndex, 100);
}


void PlayersTypeDelegate::updateEditorGeometry(QWidget *editor,
                                               const QStyleOptionViewItem &option,
                                               const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}



PlayersLevelDelegate::PlayersLevelDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}


QWidget *PlayersLevelDelegate::createEditor(QWidget *parent,
                                            const QStyleOptionViewItem &,
                                            const QModelIndex &index) const
{
    // Allow changing the level only for computer players, i.e.
    // if there is a level defined
    if (index.model()->data(index, Qt::DisplayRole).isNull())
        return NULL;
    QSpinBox *editor = new QSpinBox(parent);
    editor->setMinimum(0);
    editor->setMaximum(100);
    return editor;
}


void PlayersLevelDelegate::setEditorData(QWidget *editor,
                                         const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    int value = index.model()->data(index, Qt::DisplayRole).toInt();
    spinBox->setValue(value);
}


void PlayersLevelDelegate::setModelData(QWidget *editor,
                                        QAbstractItemModel *model,
                                        const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    model->setData(index, spinBox->value());
}


void PlayersLevelDelegate::updateEditorGeometry(QWidget *editor,
                                                const QStyleOptionViewItem &option,
                                                const QModelIndex &) const
{
    editor->setGeometry(option.rect);
}



    PlayersEventFilter::PlayersEventFilter(QObject *parent)
: QObject(parent)
{
}


bool PlayersEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    // If the Delete key is pressed, remove the selected line, if any
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Delete)
        {
            emit deletePressed();
            return true;
        }
    }

    // Standard event processing
    return QObject::eventFilter(obj, event);
}

