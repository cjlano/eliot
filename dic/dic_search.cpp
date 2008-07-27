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
#include "dic.h"
#include "header.h"
#include "encoding.h"
#include "regexp.h"
#include "automaton.h"
#include "grammar.h"


static const unsigned int DEFAULT_VECT_ALLOC = 100;


template <typename DAWG_EDGE>
const DAWG_EDGE* Dictionary::seekEdgePtr(const wchar_t* s, const DAWG_EDGE *eptr) const
{
    if (*s)
    {
        const DAWG_EDGE *p = getEdgeAt<DAWG_EDGE>(eptr->ptr);
        do
        {
            if (p->chr == getHeader().getCodeFromChar(*s))
                return seekEdgePtr(s + 1, p);
        } while (!(*p++).last);
        return getEdgeAt<DAWG_EDGE>(0);
    }
    else
        return eptr;
}


bool Dictionary::searchWord(const wstring &iWord) const
{
    if (!validateLetters(iWord))
        return false;

    if (getHeader().getVersion() == 0)
    {
        const DicEdgeOld *e =
            seekEdgePtr(iWord.c_str(), getEdgeAt<DicEdgeOld>(getRoot()));
        return e->term;
    }
    else
    {
        const DicEdge *e =
            seekEdgePtr(iWord.c_str(), getEdgeAt<DicEdge>(getRoot()));
        return e->term;
    }
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
    wchar_t added_char;
    map<wchar_t, vector<wstring> > *results;
    int search_len;
    wchar_t search_wordtst[DIC_WORD_MAX];
    char search_letters[63];
};

template <typename DAWG_EDGE>
void Dictionary::searchWordByLen(struct params_7plus1_t *params,
                                 int i, const DAWG_EDGE *edgeptr) const
{
    /* depth first search in the dictionary */
    do
    {
        /* the test is false only when reach the end-node */
        if (edgeptr->chr)
        {
            /* is the letter available in search_letters */
            if (params->search_letters[edgeptr->chr])
            {
                params->search_wordtst[i] = getHeader().getCharFromCode(edgeptr->chr);
                params->search_letters[edgeptr->chr] --;
                if (i == params->search_len)
                {
                    if (edgeptr->term)
                    {
                        (*params->results)[params->added_char].push_back(params->search_wordtst);
                    }
                }
                else
                {
                    searchWordByLen(params, i + 1, getEdgeAt<DAWG_EDGE>(edgeptr->ptr));
                }
                params->search_letters[edgeptr->chr] ++;
                params->search_wordtst[i] = L'\0';
            }

            /* the letter is of course available if we have a joker available */
            if (params->search_letters[0])
            {
                params->search_wordtst[i] = getHeader().getCharFromCode(edgeptr->chr);
                params->search_letters[0] --;
                if (i == params->search_len)
                {
                    if (edgeptr->term)
                    {
                        (*params->results)[params->added_char].push_back(params->search_wordtst);
                    }
                }
                else
                {
                    searchWordByLen(params, i + 1, getEdgeAt<DAWG_EDGE>(edgeptr->ptr));
                }
                params->search_letters[0] ++;
                params->search_wordtst[i] = L'\0';
            }
        }
    } while (! (*edgeptr++).last);
}


template <typename DAWG_EDGE>
void Dictionary::search7pl1Templ(const wstring &iRack,
                                 map<wchar_t, vector<wstring> > &oWordList,
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

    const DAWG_EDGE *root_edge = getEdgeAt<DAWG_EDGE>(getRoot());
    root_edge = getEdgeAt<DAWG_EDGE>(root_edge->ptr);

    params.results = &oWordList;

    /* search for all the words that can be done with the letters */
    params.added_char = L'\0';
    params.search_len = wordlen - 1;
    params.search_wordtst[wordlen] = L'\0';
    searchWordByLen(&params, 0, root_edge);

    /* search for all the words that can be done with the letters +1 */
    params.search_len = wordlen;
    params.search_wordtst[wordlen + 1] = L'\0';
    const wstring &letters = getHeader().getLetters();
    for (unsigned int i = 0; i < letters.size(); i++)
    {
        params.added_char = letters[i];
        unsigned int code = getHeader().getCodeFromChar(letters[i]);
        params.search_letters[code]++;

        searchWordByLen(&params, 0, root_edge);

        params.search_letters[code]--;
    }
}


void Dictionary::search7pl1(const wstring &iRack,
                            map<wchar_t, vector<wstring> > &oWordList,
                            bool joker) const
{
    if (getHeader().getVersion() == 0)
        search7pl1Templ<DicEdgeOld>(iRack, oWordList, joker);
    else
        search7pl1Templ<DicEdge>(iRack, oWordList, joker);
}

/****************************************/
/****************************************/

