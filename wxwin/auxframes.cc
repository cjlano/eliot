/* Eliot                                                                     */
/* Copyright (C) 1999  Antoine Fraboulet                                     */
/* Antoine.Fraboulet@free.fr                                                 */
/*                                                                           */
/* This program is free software; you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 2 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#include <iostream>
using namespace std;

#include "wx/sizer.h"
#include "wx/button.h"
#include "wx/intl.h"

#include "wx/clipbrd.h"
#include "wx/dataobj.h"

#include "ewx.h"

#include "dic.h"
#include "dic_search.h"
#include "training.h"
#include "game.h"

#include "configdb.h"
#include "auxframes.h"
#include "mainframe.h"
#include "searchpanel.h"

/****************************************************************/
/* AUXFRAME */
/****************************************************************/

AuxFrame::AuxFrame(wxFrame* parent, int _id, wxString _name, wxString _classname):
    wxFrame(parent, -1, wxU("Eliot: ") + _name, wxPoint(-1, -1), wxSize(-1, -1),
            wxRESIZE_BORDER | wxCAPTION | wxFRAME_FLOAT_ON_PARENT, _classname)
{
  frameid   = (frames_id_t)_id;
  name      = _name;
  classname = _classname;
  show      = config.getFrameShow(classname);
}

AuxFrame::~AuxFrame()
{
    wxPoint p = GetPosition();
    wxSize  s = GetClientSize();
    config.setFramePos(classname, p);
    config.setFrameSize(classname, s);
    config.setFrameShow(classname, show);
}

void
AuxFrame::SwitchDisplay()
{
    if (show == 0)
    {
        Show(TRUE);
        Raise();
        show = 1;
    }
    else
    {
        Show(FALSE);
        show = 0;
    }
}

void
AuxFrame::Reload()
{
#define MINW 50
#define MINH 50

    wxSize size;
    Move(config.getFramePos(classname));
    size = config.getFrameSize(classname);

    if (size.GetWidth() < MINW)
        size.SetWidth(MINW);
    if (size.GetHeight() < MINH)
        size.SetHeight(MINH);

    SetClientSize(size);
    Refresh();
    if (show)
    {
        Show(FALSE);
        Show(TRUE);
    }
}

/****************************************************************/
/* BOARD FRAME */
/****************************************************************/

BoardFrame::BoardFrame(wxFrame* parent, Game& iGame):
    AuxFrame(parent, ID_Frame_Board, wxT("Grille"), FRAMEBOARD)
{
    board = new GfxBoard(this, iGame);

    wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
    sizer->Add(board, 1, wxEXPAND, 0);
    SetAutoLayout(TRUE);
    SetSizer(sizer);
    sizer->Fit(this);
    sizer->SetSizeHints(this);
}

void
BoardFrame::Refresh(refresh_t force)
{
    if (force == REFRESH)
        board->Refresh(BOARD_REFRESH);
    else
        board->Refresh(BOARD_FORCE_REFRESH);
}

/****************************************************************/
/* BAG FRAME */
/****************************************************************/

BagFrame::BagFrame(wxFrame* parent, Game& iGame):
    AuxFrame(parent, ID_Frame_Bag, wxT("sac"), FRAMEBAG),
    m_game(iGame)
{
    tiles = new wxListCtrl(this, -1);
    tiles->SetSingleStyle(wxLC_LIST);
    tiles->SetColumnWidth(0, wxLIST_AUTOSIZE);
    tiles->SetFont(config.getFont(LISTFONT));
    tiles->SetToolTip(wxT("Lettre, nombre restant"));

    wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
    sizer->Add(tiles, 1, wxEXPAND | wxALL, 1);
    SetAutoLayout(TRUE);
    SetSizer(sizer);
    sizer->Fit(this);
}

