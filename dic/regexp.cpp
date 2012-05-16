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

#include <boost/format.hpp>
#include <fstream>

#include "dic.h"
#include "regexp.h"

using boost::format;


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


string regexpPrintLetter(char l)
{
    if (l == RE_EPSILON)    return (format("( &  [%1%])") % l).str();
    if (l == RE_FINAL_TOK)  return (format("( #  [%1%])") % l).str();
    if (l == RE_ALL_MATCH)  return (format("( .  [%1%])") % l).str();
    if (l == RE_VOWL_MATCH) return (format("(:v: [%1%])") % l).str();
    if (l == RE_CONS_MATCH) return (format("(:c: [%1%])") % l).str();
    if (l == RE_USR1_MATCH) return (format("(:1: [%1%])") % l).str();
    if (l == RE_USR2_MATCH) return (format("(:2: [%1%])") % l).str();
    if (l < RE_FINAL_TOK)
        return (format("(%1% [%2%])") % (char)(l + 'a' - 1) % (int)l).str();
    else
        return (format("(liste %1%)") % (l - RE_LIST_USER_END)).str();
}


#ifdef DEBUG_RE
void Node::printNode(ostream &out, int detail) const
{
    switch (m_type)
    {
        case NODE_VAR:
            out << regexpPrintLetter(m_var);
            break;
        case NODE_OR:
            out << "OR";
            break;
        case NODE_AND:
            out << "AND";
            break;
        case NODE_PLUS:
            out << "+";
            break;
        case NODE_STAR:
            out << "*";
            break;
    }
    if (detail == 2)
    {
        out << format("\\n pos=%1%\\n annul=%2%\\n PP=0x%3%\\n DP=0x%3%")
            % m_position % m_annulable % m_PP % m_DP;
    }
}

void Node::printNodesRec(ostream &out, int detail) const
{
    if (m_fg)
        m_fg->printNodesRec(out, detail);
    if (m_fd)
        m_fd->printNodesRec(out, detail);

    out << m_number << " [ label=\"";
    printNode(out, detail);
    out << "\"];\n";
}

void Node::printEdgesRec(ostream &out) const
{
    if (m_fg)
        m_fg->printEdgesRec(out);
    if (m_fd)
        m_fd->printEdgesRec(out);

    switch (m_type)
    {
        case NODE_OR:
            out << format("%1% -> %2%;") % m_number % m_fg->m_number;
            out << format("%1% -> %2%;") % m_number % m_fd->m_number;
            break;
        case NODE_AND:
            out << format("%1% -> %2%;") % m_number % m_fg->m_number;
            out << format("%1% -> %2%;") % m_number % m_fd->m_number;
            break;
        case NODE_PLUS:
        case NODE_STAR:
            out << format("%1% -> %2%;") % m_number % m_fg->m_number;
            break;
    }
}

void Node::printTreeDot(const string &iFileName, int detail) const
{
    ofstream out(iFileName.c_str());
    out << "digraph " << iFileName << " {\n";
    printNodesRec(out, detail);
    printEdgesRec(out);
    out << "fontsize=20;\n";
    out << "}\n";
    out.close();
}
#endif

