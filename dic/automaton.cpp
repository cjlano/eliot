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

#include "config.h"

#include <set>
#include <list>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sys/types.h>
#ifdef HAVE_SYS_WAIT_H
#   include <sys/wait.h>
#endif
#include <unistd.h>

#include "dic.h"
#include "regexp.h"
#include "automaton.h"

using namespace std;


INIT_LOGGER(dic, Automaton);


#ifdef DEBUG_AUTOMATON
#   define DMSG(a) (a)
#else
#   define DMSG(a)
#endif

#define MAX_TRANSITION_LETTERS 256


/* ************************************************** *
   Definition of the automaton state
 * ************************************************** */

static string idToString(const set<uint64_t> &iId);

class State
{
public:
    State(const set<uint64_t> iId) : m_id(iId) { init(); }
    State(uint64_t iId)
    {
        m_id.insert(iId);
        init();
    }

    const set<uint64_t> & getId() const { return m_id; }

    // FIXME: should be private
    bool m_accept;
    int id_static;
    State * m_next[MAX_TRANSITION_LETTERS];

private:
    /**
     * Id of the state. For the first automaton, each ID contains only 1
     * integer, but the ID of the deterministic automaton will contain
     * several integers, according to the usual "determinization" algorithm.
     */
    set<uint64_t> m_id;

    void init()
    {
        m_accept = false;
        id_static = 0;
        memset(m_next, 0, sizeof(State*) * MAX_TRANSITION_LETTERS);
        DMSG(printf("** state %s creation\n", idToString(m_id).c_str()));
    }
};

/* ************************************************** *
   Helper class, allowing to build a NFA, then a DFA
 * ************************************************** */

class AutomatonHelper
{
public:
    AutomatonHelper(State * iInitState);
    ~AutomatonHelper();

    State * getInitState() const { return m_initState; }
#ifdef DEBUG_AUTOMATON
    void dump(const string &iFileName) const;
#endif

    static AutomatonHelper *ps2nfa(uint64_t iInitState, int *ptl, uint64_t *PS);
    static AutomatonHelper *nfa2dfa(const AutomatonHelper &iNfa,
                                    const searchRegExpLists &iList);

    /// List of states
    list<State *> m_states;

private:
    /// Initial state of the automaton
    State * m_initState;

    void addState(State * s);
    State * getState(const set<uint64_t> &iId) const;
    void printNodes(FILE* f) const;
    void printEdges(FILE* f) const;
    void setAccept(State * s) const;
    set<uint64_t> getSuccessor(const set<uint64_t> &S, int letter,
                               const searchRegExpLists &iList) const;
};


/* ************************************************** *
   Definition of the Automaton class
 * ************************************************** */

Automaton::Automaton(uint64_t iInitState, int *ptl, uint64_t *PS,
                     const searchRegExpLists &iList)
{
    AutomatonHelper *nfa = AutomatonHelper::ps2nfa(iInitState, ptl, PS);
    DMSG(printf("\n non deterministic automaton OK \n\n"));
    DMSG(nfa->dump("auto_nfa"));

    AutomatonHelper *dfa = AutomatonHelper::nfa2dfa(*nfa, iList);
    DMSG(printf("\n deterministic automaton OK \n\n"));
    DMSG(dfa->dump("auto_dfa"));

    finalize(*dfa);
    DMSG(printf("\n final automaton OK \n\n"));
    DMSG(dump("auto_fin"));

    delete nfa;
    delete dfa;
}


Automaton::~Automaton()
{
    delete[] m_acceptors;
    for (unsigned int i = 0; i <= m_nbStates; i++)
    {
        delete[] m_transitions[i];
    }
    delete[] m_transitions;
}


