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
#include <QtGui/QPushButton>
#include <QtGui/QHeaderView>
#include <QtGui/QComboBox>
#include <QtGui/QSpinBox>
#include <QtGui/QKeyEvent>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtCore/QSettings>

#include "players_table_helper.h"
#include "custom_popup.h"
#include "qtcommon.h"


INIT_LOGGER(qt, PlayersTableHelper);


const char * PlayersTableHelper::kHUMAN = _("Human");
const char * PlayersTableHelper::kAI = _("Computer");

// Global function, used to put PlayerDef in a QSet
uint qHash(const PlayerDef &key)
{
    return qHash(key.name) ^ qHash(key.type) ^ qHash(key.level);
}


PlayerDef::PlayerDef(QString iName, QString iType, QString iLevel)
    : name(iName), type(iType), level(iLevel)
{
}

bool PlayerDef::operator==(const PlayerDef &iOther) const
{
    return name == iOther.name
        && type == iOther.type
        && level == iOther.level;
}


PlayersTableHelper::PlayersTableHelper(QObject *parent,
                                       QTableWidget *tablePlayers,
                                       QPushButton *addButton,
                                       QPushButton *removeButton)
    : QObject(parent), m_tablePlayers(tablePlayers),
    m_buttonAdd(addButton), m_buttonRemove(removeButton)
{
    // Initialize the table headers
    tablePlayers->setColumnCount(3);
    tablePlayers->setHorizontalHeaderItem(0, new QTableWidgetItem(_("Name")));
    tablePlayers->setHorizontalHeaderItem(1, new QTableWidgetItem(_("Type")));
    tablePlayers->setHorizontalHeaderItem(2, new QTableWidgetItem(_("Level")));
    QHeaderView *header = tablePlayers->horizontalHeader();
    header->setHighlightSections(false);
    header->setStretchLastSection(true);
    header->setDefaultAlignment(Qt::AlignLeft);
    header->resizeSection(0, 200);
    header->resizeSection(1, 100);
    header->resizeSection(2, 50);
    tablePlayers->verticalHeader()->setVisible(false);

    // Set other table properties
    tablePlayers->setAlternatingRowColors(true);
    tablePlayers->setSelectionBehavior(QAbstractItemView::SelectRows);
    tablePlayers->setShowGrid(false);

    if (m_buttonAdd)
    {
        QObject::connect(m_buttonAdd, SIGNAL(clicked()),
                         this, SLOT(addRow()));
    }
    if (m_buttonRemove)
    {
        QObject::connect(tablePlayers, SIGNAL(itemSelectionChanged()),
                         this, SLOT(enableRemoveButton()));
        QObject::connect(m_buttonRemove, SIGNAL(clicked()),
                         this, SLOT(removeSelectedRows()));
    }

    PlayersTypeDelegate *typeDelegate = new PlayersTypeDelegate(this);
    m_tablePlayers->setItemDelegateForColumn(1, typeDelegate);
    PlayersLevelDelegate *levelDelegate = new PlayersLevelDelegate(this);
    m_tablePlayers->setItemDelegateForColumn(2, levelDelegate);

    // Add a context menu for the results
    CustomPopup *popup = new CustomPopup(m_tablePlayers);
    QObject::connect(popup, SIGNAL(popupCreated(QMenu&, const QPoint&)),
                     this, SLOT(populateMenu(QMenu&, const QPoint&)));
}


void PlayersTableHelper::populateMenu(QMenu &iMenu, const QPoint &iPoint)
{
    const QModelIndex &index = m_tablePlayers->indexAt(iPoint);
    if (!index.isValid())
        return;

    Q_FOREACH(QAction *action, m_popupActions)
    {
        iMenu.addAction(action);
    }
}


void PlayersTableHelper::addPopupAction(QAction *iAction)
{
    m_popupActions.push_back(iAction);
}


void PlayersTableHelper::addPopupRemoveAction()
{
    QAction *removeAction = new QAction(_q("Remove selected player(s)"), this);
    removeAction->setStatusTip(_q("Remove the selected player(s) from the list"));
    removeAction->setShortcut(Qt::Key_Delete);
    QObject::connect(removeAction, SIGNAL(triggered()),
                     this, SLOT(removeSelectedRows()));
    addPopupAction(removeAction);

    // Install a custom event filter, to remove the selection when the
    // "Delete" key is pressed
    PlayersEventFilter *filter = new PlayersEventFilter(this);
    m_tablePlayers->installEventFilter(filter);
    QObject::connect(filter, SIGNAL(deletePressed()),
                     this, SLOT(removeSelectedRows()));
}


void PlayersTableHelper::enableRemoveButton()
{
    m_buttonRemove->setEnabled(!m_tablePlayers->selectedItems().isEmpty());
}