template <typename DAWG_EDGE>
void Dictionary::searchRaccTempl(const wstring &iWord, vector<wstring> &oWordList,
                                 unsigned int iMaxResults) const
{
    if (iWord == L"")
        return;

    // Allocate room for all the results
    if (iMaxResults)
        oWordList.reserve(iMaxResults);
    else
        oWordList.reserve(DEFAULT_VECT_ALLOC);

    // Try to add a letter at the front
    wchar_t wordtst[DIC_WORD_MAX];
    wcscpy(wordtst + 1, iWord.c_str());
    const wstring &letters = getHeader().getLetters();
    for (unsigned int i = 0; i <= letters.size(); i++)
    {
        wordtst[0] = letters[i];
        if (searchWord(wordtst))
            oWordList.push_back(wordtst);
        if (iMaxResults && oWordList.size() >= iMaxResults)
            return;
    }

    // Try to add a letter at the end
    int i;
    for (i = 0; iWord[i]; i++)
        wordtst[i] = iWord[i];

    wordtst[i  ] = '\0';
    wordtst[i+1] = '\0';

    const DAWG_EDGE *edge_seek =
        seekEdgePtr(iWord.c_str(), getEdgeAt<DAWG_EDGE>(getRoot()));

    /* points to what the next letter can be */
    const DAWG_EDGE *edge = getEdgeAt<DAWG_EDGE>(edge_seek->ptr);

    if (edge != getEdgeAt<DAWG_EDGE>(0))
    {
        do
        {
            if (edge->term)
            {
                wordtst[i] = getHeader().getCharFromCode(edge->chr);
                oWordList.push_back(wordtst);
                if (iMaxResults && oWordList.size() >= iMaxResults)
                    return;
            }
        } while (!(*edge++).last);
    }
}


void Dictionary::searchRacc(const wstring &iWord, vector<wstring> &oWordList, unsigned int iMaxResults) const
{
    if (getHeader().getVersion() == 0)
        searchRaccTempl<DicEdgeOld>(iWord, oWordList, iMaxResults);
    else
        searchRaccTempl<DicEdge>(iWord, oWordList, iMaxResults);
}

/****************************************/
/****************************************/

template <typename DAWG_EDGE>
void Dictionary::searchBenjTempl(const wstring &iWord, vector<wstring> &oWordList,
                                 unsigned int iMaxResults) const
{
    if (iWord == L"")
        return;

    // Allocate room for all the results
    if (iMaxResults)
        oWordList.reserve(iMaxResults);
    else
        oWordList.reserve(DEFAULT_VECT_ALLOC);

    wchar_t wordtst[DIC_WORD_MAX];
    wcscpy(wordtst + 3, iWord.c_str());
    const DAWG_EDGE *edge0, *edge1, *edge2, *edgetst;
    edge0 = getEdgeAt<DAWG_EDGE>(getRoot());
    edge0 = getEdgeAt<DAWG_EDGE>(edge0->ptr);
    do
    {
        wordtst[0] = getHeader().getCharFromCode(edge0->chr);
        edge1 = getEdgeAt<DAWG_EDGE>(edge0->ptr);
        do
        {
            wordtst[1] = getHeader().getCharFromCode(edge1->chr);
            edge2 = getEdgeAt<DAWG_EDGE>(edge1->ptr);
            do
            {
                edgetst = seekEdgePtr(iWord.c_str(), edge2);
                if (edgetst->term)
                {
                    wordtst[2] = getHeader().getCharFromCode(edge2->chr);
                    oWordList.push_back(wordtst);
                    if (iMaxResults && oWordList.size() >= iMaxResults)
                        return;
                }
            } while (!(*edge2++).last);
        } while (!(*edge1++).last);
    } while (!(*edge0++).last);
}


void Dictionary::searchBenj(const wstring &iWord, vector<wstring> &oWordList,
                            unsigned int iMaxResults) const
{
    if (getHeader().getVersion() == 0)
        searchBenjTempl<DicEdgeOld>(iWord, oWordList, iMaxResults);
    else
        searchBenjTempl<DicEdge>(iWord, oWordList, iMaxResults);
}

/****************************************/
/****************************************/

struct params_cross_t
{
    int wordlen;
    wchar_t mask[DIC_WORD_MAX];
};


template <typename DAWG_EDGE>
void Dictionary::searchCrossRecTempl(struct params_cross_t *params,
                                     vector<wstring> &oWordList,
                                     const DAWG_EDGE *edgeptr,
                                     unsigned int iMaxResults) const
{
    if (iMaxResults && oWordList.size() >= iMaxResults)
        return;

    const DAWG_EDGE *current = getEdgeAt<DAWG_EDGE>(edgeptr->ptr);

    if (params->mask[params->wordlen] == '\0')
    {
        if (edgeptr->term)
            oWordList.push_back(params->mask);
    }
    else if (current->chr == 0)
    {
        // Do not go on recursion if we are on the sink
        return;
    }
    else if (params->mask[params->wordlen] == '.')
    {
        do
        {
            params->mask[params->wordlen] = getHeader().getCharFromCode(current->chr);
            params->wordlen ++;
            searchCrossRecTempl(params, oWordList, current, iMaxResults);
            params->wordlen --;
            params->mask[params->wordlen] = '.';
        }
        while (!(*current++).last);
    }
    else
    {
        do
        {
            if (current->chr == getHeader().getCodeFromChar(params->mask[params->wordlen]))
            {
                params->wordlen ++;
                searchCrossRecTempl(params, oWordList, current, iMaxResults);
                params->wordlen --;
                break;
            }
        }
        while (!(*current++).last);
    }
}