void Automaton::finalize(const AutomatonHelper &iHelper)
{
    /* Creation */
    m_nbStates = iHelper.m_states.size();
    m_acceptors = new bool[m_nbStates + 1];
    memset(m_acceptors, 0, (m_nbStates + 1) * sizeof(bool));
    m_transitions = new int*[m_nbStates + 1];
    for (unsigned int i = 0; i <= m_nbStates; i++)
    {
        m_transitions[i] = new int[MAX_TRANSITION_LETTERS];
        memset(m_transitions[i], 0, MAX_TRANSITION_LETTERS * sizeof(int));
    }

    /* Create new id for states */
    list<State *>::const_iterator it;
    int i;
    for (i = 1, it = iHelper.m_states.begin();
         it != iHelper.m_states.end(); it++, i++)
    {
        (*it)->id_static = i;
    }

    /* Build new automaton */
    for (it = iHelper.m_states.begin(); it != iHelper.m_states.end(); it++)
    {
        State * s = *it;
        int i = s->id_static;

        if (s == iHelper.getInitState())
            m_init = i;
        if (s->m_accept)
            m_acceptors[i] = true;

        for (int l = 0; l < MAX_TRANSITION_LETTERS; l++)
        {
            if (s->m_next[l])
                m_transitions[i][l] = s->m_next[l]->id_static;
        }
    }
}


void Automaton::dump(const string &iFileName) const
{
    FILE *f = fopen(iFileName.c_str(), "w");
    fprintf(f, "digraph automaton {\n");
    for (unsigned int i = 1; i <= m_nbStates; i++)
    {
        fprintf(f, "\t%d [label = \"%d\"", i, i);
        if (i == m_init)
            fprintf(f, ", style = filled, color=lightgrey");
        if (accept(i))
            fprintf(f, ", shape = doublecircle");
        fprintf(f, "];\n");
    }
    fprintf(f, "\n");
    for (unsigned int i = 1; i <= m_nbStates; i++)
    {
        for (int l = 0; l < MAX_TRANSITION_LETTERS; l++)
        {
            if (m_transitions[i][l])
            {
                fprintf(f, "\t%d -> %d [label = \"", i, m_transitions[i][l]);
                regexp_print_letter(f, l);
                fprintf(f, "\"];\n");
            }
        }
    }
    fprintf(f, "fontsize=20;\n");
    fprintf(f, "}\n");
    fclose(f);

#ifdef HAVE_SYS_WAIT_H
    pid_t pid = fork ();
    if (pid > 0)
    {
        wait(NULL);
    }
    else if (pid == 0)
    {
        execlp("dotty", "dotty", iFileName.c_str(), NULL);
        printf("exec dotty failed\n");
        exit(1);
    }
#endif
}


/* ************************************************** *
   Definition of the AutomatonHelper class
 * ************************************************** */

AutomatonHelper::AutomatonHelper(State * iInitState)
    : m_initState(iInitState)
{
}


AutomatonHelper::~AutomatonHelper()
{
    list<State *>::const_iterator it;
    for (it = m_states.begin(); it != m_states.end(); it++)
    {
        delete *it;
    }
}


void AutomatonHelper::addState(State * s)
{
    m_states.push_front(s);
    DMSG(printf("** state %s added to automaton\n", idToString(s->getId()).c_str()));
}


State * AutomatonHelper::getState(const set<uint64_t> &iId) const
{
    list<State *>::const_iterator it;
    for (it = m_states.begin(); it != m_states.end(); it++)
    {
        State * s = *it;
        if (s->getId() == iId)
        {
            //DMSG(printf("** get state %s ok\n", idToString(s->getId()).c_str()));
            return s;
        }
    }
    return NULL;
}

/* ************************************************** *
 * ************************************************** *
 * ************************************************** */

