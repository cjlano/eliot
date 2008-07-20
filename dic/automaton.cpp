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

/**
 *  \file   automaton.c
 *  \brief  (Non)Deterministic Finite AutomatonHelper for Regexp
 *  \author Antoine Fraboulet
 *  \date   2005
 */

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

#ifdef DEBUG_AUTOMATON
#   define DMSG(a) (a)
#else
#   define DMSG(a)
#endif

#define MAX_TRANSITION_LETTERS 256

typedef struct automaton_state_t *astate;


/* ************************************************** *
   Helper class, allowing to build a NFA, then a DFA
 * ************************************************** */

class AutomatonHelper
{
public:
    AutomatonHelper(astate iInitState);
    ~AutomatonHelper();

    astate getInitState() const { return m_initState; }
#ifdef DEBUG_AUTOMATON
    void dump(const string &iFileName) const;
#endif

    static AutomatonHelper *ps2nfa(uint64_t iInitState, int *ptl, uint64_t *PS);
    static AutomatonHelper *nfa2dfa(const AutomatonHelper &iNfa,
                                    struct search_RegE_list_t *iList);

    /// List of states
    list<astate> m_states;

private:
    /// Initial state of the automaton
    astate m_initState;

    void addState(astate s);
    astate getState(const set<uint64_t> &iId) const;
    void printNodes(FILE* f) const;
    void printEdges(FILE* f) const;
    void setAccept(astate s) const;
    set<uint64_t> getSuccessor(const set<uint64_t> &S, int letter, struct search_RegE_list_t *iList) const;
};


/* ************************************************** *
   State handling
 * ************************************************** */

static set<uint64_t> s_state_id_create(uint64_t id);
static string   s_state_id_to_str(const set<uint64_t> &iId);
static astate   s_state_create   (const set<uint64_t> &iId);

struct automaton_state_t
{
    set<uint64_t> id;
    bool accept;
    int      id_static;
    astate   next[MAX_TRANSITION_LETTERS];
};


/* ************************************************** *
   Definition of the Automaton class
 * ************************************************** */

Automaton::Automaton(uint64_t iInitState, int *ptl, uint64_t *PS, struct search_RegE_list_t *iList)
{
    AutomatonHelper *nfa = AutomatonHelper::ps2nfa(iInitState, ptl, PS);
    DMSG(printf("\n non deterministic automaton OK \n\n"));
    DMSG(nfa->dump("auto_nfa"));

    AutomatonHelper *dfa = AutomatonHelper::nfa2dfa(*nfa, iList);
    DMSG(printf("\n deterministic automaton OK \n\n"));
    DMSG(dfa->dump("auto_dfa"));

    finalize(*dfa);
    DMSG(printf("\n final automaton OK \n\n"));
    DMSG(automaton_dump("auto_fin"));

    delete nfa;
    delete dfa;
}


Automaton::~Automaton()
{
    delete[] m_acceptors;
    for (int i = 0; i <= m_nbStates; i++)
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
    for (int i = 0; i <= m_nbStates; i++)
    {
        m_transitions[i] = new int[MAX_TRANSITION_LETTERS];
        memset(m_transitions[i], 0, MAX_TRANSITION_LETTERS * sizeof(int));
    }

    /* Create new id for states */
    list<astate>::const_iterator it;
    int i;
    for (i = 1, it = iHelper.m_states.begin();
         it != iHelper.m_states.end(); it++, i++)
    {
        (*it)->id_static = i;
    }

    /* Build new automaton */
    for (it = iHelper.m_states.begin(); it != iHelper.m_states.end(); it++)
    {
        astate s = *it;
        int i = s->id_static;

        if (s == iHelper.getInitState())
            m_init = i;
        if (s->accept)
            m_acceptors[i] = true;

        for (int l = 0; l < MAX_TRANSITION_LETTERS; l++)
        {
            if (s->next[l])
                m_transitions[i][l] = s->next[l]->id_static;
        }
    }
}


