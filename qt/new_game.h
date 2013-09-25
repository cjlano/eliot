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

#ifndef NEW_GAME_H_
#define NEW_GAME_H_

#include <QDialog>
#include <QPalette>

#include "ui/new_game.ui.h"
#include "logging.h"


class PlayersTableHelper;
class Dictionary;
class PublicGame;

class NewGame: public QDialog, private Ui::NewGameDialog
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    explicit NewGame(const Dictionary& iDic, QWidget *iParent = 0);

    /// Possible values for the player type
    static const char * kHUMAN;
    static const char * kAI;

    /**
     * Create and return a game object from the information of the dialog.
     * The Game object is always valid
     */
    PublicGame * createGame() const;

signals:
    void notifyProblem(QString iMsg) const;

private slots:
    void enableOkButton();
    void enableMasterControls();
    void enablePlayers(bool);
    void addSelectedToFav();
    void addFavoritePlayers();
    void onJokerChecked(int);
    void onExplosiveChecked(int);
    void browseMasterGame();
    void validateMasterGame(QString);

private:
    const Dictionary &m_dic;
    PlayersTableHelper *m_helper;

    QPalette m_blackPalette;
    QPalette m_redPalette;

    bool isMasterGameValid(QString iPath) const;
};

#endif

