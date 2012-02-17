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

#ifndef PLAYERS_TABLE_HELPER_H_
#define PLAYERS_TABLE_HELPER_H_

#include <QtGui/QWidget>
#include <QtGui/QItemDelegate>
#include <QtCore/QString>
#include <QtCore/QList>

#include "logging.h"


class QTableWidget;
class QPushButton;
class QMenu;
class QPoint;


struct PlayerDef
{
    PlayerDef(QString name, QString type, QString level);
    QString name;
    QString type;
    QString level;

    bool operator==(const PlayerDef &) const;
};


class PlayersTableHelper : public QObject
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    /// Possible values for the player type
    static const char * kHUMAN;
    static const char * kAI;

    PlayersTableHelper(QObject *parent,
                       QTableWidget *tableWidget,
                       QPushButton *addButton = 0,
                       QPushButton *removeButton = 0);

    QList<PlayerDef> getPlayers(bool onlySelected) const;
    void addPlayers(const QList<PlayerDef> &iList);
    void addPlayer(const PlayerDef &iPlayer);

    static QList<PlayerDef> getFavPlayers();
    static void saveFavPlayers(const QList<PlayerDef> &iFavPlayers);

    int getRowCount() const;

    void addPopupAction(QAction *iAction);
    QAction * getRemoveAction();

signals:
    void rowCountChanged();

private slots:
    void populateMenu(QMenu &, const QPoint &);
    void enableRemoveButton();
    void removeSelectedRows();
    void addRow();
    void addRow(const PlayerDef &iDef);

private:
    QTableWidget *m_tablePlayers;
    QPushButton *m_buttonAdd;
    QPushButton *m_buttonRemove;
    QList<QAction*> m_popupActions;

};


/// Delegate used for the edition of the players type
class PlayersTypeDelegate: public QItemDelegate
{
    Q_OBJECT;

public:
    explicit PlayersTypeDelegate(QObject *parent = 0);
    virtual ~PlayersTypeDelegate() {}

    // Implement the needed methods
    virtual QWidget *createEditor(QWidget *parent,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor,
                               const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor,
                              QAbstractItemModel *model,
                              const QModelIndex &index) const;

    virtual void updateEditorGeometry(QWidget *editor,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const;
};


/// Delegate used for the edition of the players level
class PlayersLevelDelegate: public QItemDelegate
{
    Q_OBJECT;

public:
    explicit PlayersLevelDelegate(QObject *parent = 0);
    virtual ~PlayersLevelDelegate() {}

    // Implement the needed methods
    virtual QWidget *createEditor(QWidget *parent,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor,
                               const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor,
                              QAbstractItemModel *model,
                              const QModelIndex &index) const;

    virtual void updateEditorGeometry(QWidget *editor,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const;
};


/// Event filter used for the edition of the players display
class PlayersEventFilter: public QObject
{
    Q_OBJECT;

public:
    explicit PlayersEventFilter(QObject *parent = 0);
    virtual ~PlayersEventFilter() {}

signals:
    /// As its name indicates...
    void deletePressed();

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);
};

#endif