void Automaton::dump(const string &iFileName) const
{
    FILE *f = fopen(iFileName.c_str(), "w");
    fprintf(f, "digraph automaton {\n");
    for (int i = 1; i <= m_nbStates; i++)
    {
        fprintf(f, "\t%d [label = \"%d\"", i, i);
        if (i == m_init)
            fprintf(f, ", style = filled, color=lightgrey");
        if (accept(i))
            fprintf(f, ", shape = doublecircle");
        fprintf(f, "];\n");
    }
    fprintf(f, "\n");
    for (int i = 1; i <= m_nbStates; i++)
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
   Definition of the state handling methods
 * ************************************************** */

static set<uint64_t> s_state_id_create(uint64_t id)
{
    set<uint64_t> l;
    l.insert(id);
    return l;
}


static string s_state_id_to_str(const set<uint64_t> &iId)
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


static astate s_state_create(const set<uint64_t> &iId)
{
    astate s = new automaton_state_t();
    // TODO: use copy constructor
    s->id     = iId;
    s->accept = false;
    memset(s->next, 0, sizeof(astate)*MAX_TRANSITION_LETTERS);
    DMSG(printf("** state %s creation\n", s_state_id_to_str(iId).c_str()));
    return s;
}


/* ************************************************** *
   Definition of the AutomatonHelper class
 * ************************************************** */

AutomatonHelper::AutomatonHelper(astate iInitState)
    : m_initState(iInitState)
{
}


AutomatonHelper::~AutomatonHelper()
{
    list<astate>::const_iterator it;
    for (it = m_states.begin(); it != m_states.end(); it++)
    {
        delete *it;
    }
}


void AutomatonHelper::addState(astate s)
{
    m_states.push_front(s);
    DMSG(printf("** state %s added to automaton\n", s_state_id_to_str(s->id).c_str()));
}


astate AutomatonHelper::getState(const set<uint64_t> &iId) const
{
    list<astate>::const_iterator it;
    for (it = m_states.begin(); it != m_states.end(); it++)
    {
        astate s = *it;
        if (s->id == iId)
        {
            //DMSG(printf("** get state %s ok\n", s_state_id_to_str(s->id).c_str()));
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
    astate current_state;
    char used_letter[MAX_TRANSITION_LETTERS];


    /* 1: init_state = root->PP */
    set<uint64_t> temp_id0 = s_state_id_create(init_state_id);
    astate temp_state = s_state_create(temp_id0);
    AutomatonHelper *nfa = new AutomatonHelper(temp_state);
    nfa->addState(temp_state);
    list<astate> L;
    L.push_front(temp_state);
    /* 2: while \exist state \in state_list */
    while (! L.empty())
    {
        current_state = L.front();
        L.pop_front();
        DMSG(printf("** current state = %s\n", s_state_id_to_str(current_state->id).c_str()));
        memset(used_letter, 0, sizeof(used_letter));
        /* 3: \foreach l in \sigma | l \neq # */
        for (uint32_t p = 1; p < maxpos; p++)
        {
            int current_letter = ptl[p];
            if (used_letter[current_letter] == 0)
            {
                /* 4: int set = \cup { PS(pos) | pos \in state \wedge pos == l } */
                uint64_t ens = 0;
                for (uint32_t pos = 1; pos <= maxpos; pos++)
                {
                    if (ptl[pos] == current_letter &&
                        (unsigned int)*(current_state->id.begin()) & (1 << (pos - 1)))
                        ens |= PS[pos];
                }
                /* 5: transition from current_state to temp_state */
                if (ens)
                {
                    set<uint64_t> temp_id = s_state_id_create(ens);
                    temp_state = nfa->getState(temp_id);
                    if (temp_state == NULL)
                    {
                        temp_state = s_state_create(temp_id);
                        nfa->addState(temp_state);
                        current_state->next[current_letter] = temp_state;
                        L.push_front(temp_state);
                    }
                    else
                    {
                        current_state->next[current_letter] = temp_state;
                    }
                }
                used_letter[current_letter] = 1;
            }
        }
    }

    list<astate>::const_iterator it;
    for (it = nfa->m_states.begin(); it != nfa->m_states.end(); it++)
    {
        astate s = *it;
        if (*(s->id.begin()) & (1 << (maxpos - 1)))
            s->accept = true;
    }

    return nfa;
}

/* ************************************************** *
 * ************************************************** *
 * ************************************************** */

set<uint64_t> AutomatonHelper::getSuccessor(const set<uint64_t> &S,
                                            int letter,
                                            struct search_RegE_list_t *iList) const
{
    set<uint64_t> R, r;
    set<uint64_t>::const_iterator it;
    for (it = S.begin(); it != S.end(); it++)                /* \forall y \in S */
    {
        astate y, z;

        set<uint64_t> t = s_state_id_create(*it);
        assert(y = getState(t));

        set<uint64_t> Ry;                                        /* Ry = \empty             */

        if ((z = y->next[letter]) != NULL)                   /* \delta (y,z) = l        */
        {
            r = getSuccessor(z->id, RE_EPSILON, iList);
            Ry.insert(r.begin(), r.end());
            Ry.insert(z->id.begin(), z->id.end()); /* Ry = Ry \cup succ(z)    */
        }

        /* \epsilon transition from start node */
        if ((z = y->next[RE_EPSILON]) != NULL)               /* \delta (y,z) = \epsilon */
        {
            r = getSuccessor(z->id, letter, iList);
            Ry.insert(r.begin(), r.end());       /* Ry = Ry \cup succ(z)    */
        }

        if (letter < RE_FINAL_TOK)
        {
            for (int i = 0; i < DIC_SEARCH_REGE_LIST; i++)
            {
                if (iList->valid[i])
                {
                    if (iList->letters[i][letter] && (z = y->next[(int)iList->symbl[i]]) != NULL)
                    {
                        DMSG(printf("*** letter "));
                        DMSG(regexp_print_letter(stdout, letter));
                        DMSG(printf("is in "));
                        DMSG(regexp_print_letter(stdout, i));

                        r = getSuccessor(z->id, RE_EPSILON, iList);
                        Ry.insert(r.begin(), r.end());
                        Ry.insert(z->id.begin(), z->id.end());
                    }
                }
            }
        }

#if 0
        if (alist_is_empty(Ry))                              /* Ry = \empty             */
            return Ry;
#endif

        R.insert(Ry.begin(), Ry.end());                      /* R = R \cup Ry           */
    }

    return R;
}


void AutomatonHelper::setAccept(astate s) const
{
    DMSG(printf("=== setting accept for node (%s) :", s_state_id_to_str(s->id).c_str()));
    list<astate>::const_iterator it;
    for (it = m_states.begin(); it != m_states.end(); it++)
    {
        astate ns = *it;
        int idx = *(ns->id.begin());
        DMSG(printf("%s ", s_state_id_to_str(ns->id).c_str()));
        if (ns->accept && (std::find(s->id.begin(), s->id.end(), idx) != s->id.end()))
        {
            DMSG(printf("(ok) "));
            s->accept = true;
        }
    }
    DMSG(printf("\n"));
}


AutomatonHelper *AutomatonHelper::nfa2dfa(const AutomatonHelper &iNfa,
                                          struct search_RegE_list_t *iList)
{
    astate current_state;

    list<astate> L;

    // Clone the list
    set<uint64_t> temp_id0 = iNfa.m_initState->id;
    astate temp_state = s_state_create(temp_id0);
    AutomatonHelper *dfa = new AutomatonHelper(temp_state);
    dfa->addState(temp_state);
    L.push_front(temp_state);
    while (! L.empty())
    {
        current_state = L.front();
        L.pop_front();
        DMSG(printf("** current state = %s\n", s_state_id_to_str(current_state->id).c_str()));
        for (int letter = 1; letter < DIC_LETTERS; letter++)
        {
            // DMSG(printf("*** start successor of %s\n", s_state_id_to_str(current_state->id).c_str()));

            set<uint64_t> temp_id = iNfa.getSuccessor(current_state->id, letter, iList);

            if (! temp_id.empty())
            {

                DMSG(printf("*** successor of %s for ", s_state_id_to_str(current_state->id).c_str()));
                DMSG(regexp_print_letter(stdout, letter));
                DMSG(printf(" = %s\n", s_state_id_to_str(temp_id).c_str()));

                temp_state = dfa->getState(temp_id);

                // DMSG(printf("*** automaton get state -%s- ok\n", s_state_id_to_str(temp_id).c_str()));

                if (temp_state == NULL)
                {
                    temp_state = s_state_create(temp_id);
                    dfa->addState(temp_state);
                    current_state->next[letter] = temp_state;
                    L.push_front(temp_state);
                }
                else
                {
                    current_state->next[letter] = temp_state;
                }
            }
        }
    }

    list<astate>::const_iterator it;
    for (it = dfa->m_states.begin(); it != dfa->m_states.end(); it++)
    {
        iNfa.setAccept(*it);
    }

    return dfa;
}

/* ************************************************** *
 * ************************************************** *
 * ************************************************** */

void AutomatonHelper::printNodes(FILE* f) const
{
    list<astate>::const_iterator it;
    for (it = m_states.begin(); it != m_states.end(); it++)
    {
        astate s = *it;
        string sid = s_state_id_to_str(s->id);
        fprintf(f, "\t\"%s\" [label = \"%s\"", sid.c_str(), sid.c_str());
        if (s == m_initState)
        {
            fprintf(f, ", style = filled, color=lightgrey");
        }
        if (s->accept)
        {
            fprintf(f, ", shape = doublecircle");
        }
        fprintf(f, "];\n");
    }
    fprintf(f, "\n");
}


void AutomatonHelper::printEdges(FILE* f) const
{
    list<astate>::const_iterator it;
    for (it = m_states.begin(); it != m_states.end(); it++)
    {
        astate s = *it;
        for (int letter = 0; letter < 255; letter++)
        {
            if (s->next[letter])
            {
                string sid = s_state_id_to_str(s->id);
                fprintf(f, "\t\"%s\" -> ", sid.c_str());
                sid = s_state_id_to_str(s->next[letter]->id);
                fprintf(f, "\"%s\" [label = \"", sid.c_str());
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

