/*****************************************************************************
 * Eliot
 * Copyright (C) 2009 Olivier Teulière
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

#ifndef MARK_PLAYED_CMD_H_
#define MARK_PLAYED_CMD_H_

#include "command.h"

class Duplicate;


/**
 * Command used internally to change the "has played" flag of a player
 * in a duplicate game.
 */
class MarkPlayedCmd: public Command
{
    public:
        MarkPlayedCmd(Duplicate &ioDuplicate,
                      unsigned int iPlayerId,
                      bool iPlayedFlag);

        virtual wstring toString() const;

    protected:
        virtual void doExecute();
        virtual void doUndo();

    private:
        Duplicate &m_duplicateGame;
        unsigned int m_playerId;
        bool m_newPlayedFlag;
        bool m_oldPlayedFlag;
};

#endif

