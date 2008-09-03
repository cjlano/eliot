/*****************************************************************************
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

#include <string>
#include <stack>
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility/chset.hpp>
#include <boost/spirit/tree/ast.hpp>
#ifdef DEBUG_RE
#include <boost/spirit/tree/tree_to_xml.hpp>
#include <map>
#include <iostream>
#endif

#include "dic.h"
#include "header.h"
#include "regexp.h"

using namespace boost::spirit;
using namespace std;

// TODO:
// - error handling

// A few typedefs to simplify things
typedef const wchar_t *iterator_t;
typedef tree_match<iterator_t> parse_tree_match_t;
typedef parse_tree_match_t::const_tree_iterator iter_t;


struct RegexpGrammar : grammar<RegexpGrammar>
{
    static const int wrapperId = 0;
    static const int exprId = 1;
    static const int repeatId = 2;
    static const int groupId = 3;
    static const int varId = 4;
    static const int choiceId = 5;
    static const int alphavarId = 6;

    RegexpGrammar(const wstring &letters)
    {
        wstring lower = letters;
        std::transform(lower.begin(), lower.end(), lower.begin(), towlower);
        m_allLetters = letters + lower;
    }

    template <typename ScannerT>
    struct definition
    {
        // Constructor
        definition(const RegexpGrammar &self)
        {
            wrapper
                = expr >> L"#"
                ;

            expr
                = repeat >> *expr;
                ;

            repeat
                = group >> root_node_d[ch_p(L'?')]
                | group >> root_node_d[ch_p(L'*')]
                | group >> root_node_d[ch_p(L'+')]
                | group
                ;

            group
                = var
                | root_node_d[str_p(L"[^")] >> choice >> no_node_d[ch_p(L']')]
                | root_node_d[ch_p(L'[')] >> choice >> no_node_d[ch_p(L']')]
                | root_node_d[ch_p(L'(')] >> +repeat >> no_node_d[ch_p(L')')] // XXX: 'expr' instead of '+repeat' doesn't work. Why?
                ;

            var
                = alphavar
                | ch_p(L'.')
                | str_p(L":v:")
                | str_p(L":c:")
                | str_p(L":1:")
                | str_p(L":2:")
                ;

            choice
                = leaf_node_d[+alphavar]
                ;

            alphavar
                = chset<wchar_t>(self.m_allLetters.c_str())
                ;
        }

        rule<ScannerT, parser_context<>, parser_tag<wrapperId> > wrapper;
        rule<ScannerT, parser_context<>, parser_tag<exprId> > expr;
        rule<ScannerT, parser_context<>, parser_tag<repeatId> > repeat;
        rule<ScannerT, parser_context<>, parser_tag<groupId> > group;
        rule<ScannerT, parser_context<>, parser_tag<varId> > var;
        rule<ScannerT, parser_context<>, parser_tag<choiceId> > choice;
        rule<ScannerT, parser_context<>, parser_tag<alphavarId> > alphavar;

        const rule<ScannerT, parser_context<>, parser_tag<wrapperId> > & start() const { return wrapper; }
    };

    wstring m_allLetters;
};


void evaluate(const Header &iHeader, iter_t const& i, stack<Node*> &evalStack,
              searchRegExpLists &iList, bool negate = false)
{
    if (i->value.id() == RegexpGrammar::alphavarId)
    {
        assert(i->children.size() == 0);

        // Extract the character and convert it to its internal code
        uint8_t code = iHeader.getCodeFromChar(*i->value.begin());
        Node *n = new Node(NODE_VAR, code, NULL, NULL);
        evalStack.push(n);
    }
    else if (i->value.id() == RegexpGrammar::choiceId)
    {
        assert(i->children.size() == 0);

        wstring choiceLetters(i->value.begin(), i->value.end());
        // Make sure the letters are in upper case
        std::transform(choiceLetters.begin(), choiceLetters.end(),
                       choiceLetters.begin(), towupper);
        // The dictionary letters are already in upper case
        const wstring &letters = iHeader.getLetters();
        wstring::const_iterator itLetter;
        // j is the index of the new list we create
        size_t j = iList.symbl.size();
        iList.symbl.push_back(RE_ALL_MATCH + j);
        iList.letters.push_back(vector<bool>(DIC_LETTERS + 1, false));
        for (itLetter = letters.begin(); itLetter != letters.end(); ++itLetter)
        {
            bool contains = (choiceLetters.find(*itLetter) != string::npos);
            iList.letters[j][iHeader.getCodeFromChar(*itLetter)] =
                (contains ? !negate : negate);
        }
        Node *node = new Node(NODE_VAR, iList.symbl[j], NULL, NULL);
        evalStack.push(node);
    }
    else if (i->value.id() == RegexpGrammar::varId)
    {
        assert(i->children.size() == 0);

        string var(i->value.begin(), i->value.end());
        Node *node = NULL;
        if (var == ":v:")
            node = new Node(NODE_VAR, RE_VOWL_MATCH, NULL, NULL);
        else if (var == ":c:")
            node = new Node(NODE_VAR, RE_CONS_MATCH, NULL, NULL);
        else if (var == ":1:")
            node = new Node(NODE_VAR, RE_USR1_MATCH, NULL, NULL);
        else if (var == ":2:")
            node = new Node(NODE_VAR, RE_USR2_MATCH, NULL, NULL);
        else if (var == ".")
            node = new Node(NODE_VAR, RE_ALL_MATCH, NULL, NULL);
        else
            assert(0);

        evalStack.push(node);
    }
    else if (i->value.id() == RegexpGrammar::groupId)
    {
        if (*i->value.begin() == L'(')
        {
            assert(i->children.size() != 0);
            // Create a node for each child
            iter_t iter;
            for (iter = i->children.begin(); iter != i->children.end(); ++iter)
                evaluate(iHeader, iter, evalStack, iList);
            // "Concatenate" the created child nodes with AND nodes
            for (unsigned int j = 0; j < i->children.size() - 1; ++j)
            {
                Node *old2 = evalStack.top();
                evalStack.pop();
                Node *old1 = evalStack.top();
                evalStack.pop();
                Node *node = new Node(NODE_AND, '\0', old1, old2);
                evalStack.push(node);
            }
        }
        else if (*i->value.begin() == L'[')
        {
            assert(i->children.size() == 1);
            bool hasCaret = (i->value.begin() + 1 != i->value.end());
            evaluate(iHeader, i->children.begin(), evalStack, iList, hasCaret);
        }
        else
            assert(0);
    }
    else if (i->value.id() == RegexpGrammar::repeatId)
    {
        assert(i->children.size() == 1);
        evaluate(iHeader, i->children.begin(), evalStack, iList);

        if (*i->value.begin() == L'*')
        {
            assert(i->children.size() == 1);
            Node *old = evalStack.top();
            evalStack.pop();
            Node *node = new Node(NODE_STAR, '\0', old, NULL);
            evalStack.push(node);
        }
        else if (*i->value.begin() == L'+')
        {
            assert(i->children.size() == 1);
            Node *old = evalStack.top();
            evalStack.pop();
            Node *node = new Node(NODE_PLUS, '\0', old, NULL);
            evalStack.push(node);
        }
        else if (*i->value.begin() == L'?')
        {
            assert(i->children.size() == 1);
            Node *old = evalStack.top();
            evalStack.pop();
            Node *epsilon = new Node(NODE_VAR, RE_EPSILON, NULL, NULL);
            Node *node = new Node(NODE_OR, '\0', old, epsilon);
            evalStack.push(node);
        }
        else
            assert(0);
    }
    else if (i->value.id() == RegexpGrammar::exprId)
    {
        assert(i->children.size() == 2);
        evaluate(iHeader, i->children.begin(), evalStack, iList);
        evaluate(iHeader, i->children.begin() + 1, evalStack, iList);

        Node *old2 = evalStack.top();
        evalStack.pop();
        Node *old1 = evalStack.top();
        evalStack.pop();
        Node *node = new Node(NODE_AND, '\0', old1, old2);
        evalStack.push(node);
    }
    else if (i->value.id() == RegexpGrammar::wrapperId)
    {
        assert(i->children.size() == 2);
        evaluate(iHeader, i->children.begin(), evalStack, iList);
        Node *old = evalStack.top();
        evalStack.pop();
        Node* sharp = new Node(NODE_VAR, RE_FINAL_TOK, NULL, NULL);
        Node *node = new Node(NODE_AND, '\0', old, sharp);
        evalStack.push(node);
    }
    else
    {
        assert(0);
    }
}


bool parseRegexp(const Dictionary &iDic, const wchar_t *input, Node **root,
                 searchRegExpLists &iList)
{
    // Create a grammar object
    RegexpGrammar g(iDic.getHeader().getLetters());
    // Parse the input and generate an Abstract Syntax Tree (AST)
    tree_parse_info<const wchar_t*> info = ast_parse(input, g);

    if (info.full)
    {
#ifdef DEBUG_RE
        // Dump parse tree as XML
        std::map<parser_id, std::string> rule_names;
        rule_names[RegexpGrammar::wrapperId] = "wrapper";
        rule_names[RegexpGrammar::exprId] = "expr";
        rule_names[RegexpGrammar::repeatId] = "repeat";
        rule_names[RegexpGrammar::groupId] = "group";
        rule_names[RegexpGrammar::varId] = "var";
        rule_names[RegexpGrammar::choiceId] = "choice";
        rule_names[RegexpGrammar::alphavarId] = "alphavar";
        tree_to_xml(cout, info.trees);
#endif

        stack<Node*> evalStack;
        evaluate(iDic.getHeader(), info.trees.begin(), evalStack, iList);
        assert(evalStack.size() == 1);
        *root = evalStack.top();
        return true;
    }
    else
    {
        return false;
    }
}

