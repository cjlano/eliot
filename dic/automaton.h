/*****************************************************************************
 * Eliot
 * Copyright (C) 2005-2007 Antoine Fraboulet
 * Authors: Antoine Fraboulet <antoine.fraboulet @@ free.fr>
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

#ifndef DIC_AUTOMATON_H_
#define DIC_AUTOMATON_H_

class AutomatonHelper;
struct searchRegExpLists;

class Automaton
{
public:
    /// Constructor
    /**
     * Build a static deterministic finite automaton from
     * "init_state", "ptl" and "PS" given by the parser
     */
    Automaton(uint64_t init_state, int *ptl, uint64_t *PS,
              const searchRegExpLists &iList);

    /// Destructor
    ~Automaton();

    /**
     * Get the number of states in the automaton.
     * @returns number of states
     */
    int getNbStates() const { return m_nbStates; }

    /**
     * Query the id of the init state.
     * @returns init state id
     */
    int getInitId() const { return m_init; }

    /**
     * Query the acceptor flag for the given state
     * @return true/false
     */
    bool accept(uint64_t state) const { return m_acceptors[state]; }

    /**
     * Return the next state when the transition is taken
     * @returns next state id (1 <= id <= nstate, 0 = invalid id)
     */
    uint64_t getNextState(uint64_t start, char l) const
    {
        return m_transitions[start][(int)l];
    }

    /**
     * Dump the automaton into a file (for debugging purposes)
     */
    void dump(const string &iFileName) const;

private:
    /// Number of states
    unsigned int m_nbStates;

    /// ID of the init state
    uint64_t m_init;

    /// Array of booleans, one for each state
    bool *m_acceptors;

    /// Matrix of transitions
    int **m_transitions;

    void finalize(const AutomatonHelper &a);
};

#endif /* _DIC_AUTOMATON_H_ */
