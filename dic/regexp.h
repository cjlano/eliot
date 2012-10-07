/*****************************************************************************
 * Eliot
 * Copyright (C) 1999-2012 Antoine Fraboulet & Olivier Teulière
 * Authors: Antoine Fraboulet <antoine.fraboulet @@ free.fr>
 *          Olivier Teulière <ipkiss @@ gmail.com>
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

#ifndef REGEXP_H_
#define REGEXP_H_

#include <string>
#include <vector>
#include <iosfwd>

#define NODE_TOP    0
#define NODE_VAR    1
#define NODE_OR     2
#define NODE_AND    3
#define NODE_STAR   4
#define NODE_PLUS   5

using std::string;
using std::vector;

class Node
{
public:
    /**
     * Create a node for the syntactic tree used for
     * parsing regular expressions
     */
    Node(int type, char v, Node *fg, Node *fd);

    /**
     * Delete regexp syntactic tree
     */
    ~Node();

    /**
     * Computes positions, first positions (PP), last position (DP),
     * and annulable attribute
     *
     * @param p : max position found in the tree (must be initialized to 1)
     * @param n : number of nodes in the tree (must be initialized to 1)
     * @param ptl : position to letter translation table
     */
    void traverse(int &p, int &n, int ptl[]);

    /**
     * Computes 'next position' table used for building the
     * automaton
     * @param r : root node of the syntactic tree
     * @param PS : next position table, PS[0] must contain the
     * number of terminals contained in the regular expression
     */
    void nextPos(uint64_t PS[]);

    /// Return the first position
    int getFirstPos() const { return m_PP; }

#ifdef DEBUG_RE
    /**
     * Print the tree rooted at the current node to a file suitable
     * for dot (Graphviz)
     */
    void printTreeDot(const string &iFileName, int detail) const;
#endif

private:
    int m_type;
    char m_var;
    Node *m_fg;
    Node *m_fd;
    int m_number;
    int m_position;
    bool m_annulable;
    uint64_t m_PP;
    uint64_t m_DP;

#ifdef DEBUG_RE
    /// Print the current node to file
    void printNode(ostream &out, int detail) const;

    /// Print recursively the current node and its subnodes to file
    void printNodesRec(ostream &out, int detail) const;

    /// Print recursively the edges of the tree rooted at the current node
    void printEdgesRec(ostream &out) const;
#endif
};

/**
 * different letters in the dictionary
 */
#define DIC_LETTERS 63

/**
 * maximum number of accepted terminals in regular expressions
 */
#define REGEXP_MAX 32

/**
 * special terminals that should not appear in the dictionary
 */
#define RE_EPSILON     (DIC_LETTERS + 0)
#define RE_FINAL_TOK   (DIC_LETTERS + 1)
#define RE_ALL_MATCH   (DIC_LETTERS + 2)
#define RE_VOWL_MATCH  (DIC_LETTERS + 3)
#define RE_CONS_MATCH  (DIC_LETTERS + 4)
#define RE_USR1_MATCH  (DIC_LETTERS + 5)
#define RE_USR2_MATCH  (DIC_LETTERS + 6)

/**
 * Structure used for dic.searchRegExp
 * This structure is used to explicit letters list that will be matched
 * against special tokens in the regular expression search
 */
struct searchRegExpLists
{
    /** special symbol associated with the list */
    vector<char> symbl;
    /**
     * 0 or 1 if letter is present in the list.
     * The inner vector should have a length of DIC_LETTERS+1 (it is a bitmask)
     */
    vector<vector<bool> > letters;
};

#define RE_LIST_ALL_MATCH  0
#define RE_LIST_VOYL_MATCH 1
#define RE_LIST_CONS_MATCH 2
#define RE_LIST_USER_BEGIN 3
#define RE_LIST_USER_END   4

string regexpPrintLetter(char l);
void regexp_print_PS(int PS[]);
void regexp_print_ptl(int ptl[]);

#endif /* _REGEXP_H_ */