void Dictionary::searchCross(const wstring &iMask, vector<wstring> &oWordList,
                             unsigned int iMaxResults) const
{
    if (iMask == L"")
        return;

    // Allocate room for all the results
    if (iMaxResults)
        oWordList.reserve(iMaxResults);
    else
        oWordList.reserve(DEFAULT_VECT_ALLOC);

    struct params_cross_t params;

    int i;
    for (i = 0; i < DIC_WORD_MAX && iMask[i]; i++)
    {
        if (iswalpha(iMask[i]))
            params.mask[i] = towupper(iMask[i]);
        else
            params.mask[i] = '.';
    }
    params.mask[i] = '\0';

    params.wordlen = 0;
    if (getHeader().getVersion() == 0)
    {
        searchCrossRecTempl(&params, oWordList,
                            getEdgeAt<DicEdgeOld>(getRoot()), iMaxResults);
    }
    else
    {
        searchCrossRecTempl(&params, oWordList,
                            getEdgeAt<DicEdge>(getRoot()), iMaxResults);
    }
}

/****************************************/
/****************************************/

struct params_regexp_t
{
    int minlength;
    int maxlength;
    Automaton *automaton_field;
    wchar_t word[DIC_WORD_MAX];
    int  wordlen;
};


template <typename DAWG_EDGE>
void Dictionary::searchRegexpRecTempl(struct params_regexp_t *params,
                                      int state,
                                      const DAWG_EDGE *edgeptr,
                                      vector<wstring> &oWordList,
                                      unsigned int iMaxResults) const
{
    if (iMaxResults && oWordList.size() >= iMaxResults)
        return;

    int next_state;
    /* if we have a valid word we store it */
    if (params->automaton_field->accept(state) && edgeptr->term)
    {
        int l = wcslen(params->word);
        if (params->minlength <= l &&
            params->maxlength >= l)
        {
            oWordList.push_back(params->word);
        }
    }
    /* we now drive the search by exploring the dictionary */
    const DAWG_EDGE *current = getEdgeAt<DAWG_EDGE>(edgeptr->ptr);
    do
    {
        /* the current letter is current->chr */
        next_state = params->automaton_field->getNextState(state, current->chr);
        /* 1: the letter appears in the automaton as is */
        if (next_state)
        {
            params->word[params->wordlen] =
                towlower(getHeader().getCharFromCode(current->chr));
            params->wordlen ++;
            searchRegexpRecTempl(params, next_state, current, oWordList, iMaxResults);
            params->wordlen --;
            params->word[params->wordlen] = L'\0';
        }
    } while (!(*current++).last);
}


static void init_letter_lists(const Dictionary &iDic, struct search_RegE_list_t &iList)
{
    memset(&iList, 0, sizeof(iList));
    // Prepare the space for 5 items
    iList.symbl.assign(5, 0);

    iList.valid[0] = true; // all letters
    iList.symbl[0] = RE_ALL_MATCH;
    iList.valid[1] = true; // vowels
    iList.symbl[1] = RE_VOWL_MATCH;
    iList.valid[2] = true; // consonants
    iList.symbl[2] = RE_CONS_MATCH;
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

    iList.valid[3] = false; // user defined list 1
    iList.symbl[3] = RE_USR1_MATCH;
    iList.valid[4] = false; // user defined list 2
    iList.symbl[4] = RE_USR2_MATCH;
}


void Dictionary::searchRegExp(const wstring &iRegexp,
                              vector<wstring> &oWordList,
                              unsigned int iMinLength,
                              unsigned int iMaxLength,
                              unsigned int iMaxResults) const
{
    if (iRegexp == L"")
        return;

    // Allocate room for all the results
    if (iMaxResults)
        oWordList.reserve(iMaxResults);
    else
        oWordList.reserve(DEFAULT_VECT_ALLOC);

    // Parsing
    Node *root = NULL;
    struct search_RegE_list_t llist;
    init_letter_lists(*this, llist);
    bool parsingOk = parseRegexp(*this, (iRegexp + L"#").c_str(), &root, &llist);

    if (!parsingOk)
    {
        // TODO
        delete root;
        return;
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

    Automaton *a = new Automaton(root->getFirstPos(), ptl, PS, &llist);
    if (a)
    {
        struct params_regexp_t params;
        params.minlength = iMinLength;
        params.maxlength = iMaxLength;
        params.automaton_field = a;
        memset(params.word, L'\0', sizeof(params.word));
        params.wordlen = 0;
        if (getHeader().getVersion() == 0)
        {
            searchRegexpRecTempl(&params, a->getInitId(),
                                 getEdgeAt<DicEdgeOld>(getRoot()), oWordList, iMaxResults);
        }
        else
        {
            searchRegexpRecTempl(&params, a->getInitId(),
                                 getEdgeAt<DicEdge>(getRoot()), oWordList, iMaxResults);
        }

        delete a;
    }
    delete root;
}

