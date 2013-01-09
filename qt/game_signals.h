/*****************************************************************************
 * Eliot
 * Copyright (C) 2013 Olivier Teulière
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

#ifndef GAME_SIGNALS_H_
#define GAME_SIGNALS_H_

#include <QObject>

#include "pldrack.h"
#include "logging.h"


class PublicGame;


/**
 * Since the Game library does not emit signals, GameSignals wraps a PublicGame
 * object and emit specific signals.
 * It acts as a signals demultiplexer: when receiving a gameUpdated() signal,
 * it finds out if some Game properties have changed, and if so, emits more
 * specific signals (such as gameRackChanged()).
 */
class GameSignals: public QObject
{
    Q_OBJECT;
    DEFINE_LOGGER();

public:
    GameSignals();

public slots:
    void notifyGameUpdated();
    void notifyGameChanged(PublicGame *iGame);

signals:
    /// The PublicGame object itself is changed (could be NULL)
    void gameChangedNonConst(PublicGame *iGame);
    /// The PublicGame object itself is changed (could be NULL)
    void gameChanged(const PublicGame *iGame);

    /// Something changed in the game. This is the least precise signal.
    void gameUpdated();

    /// The current turn has changed
    void turnChanged(int iCurrTurn, bool isLastTurn);

    /// Like turnChanged(), but only emitted when a new turn is created
    void newTurn(int iCurrTurn);

    /// Emitted when the game rack changes
    void gameRackChanged(const PlayedRack &iRack);

    /**
     * Emitted when the rack of the current player changes (because
     * the rack itself changes, or because the current player changes)
     */
    void currPlayerRackChanged(const PlayedRack &iRack);

private:
    /// Wrapped game (can be NULL)
    const PublicGame *m_game;

    /// Current turn number. Used to emit turnChanged()
    unsigned m_currentTurn;

    /// Last known turn number. Used to emit newTurn()
    unsigned m_lastTurn;

    // Last known game rack. Used to emit gameRackChanged()
    PlayedRack m_lastGameRack;

    // Last known rack for the current player. Used to emit currPlayerRackChanged()
    PlayedRack m_lastCurrPlayerRack;

};

#endif

