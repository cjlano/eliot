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

#include <QtGui/QStandardItemModel>
#include <QtGui/QKeyEvent>
#include <QtGui/QMessageBox>

#include "new_game.h"
#include "qtcommon.h"
#include "game_factory.h"
#include "game.h"
#include "training.h"
#include "freegame.h"
#include "duplicate.h"
#include "player.h"
#include "ai_percent.h"


const char * NewGame::kHUMAN = _("Human");
const char * NewGame::kAI = _("Computer");


NewGame::NewGame(QWidget *iParent)
    : QDialog(iParent)
{
    setupUi(this);

    // Initialize the model of the default players
    m_model = new QStandardItemModel(2, 2, this);
    m_model->setHeaderData(0, Qt::Horizontal, _q("Name"), Qt::DisplayRole);
    m_model->setHeaderData(1, Qt::Horizontal, _q("Type"), Qt::DisplayRole);
    m_model->setData(m_model->index(0, 0), _q("Player"));
    m_model->setData(m_model->index(0, 1), _q(kHUMAN));
    m_model->setData(m_model->index(1, 0), _q("Eliot"));
    m_model->setData(m_model->index(1, 1), _q(kAI));

    // Initialize the QTreeView with the model we just created
    treeViewPlayers->setModel(m_model);
    PlayersDelegate *delegate = new PlayersDelegate(this);
    treeViewPlayers->setItemDelegateForColumn(1, delegate);

    // Enable the Remove button only when there is a selection in the tree
    QObject::connect(treeViewPlayers->selectionModel(),
                     SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                     this,
                     SLOT(enableRemoveButton(const QItemSelection&, const QItemSelection&)));
    // Enable the Ok button only when if there are enough players for the
    // current mode
    QObject::connect(m_model, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                     this, SLOT(enableOkButton()));
    QObject::connect(m_model, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
                     this, SLOT(enableOkButton()));
    QObject::connect(comboBoxMode, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(enableOkButton()));

    // Install a custom event filter, to remove the selection when the
    // "Delete" key is pressed
    PlayersEventFilter *filter = new PlayersEventFilter(this);
    treeViewPlayers->installEventFilter(filter);
    QObject::connect(filter, SIGNAL(deletePressed()),
                     this, SLOT(on_pushButtonRemove_clicked()));
}


Game * NewGame::createGame(const Dictionary &iDic) const
{
    // Create the game
    Game *game = NULL;
    if (comboBoxMode->currentText() == _q("Training"))
        game = GameFactory::Instance()->createTraining(iDic);
    else if (comboBoxMode->currentText() == _q("Free game"))
        game = GameFactory::Instance()->createFreeGame(iDic);
    else
        game = GameFactory::Instance()->createDuplicate(iDic);

    // Add the players
    if (comboBoxMode->currentText() != _q("Training"))
    {
        set<QString> allNames;
        for (int num = 0; num < m_model->rowCount(); ++num)
        {
            QString name = m_model->data(m_model->index(num, 0)).toString();
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

            QString type = m_model->data(m_model->index(num, 1)).toString();
            Player *player;
            if (type == _q(kHUMAN))
                player = new HumanPlayer;
            else
                player = new AIPercent(1);
            player->setName(qtw(name));
            game->addPlayer(player);
        }
    }
    else
    {
        const_cast<Player *>(&game->getPlayer(0))->setName(qtw(_q("Training")));
    }

    // Joker game?
    if (checkBoxJoker->isChecked())
        game->setVariant(Game::kJOKER);

    return game;
}


void NewGame::enableOkButton()
{
    // Enable the "Ok" button:
    // - always in training mode
    // - if there is at least one player in duplicate mode
    // - if there are at least 2 players in free game mode
    bool disable =
        (comboBoxMode->currentText() == _q("Duplicate") && m_model->rowCount() < 1) ||
        (comboBoxMode->currentText() == _q("Free game") && m_model->rowCount() < 2);
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!disable);
}


void NewGame::enableRemoveButton(const QItemSelection &iSelected,
                                 const QItemSelection &)
{
    // Enable the "Remove" button iff at least one line in the tree view
    // is selected
    pushButtonRemove->setEnabled(!iSelected.indexes().empty());
}


void NewGame::on_comboBoxMode_activated(int iIndex)
{
    groupBoxPlayers->setEnabled(iIndex != 2);
}


void NewGame::on_pushButtonAdd_clicked()
{
    // Add a new row
    int rowNum = m_model->rowCount();
    bool res = m_model->insertRow(rowNum);
    if (!res)
        return;

    // Change the contents of the row
    m_model->setData(m_model->index(rowNum, 0), lineEditName->displayText());
    m_model->setData(m_model->index(rowNum, 1), comboBoxType->currentText());
}


void NewGame::on_pushButtonRemove_clicked()
{
    QModelIndexList indexList = treeViewPlayers->selectionModel()->selectedIndexes();
    if (indexList.empty())
        return;
    m_model->removeRow(indexList.front().row());
}



PlayersDelegate::PlayersDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}


QWidget *PlayersDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &,
                                       const QModelIndex &) const
{
    QComboBox *editor = new QComboBox(parent);
    editor->addItem(_q(NewGame::kHUMAN));
    editor->addItem(_q(NewGame::kAI));
    return editor;
}


void PlayersDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QComboBox *combo = static_cast<QComboBox*>(editor);
    QString text = index.model()->data(index, Qt::DisplayRole).toString();
    combo->setCurrentIndex(combo->findText(text));
}


void PlayersDelegate::setModelData(QWidget *editor,
                                   QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QComboBox *combo = static_cast<QComboBox*>(editor);
    model->setData(index, combo->currentText());
}


void PlayersDelegate::updateEditorGeometry(QWidget *editor,
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

