/*****************************************************************************
 * Eliot
 * Copyright (C) 2002-2008 Antoine Fraboulet & Olivier Teulière
 * Authors: Antoine Fraboulet <antoine.fraboulet @@ free.fr>
 *          Olivier Teulière  <ipkiss @@ gmail.com>
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

#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <cwctype>

#include "dic_internals.h"
#include "dic_exception.h"
#include "dic.h"
#include "header.h"
#include "encoding.h"
#include "regexp.h"
#include "automaton.h"
#include "grammar.h"


static const unsigned int DEFAULT_VECT_ALLOC = 100;


const DicEdge* Dictionary::seekEdgePtr(const wchar_t* s, const DicEdge *eptr) const
{
    if (*s)
    {
        const DicEdge *p = getEdgeAt(eptr->ptr);
        do
        {
            if (p->chr == getHeader().getCodeFromChar(*s))
                return seekEdgePtr(s + 1, p);
        } while (!(*p++).last);
        return getEdgeAt(0);
    }
    else
        return eptr;
}


bool Dictionary::searchWord(const wstring &iWord) const
{
    if (!validateLetters(iWord))
        return false;

    const DicEdge *e = seekEdgePtr(iWord.c_str(), getEdgeAt(getRoot()));
    return e->term;
}


/**
 * Global variables for searchWordByLen:
 *
 * A pointer to the structure is passed as a parameter
 * so that all the search_* variables appear to the functions
 * as global but the code remains re-entrant.
 * Should be better to change the algorithm ...
 */

struct params_7plus1_t
{
    wchar_t added_code;
    wdstring added_display;
    map<wdstring, vector<wdstring> > *results;
    int search_len;
    wchar_t search_wordtst[DIC_WORD_MAX];
    char search_letters[63];
};

void Dictionary::searchWordByLen(struct params_7plus1_t &params,
                                 int i, const DicEdge *edgeptr) const
{
    /* depth first search in the dictionary */
    do
    {
        /* the test is false only when reach the end-node */
        if (edgeptr->chr)
        {
            /* is the letter available in search_letters */
            if (params.search_letters[edgeptr->chr])
            {
                params.search_wordtst[i] = getHeader().getCharFromCode(edgeptr->chr);
                params.search_letters[edgeptr->chr] --;
                if (i == params.search_len)
                {
                    if (edgeptr->term)
                    {
                        // Add the solution
                        vector<wdstring> &sols = (*params.results)[params.added_display];
                        if (sols.empty() || sols.back() != params.search_wordtst)
                            sols.push_back(getHeader().convertToDisplay(params.search_wordtst));
                    }
                }
                else
                {
                    searchWordByLen(params, i + 1, getEdgeAt(edgeptr->ptr));
                }
                params.search_letters[edgeptr->chr] ++;
                params.search_wordtst[i] = L'\0';
            }

            /* the letter is of course available if we have a joker available */
            if (params.search_letters[0])
            {
                params.search_wordtst[i] = getHeader().getCharFromCode(edgeptr->chr);
                params.search_letters[0] --;
                if (i == params.search_len)
                {
                    if (edgeptr->term)
                    {
                        // Add the solution
                        vector<wdstring> &sols = (*params.results)[params.added_display];
                        if (sols.empty() || sols.back() != params.search_wordtst)
                            sols.push_back(getHeader().convertToDisplay(params.search_wordtst));
                    }
                }
                else
                {
                    searchWordByLen(params, i + 1, getEdgeAt(edgeptr->ptr));
                }
                params.search_letters[0] ++;
                params.search_wordtst[i] = L'\0';
            }
        }
    } while (! (*edgeptr++).last);
}


