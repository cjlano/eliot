/*****************************************************************************
 * Eliot
 * Copyright (C) 2009-2012 Olivier Teulière
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

#ifndef MASTER_MOVE_CMD_H_
#define MASTER_MOVE_CMD_H_

#include "command.h"
#include "move.h"
#include "logging.h"

class Duplicate;


/**
 * Command used to set the "master move" in a duplicate game.
 */
class MasterMoveCmd: public Command
{
    DEFINE_LOGGER();

    public:
        MasterMoveCmd(Duplicate &ioDuplicate,
                      const Move &iMove);

        const Move &getMove() const { return m_newMove; }

        virtual wstring toString() const;

    protected:
        virtual void doExecute();
        virtual void doUndo();

    private:
        Duplicate &m_duplicateGame;
        Move m_oldMove;
        Move m_newMove;
};

#endif

