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

#ifndef NEW_GAME_H_
#define NEW_GAME_H_

#include <QDialog>
#include <QtGui/QItemDelegate>

#include <ui/new_game.ui.h>


class Dictionary;
class PublicGame;
class QStandardItemModel;

class NewGame: public QDialog, private Ui::NewGameDialog
{
    Q_OBJECT;

public:
    explicit NewGame(QWidget *iParent = 0);

    /// Possible values for the player type
    static const char * kHUMAN;
    static const char * kAI;

    /**
     * Create and return a game object from the information of the dialog.
     * The Game object is always valid
     */
    PublicGame * createGame(const Dictionary& iDic) const;

private slots:
    void enableLevelSpinBox(int);
    void enableOkButton();
    void enableRemoveButton(const QItemSelection&, const QItemSelection&);
    void enablePlayers(bool);

    // The following slots are automatically connected
    void on_pushButtonAdd_clicked();
    void on_pushButtonRemove_clicked();
    void on_checkBoxJoker_stateChanged(int);
    void on_checkBoxExplosive_stateChanged(int);

private:
    /// Model of the players
    QStandardItemModel *m_model;
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

