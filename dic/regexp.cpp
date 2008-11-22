/*****************************************************************************
 * Eliot
 * Copyright (C) 1999-2006 Antoine Fraboulet
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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#ifdef HAVE_SYS_WAIT_H
#   include <sys/wait.h>
#endif
#include <unistd.h>

#include "dic.h"
#include "regexp.h"
#include "automaton.h"


Node::Node(int type, char v, Node *fg, Node *fd)
    : m_type(type), m_var(v), m_fg(fg), m_fd(fd), m_number(0), m_position(0),
    m_annulable(false), m_PP(0), m_DP(0)
{
}

Node::~Node()
{
    delete m_fg;
    delete m_fd;
}


/**
 * p is the current leaf position
 * n is the current node number
 */
void Node::traverse(int &p, int &n, int ptl[])
{
    if (m_fg)
        m_fg->traverse(p, n, ptl);
    if (m_fd)
        m_fd->traverse(p, n, ptl);

    m_number = n;
    ++n;

    switch (m_type)
    {
        case NODE_VAR:
            m_position = p;
            ptl[p] = m_var;
            ++p;
            m_annulable = false;
            m_PP = 1 << (m_position - 1);
            m_DP = 1 << (m_position - 1);
            break;
        case NODE_OR:
            m_position = 0;
            m_annulable = m_fg->m_annulable || m_fd->m_annulable;
            m_PP = m_fg->m_PP | m_fd->m_PP;
            m_DP = m_fg->m_DP | m_fd->m_DP;
            break;
        case NODE_AND:
            m_position = 0;
            m_annulable = m_fg->m_annulable && m_fd->m_annulable;
            m_PP = (m_fg->m_annulable) ? (m_fg->m_PP | m_fd->m_PP) : m_fg->m_PP;
            m_DP = (m_fd->m_annulable) ? (m_fg->m_DP | m_fd->m_DP) : m_fd->m_DP;
            break;
        case NODE_PLUS:
            m_position = 0;
            m_annulable = false;
            m_PP = m_fg->m_PP;
            m_DP = m_fg->m_DP;
            break;
        case NODE_STAR:
            m_position = 0;
            m_annulable = true;
            m_PP = m_fg->m_PP;
            m_DP = m_fg->m_DP;
            break;
    }
}


void Node::nextPos(uint64_t PS[])
{
    if (m_fg)
        m_fg->nextPos(PS);
    if (m_fd)
        m_fd->nextPos(PS);

    switch (m_type)
    {
        case NODE_AND:
            /************************************/
            /* \forall p \in DP(left)           */
            /*     PS[p] = PS[p] \cup PP(right) */
            /************************************/
            for (uint32_t pos = 1; pos <= PS[0]; pos++)
            {
                if (m_fg->m_DP & (1 << (pos-1)))
                    PS[pos] |= m_fd->m_PP;
            }
            break;
        case NODE_PLUS:
            /************************************/
            /* == same as START                 */
            /* \forall p \in DP(left)           */
            /*     PS[p] = PS[p] \cup PP(left)  */
            /************************************/
            for (uint32_t pos = 1; pos <= PS[0]; pos++)
            {
                if (m_DP & (1 << (pos-1)))
                    PS[pos] |= m_PP;
            }
            break;
        case NODE_STAR:
            /************************************/
            /* \forall p \in DP(left)           */
            /*     PS[p] = PS[p] \cup PP(left)  */
            /************************************/
            for (uint32_t pos = 1; pos <= PS[0]; pos++)
            {
                if (m_DP & (1 << (pos-1)))
                    PS[pos] |= m_PP;
            }
            break;
    }
}

////////////////////////////////////////////////
// DEBUG only fonctions
////////////////////////////////////////////////

#ifdef DEBUG_RE
void printPS(int PS[])
{
    printf("** next positions **\n");
    for (int i = 1; i <= PS[0]; i++)
    {
        printf("%02d: 0x%08x\n", i, PS[i]);
    }
}