void
BagFrame::Refresh(refresh_t force)
{
    int index;
    wxString buf;
    wxChar format[] = wxT("%c:%2d");

    tiles->ClearAll();

    const list<Tile>& allTiles = Tile::getAllTiles();
    list<Tile>::const_iterator it;
    for (index = 0, it = allTiles.begin(); it != allTiles.end(); index++, it++)
    {
        buf.Printf(format, it->toChar(), m_game.getNCharInBag(*it));
        tiles->InsertItem(index,buf);
    }
}

/****************************************************************/
/* RECHERCHE */
/****************************************************************/

SearchFrame::SearchFrame(wxFrame *parent, Dictionary _dic):
    AuxFrame(parent, ID_Frame_Search, wxT("recherche"), FRAMESEARCH)
{
    panel = new SearchPanel(this, _dic);
    wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
    sizer->Add(panel, 1, wxEXPAND, 0);
    SetAutoLayout(TRUE);
    SetSizer(sizer);
    sizer->Fit(this);
}

void
SearchFrame::Refresh(refresh_t force)
{
}

/****************************************************************/
/* VERIF FRAME */
/****************************************************************/

enum
{
    Word_Id,
    Result_Id,
};

BEGIN_EVENT_TABLE(VerifFrame, AuxFrame)
  EVT_TEXT(Word_Id, VerifFrame::OnText)
END_EVENT_TABLE()

VerifFrame::VerifFrame(wxFrame* parent, Dictionary _dic):
  AuxFrame(parent, ID_Frame_Verif, wxT("vérification"), FRAMEVERIF)
{
    dic = _dic;
    word = new wxTextCtrl(this, Word_Id, wxT(""));
    word->SetFont(config.getFont(LISTFONT));
    word->SetToolTip(wxT("Mot à vérifier"));
    result = new wxStaticText(this, Result_Id, wxT(""));
    result->SetFont(config.getFont(LISTFONT));
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(word, 1, wxEXPAND | wxALL, 1);
    sizer->Add(result, 1, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT | wxALL, 1);

    SetAutoLayout(TRUE);
    SetSizer(sizer);
    sizer->Fit(this);
    sizer->SetSizeHints(this);
}

void
VerifFrame::verif()
{
    if (dic == NULL)
    {
        result->SetLabel(wxT("pas de dictionnaire"));
        return;
    }
    if (Dic_search_word(dic, word->GetValue().mb_str()))
        result->SetLabel(wxT("existe"));
    else
        result->SetLabel(wxT("n'existe pas"));
}

void
VerifFrame::OnText(wxCommandEvent&)
{
    verif();
}

void
VerifFrame::Refresh(refresh_t force)
{
}

/** *******************************************************************
 **
 **
 **
 **
 **
 ** ********************************************************************/

/****************************************************************/
/* AUXFRAMELIST */
/****************************************************************/

enum {
  ListBoxID,
  ButtonCopyID
};

BEGIN_EVENT_TABLE(AuxFrameList, AuxFrame)
  EVT_BUTTON        (ButtonCopyID , AuxFrameList::OnCopy)
END_EVENT_TABLE()

AuxFrameList::AuxFrameList(wxFrame* parent, int _id, wxString _name, wxString _classname):
  AuxFrame(parent, _id, _name, _classname)

{
    wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
    listbox = new wxListBox(this, ListBoxID);
    listbox->SetFont(config.getFont(LISTFONT));
    listbox->SetToolTip(name);
    sizer_v->Add(listbox, 1, wxEXPAND | wxALL, 1);

    button = new wxButton(this, ButtonCopyID, wxT("Copier"), wxPoint(0, 0), wxSize(-1, -1));
    sizer_v->Add(button, 0, wxEXPAND | wxALL, 1);

    wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
    sizer->Add(sizer_v, 1, wxEXPAND, 0);

    SetAutoLayout(TRUE);
    SetSizer(sizer);
    sizer->Fit(this);
    sizer->SetSizeHints(this);
}

void
AuxFrameList::OnCopy(wxCommandEvent& event)
{
    wxString textdata;

    if (wxTheClipboard->Open())
    {
        textdata = wxT("");
        for (int i = 0; i < listbox->GetCount(); i++)
        {
            textdata << listbox->GetString(i) << wxT("\n");
        }
        wxTextDataObject* ptr = new wxTextDataObject(textdata);
        wxTheClipboard->AddData(ptr);
        wxTheClipboard->Close();
    }
}

