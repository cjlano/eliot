/*******************************************************************
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

#ifndef TURN_CMD_H_
#define TURN_CMD_H_

#include <vector>

#include "logging.h"

using namespace std;

class Command;


/**
 * This class encapsulates commands, and almost behaves as one itself.
 * The main difference with a normal command (and thus with the composite pattern)
 * is that a TurnCmd provides partial execution: some of the commands it contains
 * (the first ones) can be executed, whereas others not.
 * The ones which can be executed are the ones flagged "auto-executable", and are
 * named AE commands (or AEC) in the following comments.
 * The non AE commands are named NAEC.
 *
 * Without loss of generality, the commands in a TurnCmd can always be structured
 * like this in a unique way:
 *      - a (possibly empty) sequence of AEC
 *      - a (possibly empty) sequence of AEC and NAEC, starting with a NAEC
 *
 * There are only 3 valid "states" for a TurnCmd:
 *      - not at all executed (none of the commands is executed)
 *      - partially executed (all the AEC until the first NAEC are executed)
 *      - fully executed (all the commands are executed)
 * These states are not at all exclusive. For example, for a TurnCmd object without
 * any command, the three states are equal.
 *
 * Here are a few "graphical" examples, where x represents an AEC, N represents,
 * a NAEC, and ^ indicates the position of m_firstNotExecuted if the state is
 * "partially executed":
 *      |xxxxNNxNxN|  |xxxx|  |Nxxx|  ||
 *           ^             ^   ^       ^
 */
class TurnCmd
{
    DEFINE_LOGGER();

    public:
        TurnCmd();
        virtual ~TurnCmd();

        /**
         * Add the given command and execute it.
         * The TurnCmd object takes ownership of the given Command.
         */
        void addAndExecute(Command *iCmd);

        // Getters for the execution status.

        /// Return true iff all the commands have been executed
        bool isFullyExecuted() const;

        /**
         * Indicate whether all the auto-executable commands have been executed
         * (whether or not there are AE commands, and whether or not there are
         * non AE commands).
         * When isFullyExecuted() returns true, this methods also returns true.
         */
        bool isPartiallyExecuted() const;

        /// Return true iff no command has been executed
        bool isNotAtAllExecuted() const;


        bool hasNonAutoExecCmd() const;

        const vector<Command *> & getCommands() const { return m_commands; }

        bool isHumanIndependent() const;

        wstring toString() const;

        /// Execute all the commands which were not yet executed
        void execute();
        /// Undo all the commands which were not yet undone
        void undo();

        /// Execute all the AE commands, until a non AE one is found, to reach the "isPartiallyExecuted" state
        void partialExecute();
        /// Undo all the non AE commands, to reach the "isPartiallyExecuted" state
        void partialUndo();

        /// Drop the non-executed commands. Use it with care...
        void dropNonExecutedCommands();

        /// Drop (and undo if needed) all the commands, starting with the given one. Use it with care...
        void dropFrom(const Command &iCmd);

        /// Drop an insertable command. Use it with care...
        void dropCommand(const Command &iCmd);

        /**
         * Insert the given command before the first NAEC (or at the end if
         * there is no NAEC). The command must be flagged insertable and NAEC.
         */
        void insertCommand(Command *iCmd);

        /**
         * Replace the first command with the second one.
         * The TurnCmd object takes ownership of the given Command.
         * Use with care...
         */
        void replaceCommand(const Command &iOldCmd,
                            Command *iNewCmd);

        /**
         * Find the command matching the given predicate, or 0 if not found.
         * The commands are iterated from the last one to the first one,
         * and the first one to match the predicate is returned.
         */
        template<typename CMD, typename PRED>
        const CMD * findMatchingCmd(PRED predicate) const
        {
            // Iterate backwards, to be sure to have the latest one for the player
            vector<Command*>::const_reverse_iterator it;
            for (it = m_commands.rbegin(); it != m_commands.rend(); ++it)
            {
                const CMD *cmd = dynamic_cast<const CMD*>(*it);
                if (cmd != 0 && predicate(*cmd))
                {
                    // Found it!
                    return cmd;
                }
            }
            return 0;
        }

        /**
         * Find a command of the template type.
         * The last (most recent) one is returned, or 0 if not found.
         */
        template<typename CMD>
        const CMD * findMatchingCmd() const
        {
            return findMatchingCmd<CMD, TruePred<CMD> >(TruePred<CMD>());
        }

    private:
        vector<Command *> m_commands;
        /**
         * Pointer to the first not executed command.
         * If it is equal to m_commands.size(), all the commands have been executed.
         */
        unsigned int m_firstNotExecuted;

        /**
         * Return the index of the given command,
         * or m_commands.size() if not found
         */
        unsigned findIndex(const Command &iCmd) const;

        /**
         * or m_commands.size() if not found
         */
        unsigned findIndexFirstNaec() const;

        /**
         * Helper method to execute the commands (if needed) up to
         * the given position (included).
         * The previous value of m_firstNotExecuted is returned.
         */
        unsigned execTo(unsigned iNewFirstNotExec);

        /**
         * Helper method to undo the commands (if needed) up to
         * the given position (included).
         * The previous value of m_firstNotExecuted is returned.
         */
        unsigned undoTo(unsigned iNewFirstNotExec);

        /**
         * Helper method to drop everything (after undoing, if needed),
         * starting at the given index
         */
        void dropFrom(unsigned iFirstToDrop);

        template<typename T>
        struct TruePred : public unary_function<T, bool>
        {
            bool operator()(const T &) const { return true; }
        };
};

#endif