void regexp_print_ptl(int ptl[])
{
    printf("** pos -> lettre: ");
    for (int i = 1; i <= ptl[0]; i++)
    {
        printf("%d=%c ", i, ptl[i]);
    }
    printf("\n");
}
#endif


void regexp_print_letter(FILE* f, char l)
{
    switch (l)
    {
        case RE_EPSILON:    fprintf(f, "( &  [%d])", l); break;
        case RE_FINAL_TOK:  fprintf(f, "( #  [%d])", l);  break;
        case RE_ALL_MATCH:  fprintf(f, "( .  [%d])", l);  break;
        case RE_VOWL_MATCH: fprintf(f, "(:v: [%d])", l); break;
        case RE_CONS_MATCH: fprintf(f, "(:c: [%d])", l); break;
        case RE_USR1_MATCH: fprintf(f, "(:1: [%d])", l); break;
        case RE_USR2_MATCH: fprintf(f, "(:2: [%d])", l); break;
        default:
            if (l < RE_FINAL_TOK)
                fprintf(f, " (%c [%d]) ", l + 'a' - 1, l);
            else
                fprintf(f, " (liste %d)", l - RE_LIST_USER_END);
            break;
    }
}


void regexp_print_letter2(FILE* f, char l)
{
    switch (l)
    {
        case RE_EPSILON:    fprintf(f, "&"); break;
        case RE_FINAL_TOK:  fprintf(f, "#");  break;
        case RE_ALL_MATCH:  fprintf(f, ".");  break;
        case RE_VOWL_MATCH: fprintf(f, ":v:"); break;
        case RE_CONS_MATCH: fprintf(f, ":c:"); break;
        case RE_USR1_MATCH: fprintf(f, ":1:"); break;
        case RE_USR2_MATCH: fprintf(f, ":2:"); break;
        default:
            if (l < RE_FINAL_TOK)
                fprintf(f, "%c", l + 'a' - 1);
            else
                fprintf(f, "l%d", l - RE_LIST_USER_END);
            break;
    }
}


#ifdef DEBUG_RE
void Node::printNode(FILE* f, int detail) const
{
    switch (m_type)
    {
        case NODE_VAR:
            regexp_print_letter(f, m_var);
            break;
        case NODE_OR:
            fprintf(f, "OR");
            break;
        case NODE_AND:
            fprintf(f, "AND");
            break;
        case NODE_PLUS:
            fprintf(f, "+");
            break;
        case NODE_STAR:
            fprintf(f, "*");
            break;
    }
    if (detail == 2)
    {
        fprintf(f, "\\n pos=%d\\n annul=%d\\n PP=0x%04x\\n DP=0x%04x",
                m_position, m_annulable, m_PP, m_DP);
    }
}

void Node::printNodesRec(FILE* f, int detail) const
{
    if (m_fg)
        m_fg->printNodesRec(f, detail);
    if (m_fd)
        m_fd->printNodesRec(f, detail);

    fprintf(f, "%d [ label=\"", m_number);
    printNode(f, detail);
    fprintf(f, "\"];\n");
}

void Node::printEdgesRec(FILE *f) const
{
    if (m_fg)
        m_fg->printEdgesRec(f);
    if (m_fd)
        m_fd->printEdgesRec(f);

    switch (m_type)
    {
        case NODE_OR:
            fprintf(f, "%d -> %d;", m_number, m_fg->m_number);
            fprintf(f, "%d -> %d;", m_number, m_fd->m_number);
            break;
        case NODE_AND:
            fprintf(f, "%d -> %d;", m_number, m_fg->m_number);
            fprintf(f, "%d -> %d;", m_number, m_fd->m_number);
            break;
        case NODE_PLUS:
        case NODE_STAR:
            fprintf(f, "%d -> %d;", m_number, m_fg->m_number);
            break;
    }
}

void Node::printTreeDot(const string &iFileName, int detail) const
{
    FILE *f = fopen(iFileName.c_str(), "w");
    if (f == NULL)
        return;
    fprintf(f, "digraph %s {\n", iFileName.c_str());
    printNodesRec(f, detail);
    printEdgesRec(f);
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