void Dictionary::search7pl1(const wstring &iRack,
                            map<wdstring, vector<wdstring> > &oWordList,
                            bool joker) const
{
    if (iRack == L"" || iRack.size() > DIC_WORD_MAX)
        return;

    struct params_7plus1_t params;

    for (unsigned int i = 0; i < sizeof(params.search_letters); i++)
        params.search_letters[i] = 0;

    /*
     * the letters are verified and changed to the dic internal
     * representation (using getCodeFromChar(*r))
     */
    int wordlen = 0;
    for (const wchar_t* r = iRack.c_str(); *r; r++)
    {
        if (iswalpha(*r))
        {
            params.search_letters[getHeader().getCodeFromChar(*r)]++;
            wordlen++;
        }
        else if (*r == L'?')
        {
            if (joker)
            {
                params.search_letters[0]++;
                wordlen++;
            }
            else
            {
                oWordList[0].push_back(L"** joker **");
                return;
            }
        }
    }

    if (wordlen < 1)
        return;

    const DicEdge *root_edge = getEdgeAt(getRoot());
    root_edge = getEdgeAt(root_edge->ptr);

    params.results = &oWordList;

    /* search for all the words that can be done with the letters */
    params.added_code = 0;
    params.added_display = L"";
    params.search_len = wordlen - 1;
    params.search_wordtst[wordlen] = L'\0';
    searchWordByLen(params, 0, root_edge);

    /* search for all the words that can be done with the letters +1 */
    params.search_len = wordlen;
    params.search_wordtst[wordlen + 1] = L'\0';
    const wstring &letters = getHeader().getLetters();
    for (unsigned int i = 0; i < letters.size(); i++)
    {
        unsigned int code = getHeader().getCodeFromChar(letters[i]);
        params.added_code = code;
        params.added_display = getHeader().getDisplayStr(code);
        params.search_letters[code]++;

        searchWordByLen(params, 0, root_edge);

        params.search_letters[code]--;
    }
}

/****************************************/
/****************************************/

void Dictionary::searchRacc(const wstring &iWord,
                            vector<wdstring> &oWordList,
                            unsigned int iMaxResults) const
{
    if (iWord == L"")
        return;

    // Allocate room for all the results
    if (iMaxResults)
        oWordList.reserve(iMaxResults);
    else
        oWordList.reserve(DEFAULT_VECT_ALLOC);

    // Transform the given word to make it suitable for display
    const wdstring &displayWord = getHeader().convertToDisplay(iWord);

    // Try to add a letter at the front
    const wstring &letters = getHeader().getLetters();
    for (unsigned int i = 0; i <= letters.size(); i++)
    {
        if (searchWord(letters[i] + iWord))
        {
            const wdstring &chr =
                getHeader().getDisplayStr(getHeader().getCodeFromChar(letters[i]));
            oWordList.push_back(chr + displayWord);
        }
        if (iMaxResults && oWordList.size() >= iMaxResults)
            return;
    }

    // Try to add a letter at the end
    const DicEdge *edge_seek =
        seekEdgePtr(iWord.c_str(), getEdgeAt(getRoot()));

    // Point to what the next letter can be
    const DicEdge *edge = getEdgeAt(edge_seek->ptr);

    if (edge != getEdgeAt(0))
    {
        do
        {
            if (edge->term)
            {
                oWordList.push_back(displayWord + getHeader().getDisplayStr(edge->chr));
                if (iMaxResults && oWordList.size() >= iMaxResults)
                    return;
            }
        } while (!(*edge++).last);
    }
}

/****************************************/
/****************************************/

void Dictionary::searchBenj(const wstring &iWord, vector<wdstring> &oWordList,
                            unsigned int iMaxResults) const
{
    if (iWord == L"")
        return;

    // Allocate room for all the results
    if (iMaxResults)
        oWordList.reserve(iMaxResults);
    else
        oWordList.reserve(DEFAULT_VECT_ALLOC);

    // Transform the given word to make it suitable for display
    const wdstring &displayWord = getHeader().convertToDisplay(iWord);

    const DicEdge *edge0, *edge1, *edge2, *edgetst;
    edge0 = getEdgeAt(getRoot());
    edge0 = getEdgeAt(edge0->ptr);
    do
    {
        const wdstring &chr0 = getHeader().getDisplayStr(edge0->chr);
        edge1 = getEdgeAt(edge0->ptr);
        do
        {
            const wdstring &chr1 = getHeader().getDisplayStr(edge1->chr);
            edge2 = getEdgeAt(edge1->ptr);
            do
            {
                edgetst = seekEdgePtr(iWord.c_str(), edge2);
                if (edgetst->term)
                {
                    const wdstring &chr2 = getHeader().getDisplayStr(edge2->chr);
                    oWordList.push_back(chr0 + chr1 + chr2 + displayWord);
                    if (iMaxResults && oWordList.size() >= iMaxResults)
                        return;
                }
            } while (!(*edge2++).last);
        } while (!(*edge1++).last);
    } while (!(*edge0++).last);
}

/****************************************/
/****************************************/

struct params_regexp_t
{
    unsigned int minlength;
    unsigned int maxlength;
    Automaton *automaton_field;
};