void
AuxFrameList::Waiting()
{
    listbox->Clear();
    listbox->Show(TRUE);
}

/****************************************************************/
/* PLUS1 FRAME */
/****************************************************************/

Plus1Frame::Plus1Frame(wxFrame* parent, Game& iGame):
    AuxFrameList(parent, ID_Frame_Plus1, wxT("Tirage + 1"), FRAMEPLUS1),
    m_game(iGame)
{
    m_rack[0] = '\0';
}

void
Plus1Frame::Refresh(refresh_t force)
{
    int  i, j;
    string rack2;
    char buff[DIC_LETTERS][RES_7PL1_MAX][DIC_WORD_MAX];

    rack2 = m_game.getPlayerRack(0);

    if (m_rack == rack2)
        return;

    m_rack = rack2;

    Waiting();
    Dic_search_7pl1(m_game.getDic(), m_rack.c_str(), buff, config.getJokerPlus1());

    int resnum = 0;
    wxString res[DIC_LETTERS*(RES_7PL1_MAX+1)];
    // wxString(wxT) added for clean compile with wx2.4
    res[resnum++] = wxString(wxT("Tirage: ")) + wxString(wxU(m_rack.c_str()));
    for (i = 0; i < DIC_LETTERS; i++)
    {
        if (i && buff[i][0][0])
            res[resnum++] = wxString(wxT("+")) + (wxChar)(i + 'A' - 1);
        for (j = 0; j < RES_7PL1_MAX && buff[i][j][0]; j++)
            res[resnum++] = wxString(wxT("  ")) + wxU(buff[i][j]);
    }
    listbox->Set(resnum, res);
}

/****************************************************************/
/*  BENJAMINS */
/****************************************************************/

BenjFrame::BenjFrame(wxFrame* parent, Game& iGame, wxListCtrl* _results):
    AuxFrameList(parent, ID_Frame_Benj, wxT("benjamins"), FRAMEBENJ),
    m_game(iGame)
{
    results = _results;
}

void
BenjFrame::Refresh(refresh_t force)
{
    int i;
    char wordlist[RES_BENJ_MAX][DIC_WORD_MAX];
    long item = -1;
    item = results->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    // item can be -1

    if (item < 0)
    {
        listbox->Clear();
        return;
    }

    Waiting();
    Dic_search_Benj(m_game.getDic(),
		    ((Training&)m_game).getSearchedWord(item).c_str(), wordlist);

    int resnum = 0;
    wxString res[RES_BENJ_MAX];
    for (i = 0; (i < RES_BENJ_MAX) && (wordlist[i][0]); i++)
        res[resnum++] = wxU(wordlist[i]);
    listbox->Set(resnum, res);
}


/****************************************************************/
/* RACC FRAME */
/****************************************************************/

RaccFrame::RaccFrame(wxFrame* parent, Game& iGame, wxListCtrl* _results):
    AuxFrameList(parent, ID_Frame_Racc, wxT("raccords"), FRAMERACC),
    m_game(iGame)
{
    results = _results;
}

void
RaccFrame::Refresh(refresh_t force)
{
    int i;
    char wordlist[RES_RACC_MAX][DIC_WORD_MAX];
    long item = -1;
    item = results->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    // item can be -1

    if (item < 0)
    {
        listbox->Clear();
        return;
    }

    Waiting();
    Dic_search_Racc(m_game.getDic(),
		    ((Training&)m_game).getSearchedWord(item).c_str(), wordlist);

    int resnum = 0;
    wxString res[RES_RACC_MAX];
    for (i = 0; (i < RES_RACC_MAX) && (wordlist[i][0]); i++)
    {
        res[resnum++] = wxU(wordlist[i]);
    }
    listbox->Set(resnum, res);
}


/****************************************************************/
/****************************************************************/