void PlayersTableHelper::removeSelectedRows()
{
    bool changed = false;
    QItemSelectionModel *selModel = m_tablePlayers->selectionModel();
    for (int i = m_tablePlayers->rowCount() - 1; i >= 0; --i)
    {
        if (selModel->isRowSelected(i, QModelIndex()))
        {
            changed = true;
            m_tablePlayers->removeRow(i);
        }
    }
    if (changed)
        emit rowCountChanged();
}


void PlayersTableHelper::addRow()
{
    addRow(PlayerDef(_q("New player"), _q(kHUMAN), ""));
    // Give focus to the newly created cell containing the player name,
    // to allow fast edition
    m_tablePlayers->setFocus();
    m_tablePlayers->setCurrentCell(m_tablePlayers->rowCount() - 1, 0,
                                   QItemSelectionModel::ClearAndSelect |
                                   QItemSelectionModel::Current |
                                   QItemSelectionModel::Rows);
}


QList<PlayerDef> PlayersTableHelper::getPlayers(bool onlySelected) const
{
    QList<PlayerDef> playersList;
    QItemSelectionModel *selModel = m_tablePlayers->selectionModel();
    for (int i = 0; i < m_tablePlayers->rowCount(); ++i)
    {
        if (onlySelected && !selModel->isRowSelected(i, QModelIndex()))
            continue;
        PlayerDef playerDef(m_tablePlayers->item(i, 0)->text(),
                            m_tablePlayers->item(i, 1)->text(),
                            m_tablePlayers->item(i, 2)->text());
        playersList.push_back(playerDef);
    }
    return playersList;
}


int PlayersTableHelper::getRowCount() const
{
    return m_tablePlayers->rowCount();
}


void PlayersTableHelper::addPlayers(const QList<PlayerDef> &iList)
{
    // Only add players which are not already there
    QSet<PlayerDef> tmpSet = getPlayers(false).toSet();
    Q_FOREACH(const PlayerDef &player, iList)
    {
        if (!tmpSet.contains(player))
        {
            addRow(player);
            tmpSet.insert(player);
        }
    }
}


void PlayersTableHelper::addPlayer(const PlayerDef &iPlayer, bool selectAndEdit)
{
    QList<PlayerDef> tmpList;
    tmpList.push_back(iPlayer);
    addPlayers(tmpList);

    if (selectAndEdit)
    {
        int row = m_tablePlayers->rowCount() - 1;
        // Give focus to the newly created cell containing the player name
        m_tablePlayers->setCurrentCell(row, 0,
                                       QItemSelectionModel::ClearAndSelect |
                                       QItemSelectionModel::Current |
                                       QItemSelectionModel::Rows);
        // Edit the name
        m_tablePlayers->editItem(m_tablePlayers->item(row, 0));
    }
}


void PlayersTableHelper::addRow(const PlayerDef &iDef)
{
    const int row = m_tablePlayers->rowCount();
    m_tablePlayers->setRowCount(row + 1);
    m_tablePlayers->setRowHeight(row, 24);
    m_tablePlayers->setItem(row, 0, new QTableWidgetItem(iDef.name));
    m_tablePlayers->setItem(row, 1, new QTableWidgetItem(iDef.type));
    m_tablePlayers->setItem(row, 2, new QTableWidgetItem(iDef.level));
    emit rowCountChanged();
}


QList<PlayerDef> PlayersTableHelper::getFavPlayers()
{
    QList<PlayerDef> playersList;
    QSettings qs;
    int size = qs.beginReadArray("FavPlayers");
    for (int i = 0; i < size; ++i)
    {
        qs.setArrayIndex(i);
        PlayerDef playerDef(qs.value("name").toString(),
                            qs.value("type").toString(),
                            qs.value("level").toString());
        playersList.push_back(playerDef);
    }
    qs.endArray();
    return playersList;
}


void PlayersTableHelper::saveFavPlayers(const QList<PlayerDef> &iFavPlayers)
{
    QSettings qs;
    qs.beginWriteArray("FavPlayers");
    for (int i = 0; i < iFavPlayers.size(); ++i)
    {
        qs.setArrayIndex(i);
        qs.setValue("name", iFavPlayers.at(i).name);
        qs.setValue("type", iFavPlayers.at(i).type);
        qs.setValue("level", iFavPlayers.at(i).level);
    }
    qs.endArray();
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
    editor->addItem(_q(PlayersTableHelper::kHUMAN));
    editor->addItem(_q(PlayersTableHelper::kAI));
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
    if (combo->currentText() == _q(PlayersTableHelper::kHUMAN))
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
    const QAbstractItemModel *model = index.model();
    QString type = model->data(model->index(index.row(), 1), Qt::DisplayRole).toString();
    if (type != _q(PlayersTableHelper::kAI))
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