AutomatonHelper *AutomatonHelper::ps2nfa(uint64_t init_state_id, int *ptl, uint64_t *PS)
{
    uint64_t maxpos = PS[0];
    State * current_state;
    bool used_letter[MAX_TRANSITION_LETTERS];


    /* 1: init_state = root->PP */
    State * temp_state = new State(init_state_id);
    AutomatonHelper *nfa = new AutomatonHelper(temp_state);
    nfa->addState(temp_state);
    list<State *> L;
    L.push_front(temp_state);
    /* 2: while \exist state \in state_list */
    while (! L.empty())
    {
        current_state = L.front();
        L.pop_front();
        DMSG(printf("** current state = %s\n", idToString(current_state->getId()).c_str()));
        memset(used_letter, 0, sizeof(used_letter));
        /* 3: \foreach l in \sigma | l \neq # */
        for (uint32_t p = 1; p < maxpos; p++)
        {
            int current_letter = ptl[p];
            if (used_letter[current_letter] == false)
            {
                /* 4: int set = \cup { PS(pos) | pos \in state \wedge pos == l } */
                uint64_t ens = 0;
                for (uint32_t pos = 1; pos <= maxpos; pos++)
                {
                    if (ptl[pos] == current_letter &&
                        (unsigned int)*(current_state->getId().begin()) & (1 << (pos - 1)))
                        ens |= PS[pos];
                }
                /* 5: transition from current_state to temp_state */
                if (ens)
                {
                    set<uint64_t> temp_id;
                    temp_id.insert(ens);
                    temp_state = nfa->getState(temp_id);
                    if (temp_state == NULL)
                    {
                        temp_state = new State(temp_id);
                        nfa->addState(temp_state);
                        L.push_front(temp_state);
                    }
                    current_state->m_next[current_letter] = temp_state;
                }
                used_letter[current_letter] = true;
            }
        }
    }

    list<State *>::const_iterator it;
    for (it = nfa->m_states.begin(); it != nfa->m_states.end(); it++)
    {
        State * s = *it;
        if (*(s->getId().begin()) & (1 << (maxpos - 1)))
            s->m_accept = true;
    }

    return nfa;
}

/* ************************************************** *
 * ************************************************** *
 * ************************************************** */

set<uint64_t> AutomatonHelper::getSuccessor(const set<uint64_t> &S,
                                            int letter,
                                            const searchRegExpLists &iList) const
{
    set<uint64_t> R, r;
    set<uint64_t>::const_iterator it;
    for (it = S.begin(); it != S.end(); it++)                /* \forall y \in S */
    {

        set<uint64_t> t;
        t.insert(*it);
        State *y = getState(t);
        assert(y != NULL);

        set<uint64_t> Ry;                                        /* Ry = \empty             */

        State *z;
        if ((z = y->m_next[letter]) != NULL)                   /* \delta (y,z) = l        */
        {
            r = getSuccessor(z->getId(), RE_EPSILON, iList);
            Ry.insert(r.begin(), r.end());
            Ry.insert(z->getId().begin(), z->getId().end()); /* Ry = Ry \cup succ(z)    */
        }

        /* \epsilon transition from start node */
        if ((z = y->m_next[RE_EPSILON]) != NULL)               /* \delta (y,z) = \epsilon */
        {
            r = getSuccessor(z->getId(), letter, iList);
            Ry.insert(r.begin(), r.end());       /* Ry = Ry \cup succ(z)    */
        }

        if (letter < RE_FINAL_TOK)
        {
            for (unsigned int i = 0; i < iList.symbl.size(); i++)
            {
                if (iList.letters[i][letter] && (z = y->m_next[(int)iList.symbl[i]]) != NULL)
                {
                    DMSG(printf("*** letter "));
                    DMSG(regexp_print_letter(stdout, letter));
                    DMSG(printf("is in "));
                    DMSG(regexp_print_letter(stdout, i));

                    r = getSuccessor(z->getId(), RE_EPSILON, iList);
                    Ry.insert(r.begin(), r.end());
                    Ry.insert(z->getId().begin(), z->getId().end());
                }
            }
        }

        R.insert(Ry.begin(), Ry.end());                   /* R = R \cup Ry  */
    }

    return R;
}


void AutomatonHelper::setAccept(State * s) const
{
    DMSG(printf("=== setting accept for node (%s) :", idToString(s->getId()).c_str()));
    list<State *>::const_iterator it;
    for (it = m_states.begin(); it != m_states.end(); it++)
    {
        State * ns = *it;
        uint64_t idx = *(ns->getId().begin());
        DMSG(printf("%s ", idToString(ns->getId()).c_str()));
        if (ns->m_accept && (std::find(s->getId().begin(), s->getId().end(), idx) != s->getId().end()))
        {
            DMSG(printf("(ok) "));
            s->m_accept = true;
        }
    }
    DMSG(printf("\n"));
}


