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
#include "misc_helpers.h"
#include "qtcommon.h"
#include "debug.h"


INIT_LOGGER(qt, PlayersTableHelper);


const char * PlayersTableHelper::kHUMAN = _("Human");
const char * PlayersTableHelper::kAI = _("Computer");

// Global function, used to put PlayerDef in a QSet
uint qHash(const PlayerDef &key)
{
    return qHash(key.name) ^ qHash(key.type) ^ qHash(key.level);
}


PlayerDef::PlayerDef(QString iName, QString iType, QString iLevel, bool iIsDefault)
    : name(iName), type(iType), level(iLevel), isDefault(iIsDefault)
{
}

bool PlayerDef::operator==(const PlayerDef &iOther) const
{
    // Ignore the "isDefault" flag
    return name == iOther.name
        && type == iOther.type
        && level == iOther.level;
}


PlayersTableHelper::PlayersTableHelper(QObject *parent,
                                       QTableWidget *tablePlayers,
                                       QPushButton *addButton,
                                       QPushButton *removeButton,
                                       bool showDefaultColumn)
    : QObject(parent), m_tablePlayers(tablePlayers),
    m_buttonAdd(addButton), m_buttonRemove(removeButton),
    m_showDefaultColumn(showDefaultColumn)
{
    // Initialize the table headers
    tablePlayers->setColumnCount(m_showDefaultColumn ? 4 : 3);
    tablePlayers->setHorizontalHeaderItem(0, new QTableWidgetItem(_q("Name")));
    tablePlayers->setHorizontalHeaderItem(1, new QTableWidgetItem(_q("Type")));
    tablePlayers->setHorizontalHeaderItem(2, new QTableWidgetItem(_q("Level")));
    tablePlayers->setHorizontalHeaderItem(3, new QTableWidgetItem(_q("Default")));
    QHeaderView *header = tablePlayers->horizontalHeader();
    header->setHighlightSections(false);
    header->setStretchLastSection(true);
    header->setDefaultAlignment(Qt::AlignLeft);
    header->resizeSection(0, 200);
    header->resizeSection(1, 100);
    header->resizeSection(2, 50);
    header->resizeSection(3, 70);
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
    KeyEventFilter *filter = new KeyEventFilter(this, Qt::Key_Delete);
    m_tablePlayers->installEventFilter(filter);
    QObject::connect(filter, SIGNAL(keyPressed(int, int)),
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
    addPlayer(PlayerDef(_q("New player"), _q(kHUMAN), "", false),
              true, true);
}


QList<PlayerDef> PlayersTableHelper::getPlayers(bool onlySelected) const
{
    QList<PlayerDef> playersList;
    QItemSelectionModel *selModel = m_tablePlayers->selectionModel();
    for (int i = 0; i < m_tablePlayers->rowCount(); ++i)
    {
        if (onlySelected && !selModel->isRowSelected(i, QModelIndex()))
            continue;
        const QVariant & data = m_tablePlayers->model()->data(m_tablePlayers->model()->index(i, 3));
        PlayerDef playerDef(m_tablePlayers->item(i, 0)->text(),
                            m_tablePlayers->item(i, 1)->text(),
                            m_tablePlayers->item(i, 2)->text(),
                            data.toBool());
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
    Q_FOREACH(const PlayerDef &player, iList)
    {
        addPlayer(player, false, false);
    }
}


void PlayersTableHelper::addPlayer(const PlayerDef &iPlayer,
                                   bool selectAndEdit,
                                   bool renameIfDuplicate)
{
    QSet<PlayerDef> tmpSet = getPlayers(false).toSet();
    PlayerDef def = normalize(iPlayer);
    // Generate a unique name if needed
    if (renameIfDuplicate)
    {
        int i = 1;
        while (tmpSet.contains(def))
        {
            def.name = QString("%1 (%2)").arg(iPlayer.name).arg(i);
            ++i;
        }
    }
    else if (tmpSet.contains(def))
    {
        return;
    }
    // Add the player
    addRow(def);

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

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setData(Qt::DisplayRole, iDef.isDefault);
    m_tablePlayers->setItem(row, 3, item);
    emit rowCountChanged();
}


void PlayersTableHelper::moveRow(int rowFrom, int rowTo)
{
    ASSERT(rowFrom >= 0 && rowFrom < m_tablePlayers->rowCount(),
           "Invalid rowFrom argument: " << rowFrom);
    ASSERT(rowTo >= 0 && rowTo < m_tablePlayers->rowCount(),
           "Invalid rowTo argument: " << rowTo);
    ASSERT(rowFrom != rowTo, "moveRow() should be called with different values");

    // We are going to insert a row, so update the indices
    int realFrom = rowFrom;
    int realTo = rowTo;
    if (rowFrom < rowTo)
        realTo = rowTo + 1;
    else
        realFrom = rowFrom + 1;

    // Insert a new row
    QAbstractItemModel *model = m_tablePlayers->model();
    model->insertRow(realTo);

    // Copy the values from the row to move
    for (int col = 0; col < model->columnCount(); ++col)
    {
        QMap<int, QVariant> itemData = model->itemData(model->index(realFrom, col));
        model->setItemData(model->index(realTo, col), itemData);
    }

    // Delete the old row
    model->removeRow(realFrom);
}


void PlayersTableHelper::moveSelectionUp()
{
    QItemSelectionModel *selModel = m_tablePlayers->selectionModel();
    if (!selModel->hasSelection())
        return;

    for (int i = 0; i < m_tablePlayers->rowCount() - 1; ++i)
    {
        if (!selModel->isRowSelected(i, QModelIndex()) &&
            selModel->isRowSelected(i + 1, QModelIndex()))
        {
            // Find the first unselected row after this contiguous selection
            int j = i + 1;
            while (j + 1 < m_tablePlayers->rowCount() &&
                   selModel->isRowSelected(j + 1, QModelIndex()))
                ++j;

            // Move the row
            moveRow(i, j);

            // Skip ahead
            i = j;
        }
    }
}


void PlayersTableHelper::moveSelectionDown()
{
    QItemSelectionModel *selModel = m_tablePlayers->selectionModel();
    if (!selModel->hasSelection())
        return;

    for (int i = m_tablePlayers->rowCount() - 1; i >= 0; --i)
    {
        if (!selModel->isRowSelected(i, QModelIndex()) &&
            selModel->isRowSelected(i - 1, QModelIndex()))
        {
            // Find the last row of this contiguous selection
            int j = i - 1;
            while (j - 1 < m_tablePlayers->rowCount() &&
                   selModel->isRowSelected(j - 1, QModelIndex()))
                --j;

            // Move the row
            moveRow(i, j);

            // Skip ahead
            i = j;
        }
    }
}


PlayerDef PlayersTableHelper::normalize(const PlayerDef &iDef) const
{
    PlayerDef defCopy = iDef;
    defCopy.type = iDef.type == _q(kAI) ? _q(kAI) : _q(kHUMAN);

    QString level;
    if (defCopy.type == _q(kAI))
    {
        bool ok = false;
        int value = iDef.level.toInt(&ok);
        if (!ok)
            value = 100;
        if (value < 0)
            value = 0;
        if (value > 100)
            value = 100;
        level = QString("%1").arg(value);
    }
    else
        level = "";
    defCopy.level = level;

    return defCopy;
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
                            qs.value("level").toString(),
                            qs.value("default").toBool());
        playersList.push_back(playerDef);
    }
    qs.endArray();
    return playersList;
}


void PlayersTableHelper::saveFavPlayers(const QList<PlayerDef> &iFavPlayers)
{
    QSettings qs;
    qs.remove("FavPlayers");
    qs.beginWriteArray("FavPlayers");
    for (int i = 0; i < iFavPlayers.size(); ++i)
    {
        LOG_DEBUG("Writing fav player");
        qs.setArrayIndex(i);
        qs.setValue("name", iFavPlayers.at(i).name);
        qs.setValue("type", iFavPlayers.at(i).type);
        qs.setValue("level", iFavPlayers.at(i).level);
        qs.setValue("default", iFavPlayers.at(i).isDefault);
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