void Dictionary::searchRegexpRec(const struct params_regexp_t &params,
                                 int state,
                                 const DicEdge *edgeptr,
                                 vector<wdstring> &oWordList,
                                 unsigned int iMaxResults,
                                 const wdstring &iCurrWord,
                                 unsigned int iNbChars) const
{
    if (iMaxResults && oWordList.size() >= iMaxResults)
        return;

    int next_state;
    /* if we have a valid word we store it */
    if (params.automaton_field->accept(state) && edgeptr->term)
    {
        if (params.minlength <= iNbChars &&
            params.maxlength >= iNbChars)
        {
            oWordList.push_back(iCurrWord);
        }
    }
    /* we now drive the search by exploring the dictionary */
    const DicEdge *current = getEdgeAt(edgeptr->ptr);
    do
    {
        /* the current letter is current->chr */
        next_state = params.automaton_field->getNextState(state, current->chr);
        /* 1: the letter appears in the automaton as is */
        if (next_state)
        {
            searchRegexpRec(params, next_state, current, oWordList, iMaxResults,
                            iCurrWord + getHeader().getDisplayStr(current->chr), iNbChars + 1);
        }
    } while (!(*current++).last);
}


/**
 * Initialize the lists of letters with pre-defined lists
 * 0: all tiles
 * 1: vowels
 * 2: consonants
 * 3: user defined 1
 * 4: user defined 2
 * x: lists used during parsing
 */
static void initLetterLists(const Dictionary &iDic,
                            searchRegExpLists &iList)
{
    memset(&iList, 0, sizeof(iList));
    // Prepare the space for 5 items
    iList.symbl.assign(5, 0);
    iList.letters.assign(5, vector<bool>(DIC_LETTERS + 1, false));

    iList.symbl[0] = RE_ALL_MATCH; // All letters
    iList.symbl[1] = RE_VOWL_MATCH; // Vowels
    iList.symbl[2] = RE_CONS_MATCH; // Consonants
    iList.letters[0][0] = false;
    iList.letters[1][0] = false;
    iList.letters[2][0] = false;
    const wstring &allLetters = iDic.getHeader().getLetters();
    for (size_t i = 1; i <= allLetters.size(); ++i)
    {
        iList.letters[0][i] = true;
        iList.letters[1][i] = iDic.getHeader().isVowel(i);
        iList.letters[2][i] = iDic.getHeader().isConsonant(i);
    }

    iList.symbl[3] = RE_USR1_MATCH; // User defined list 1
    iList.symbl[4] = RE_USR2_MATCH; // User defined list 2
}


bool Dictionary::searchRegExp(const wstring &iRegexp,
                              vector<wdstring> &oWordList,
                              unsigned int iMinLength,
                              unsigned int iMaxLength,
                              unsigned int iMaxResults) const
{
    // XXX: throw an exception?
    if (iRegexp == L"")
        return true;

    // Allocate room for all the results
    // XXX: is it really a good idea?
    if (iMaxResults)
        oWordList.reserve(iMaxResults);
    else
        oWordList.reserve(DEFAULT_VECT_ALLOC);

    // Parsing
    Node *root = NULL;
    searchRegExpLists llist;
    // Initialize the lists of letters
    initLetterLists(*this, llist);
    bool parsingOk = parseRegexp(*this, (iRegexp + L"#").c_str(), &root, llist);

    if (!parsingOk)
    {
        delete root;
        throw InvalidRegexpException(convertToMb(iRegexp));
    }

    int ptl[REGEXP_MAX+1];
    uint64_t PS[REGEXP_MAX+1];

    for (int i = 0; i < REGEXP_MAX; i++)
    {
        PS[i] = 0;
        ptl[i] = 0;
    }

    int n = 1;
    int p = 1;
    root->traverse(p, n, ptl);
    PS [0] = p - 1;
    ptl[0] = p - 1;

    root->nextPos(PS);

    Automaton *a = new Automaton(root->getFirstPos(), ptl, PS, llist);
    if (a)
    {
        struct params_regexp_t params;
        params.minlength = iMinLength;
        params.maxlength = iMaxLength;
        params.automaton_field = a;
        searchRegexpRec(params, a->getInitId(),
                        getEdgeAt(getRoot()), oWordList,
                        iMaxResults ? iMaxResults + 1 : 0);
        delete a;
    }
    delete root;

    // Check whether the maximum number of results was reached
    if (iMaxResults && oWordList.size() > iMaxResults)
    {
        oWordList.pop_back();
        return false;
    }
    else
        return true;
}