AutomatonHelper *AutomatonHelper::nfa2dfa(const AutomatonHelper &iNfa,
                                          const searchRegExpLists &iList)
{
    State * current_state;

    list<State *> L;

    // Clone the list
    State * temp_state = new State(iNfa.m_initState->getId());
    AutomatonHelper *dfa = new AutomatonHelper(temp_state);
    dfa->addState(temp_state);
    L.push_front(temp_state);
    while (! L.empty())
    {
        current_state = L.front();
        L.pop_front();
        DMSG(printf("** current state = %s\n", idToString(current_state->getId()).c_str()));
        for (int letter = 1; letter < DIC_LETTERS; letter++)
        {
            // DMSG(printf("*** start successor of %s\n", idToString(current_state->getId()).c_str()));

            set<uint64_t> temp_id = iNfa.getSuccessor(current_state->getId(), letter, iList);

            if (! temp_id.empty())
            {
                DMSG(printf("*** successor of %s for ", idToString(current_state->getId()).c_str()));
                DMSG(regexp_print_letter(stdout, letter));
                DMSG(printf(" = %s\n", idToString(temp_id).c_str()));

                temp_state = dfa->getState(temp_id);

                // DMSG(printf("*** automaton get state -%s- ok\n", idToString(temp_id).c_str()));

                if (temp_state == NULL)
                {
                    temp_state = new State(temp_id);
                    dfa->addState(temp_state);
                    L.push_front(temp_state);
                }
                current_state->m_next[letter] = temp_state;
            }
        }
    }

    list<State *>::const_iterator it;
    for (it = dfa->m_states.begin(); it != dfa->m_states.end(); it++)
    {
        iNfa.setAccept(*it);
    }

    return dfa;
}

/* ************************************************** *
 * ************************************************** *
 * ************************************************** */

static string idToString(const set<uint64_t> &iId)
{
    string s;
    set<uint64_t>::const_iterator it;
    for (it = iId.begin(); it != iId.end(); it++)
    {
        char tmp[50];
        sprintf(tmp, "%llu ", *it);
        s += tmp;
    }
    return s;
}


void AutomatonHelper::printNodes(FILE* f) const
{
    list<State *>::const_iterator it;
    for (it = m_states.begin(); it != m_states.end(); it++)
    {
        State * s = *it;
        string sid = idToString(s->getId());
        fprintf(f, "\t\"%s\" [label = \"%s\"", sid.c_str(), sid.c_str());
        if (s == m_initState)
        {
            fprintf(f, ", style = filled, color=lightgrey");
        }
        if (s->m_accept)
        {
            fprintf(f, ", shape = doublecircle");
        }
        fprintf(f, "];\n");
    }
    fprintf(f, "\n");
}


void AutomatonHelper::printEdges(FILE* f) const
{
    list<State *>::const_iterator it;
    for (it = m_states.begin(); it != m_states.end(); it++)
    {
        State * s = *it;
        for (int letter = 0; letter < 255; letter++)
        {
            if (s->m_next[letter])
            {
                fprintf(f, "\t\"%s\" -> ", idToString(s->getId()).c_str());
                fprintf(f, "\"%s\" [label = \"", idToString(s->m_next[letter]->getId()).c_str());
                regexp_print_letter(f, letter);
                fprintf(f, "\"];\n");
            }
        }
    }
}


#ifdef DEBUG_AUTOMATON
void AutomatonHelper::dump(const string &iFileName) const
{
    FILE *f = fopen(iFileName.c_str(), "w");
    fprintf(f, "digraph automaton {\n");
    printNodes(f);
    printEdges(f);
    fprintf(f, "fontsize=20;\n");
    fprintf(f, "}\n");
    fclose(f);

#ifdef HAVE_SYS_WAIT_H
    pid_t pid = fork();
    if (pid > 0)
    {
        wait(NULL);
    }
    else if (pid == 0)
    {
        execlp("dotty", "dotty", iFileName.c_str(), NULL);
        printf("exec dotty failed\n");
        exit(1);
    }
#endif
}
#endif

