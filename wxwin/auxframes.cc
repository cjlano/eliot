/* Eliot                                                                     */
/* Copyright (C) 1999  Antoine Fraboulet                                     */
/*                                                                           */
/* This file is part of Eliot.                                               */
/*                                                                           */
/* Eliot is free software; you can redistribute it and/or modify             */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 2 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* Eliot is distributed in the hope that it will be useful,                  */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

/**
 *  \file   auxframes.cc
 *  \brief  Window Frames used in Eliot
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#include <iostream>
#include <sstream>
#include <list>
#include <string>

#include "wx/sizer.h"
#include "wx/button.h"
#include "wx/intl.h"

#include "wx/clipbrd.h"
#include "wx/dataobj.h"

#include "ewx.h"

#include "dic.h"
#include "training.h"
#include "player.h"
#include "game.h"
#include "encoding.h"

#include "configdb.h"
#include "auxframes.h"
#include "mainframe.h"
#include "searchpanel.h"

/****************************************************************/
/* AUXFRAME */
/****************************************************************/

AuxFrame::AuxFrame(wxFrame* parent, int _id, wxString _name, wxString _classname):
    wxFrame(parent, -1, wxT("Eliot: ") + _name, wxPoint(-1, -1), wxSize(-1, -1),
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
        Reload();
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
    /* debug("  %s::Reload() - %s\n",(const char*)classname.mb_str(),(const char*)name.mb_str()); */

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
    AuxFrame(parent, ID_Frame_Board, _("Grid"), FRAMEBOARD)
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
        board->Refresh(GfxBoard::BOARD_REFRESH);
    else
        board->Refresh(GfxBoard::BOARD_FORCE_REFRESH);
}

/****************************************************************/
/* BAG FRAME */
/****************************************************************/

BagFrame::BagFrame(wxFrame* parent, Game& iGame):
    AuxFrame(parent, ID_Frame_Bag, _("Bag"), FRAMEBAG),
    m_game(iGame)
{
    tiles = new wxListCtrl(this, -1);
    tiles->SetSingleStyle(wxLC_LIST);
    //tiles->SetColumnWidth(0, wxLIST_AUTOSIZE);
    //tiles->SetFont(config.getFont(LISTFONT));
    //tiles->SetToolTip(wxT("Lettre, nombre restant"));

#ifdef DEBUG
    wxFont font(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                wxFONTWEIGHT_NORMAL, false, wxString(wxT("Courier New")), wxFONTENCODING_SYSTEM);
    tiles->SetFont(font);
#endif

    wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
    sizer->Add(tiles, 1, wxEXPAND | wxALL, 1);
    SetAutoLayout(TRUE);
    SetSizer(sizer);
    sizer->Fit(this);
}

void
BagFrame::Refresh(refresh_t __UNUSED__ force)
{ 
    //debug("    BagFrame::Refresh\n");
    int n,index;
    wxString buf;
#ifdef DEBUG
    wxChar format[] = wxT("%c:%2d[%2d]");
#else
    wxChar format[] = wxT("%c:%2d");
#endif

    tiles->ClearAll();

    std::vector<Tile>::const_iterator it;
    const std::vector<Tile>& allTiles = m_game.getDic().getAllTiles();
    for (index = 0, it = allTiles.begin(); it != allTiles.end(); index++, it++)
    {
        n = m_game.getBag().in(*it);
#ifdef DEBUG
        buf.Printf(format, it->toChar(), n, n - it->maxNumber());
#else
        buf.Printf(format, it->toChar(), n);
#endif
        tiles->InsertItem(index,buf);
    }
}

/****************************************************************/
/* RECHERCHE */
/****************************************************************/

SearchFrame::SearchFrame(wxFrame *parent, const Dictionary &_dic):
    AuxFrame(parent, ID_Frame_Search, _("Search"), FRAMESEARCH)
{
    panel = new SearchPanel(this, _dic);
    wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
    sizer->Add(panel, 1, wxEXPAND, 0);
    SetAutoLayout(TRUE);
    SetSizer(sizer);
    sizer->Fit(this);
}

void
SearchFrame::Refresh(refresh_t __UNUSED__ force)
{
    //debug("    SearchFrame::Refresh\n");
}

/****************************************************************/
/* VERIF FRAME */
/****************************************************************/

enum
{
    Word_Id,
    Result_Id
};

BEGIN_EVENT_TABLE(VerifFrame, AuxFrame)
  EVT_TEXT(Word_Id, VerifFrame::OnText)
END_EVENT_TABLE()

VerifFrame::VerifFrame(wxFrame* parent, const Dictionary &_dic):
  AuxFrame(parent, ID_Frame_Verif, _("Check"), FRAMEVERIF)
{
    dic = &_dic;
    word = new wxTextCtrl(this, Word_Id, wxT(""));
    word->SetFont(config.getFont(LISTFONT));
    word->SetToolTip(_("Word to check"));
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
        result->SetLabel(_("No dictionary"));
        return;
    }
    if (dic->searchWord(word->GetValue().wc_str()))
        result->SetLabel(_("exists"));
    else
        result->SetLabel(_("doesn't exist"));
}

void
VerifFrame::OnText(wxCommandEvent&)
{
    verif();
}

void
VerifFrame::Refresh(refresh_t __UNUSED__ force)
{
    //debug("    VerifFrame::Refresh\n");
}

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

AuxFrameList::AuxFrameList(wxFrame* parent, int _id, wxString _name, wxString _classname, Game *g):
  AuxFrame(parent, _id, _name, _classname)

{
    game = g;
    savedword = L"";

    wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
    listbox = new wxListBox(this, ListBoxID);
    listbox->SetFont(config.getFont(LISTFONT));
    listbox->SetToolTip(name);
    sizer_v->Add(listbox, 1, wxEXPAND | wxALL, 1);

    button = new wxButton(this, ButtonCopyID, _("Copy"), wxPoint(0, 0), wxSize(-1, -1));
    sizer_v->Add(button, 0, wxEXPAND | wxALL, 1);

    wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
    sizer->Add(sizer_v, 1, wxEXPAND, 0);

    SetAutoLayout(TRUE);
    SetSizer(sizer);
    sizer->Fit(this);
    sizer->SetSizeHints(this);
}

void
AuxFrameList::OnCopy(wxCommandEvent __UNUSED__ &event)
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

void
AuxFrameList::Refresh(refresh_t __UNUSED__ force)
{
    //debug("    %s : Refresh start\n",(const char*)name.mb_str());
    if (game == NULL)
    {
        listbox->Clear();
        listbox->Append(_("No on going game"));
        //debug("  %s : Refresh end - no game\n",(const char*)name.mb_str());
        return;
    }
    if (show == 0)
    {
        //debug("  %s : Refresh end - no window\n",(const char*)name.mb_str());
        return;
    }
    noresult = true;
    refresh();
    if (noresult == true)
    {
        //debug("      %s : noresult == true\n",(const char*)name.mb_str());
        listbox->Clear();
        listbox->Append(_("No result"));
    }
    //debug("  %s : Refresh end\n",(const char*)name.mb_str());
}

/****************************************************************/
/* PLUS1 FRAME */
/****************************************************************/

Plus1Frame::Plus1Frame(wxFrame* p, Game* g)
    : AuxFrameList(p, ID_Frame_Plus1, _("Rack + 1"), FRAMEPLUS1, g)
{
}


void
Plus1Frame::refresh()
{
    //debug("      Plus1Frame::refresh start\n");
    std::wstring rack = game->getCurrentPlayer().getCurrentRack().toString();
    //debug("         CurrentPlayer -> rack : %s\n",rack.c_str());

    if (savedword == rack)
    {
        noresult = false; // keep old results
        //debug("      Plus1Frame::refresh end, no change\n");
        return;
    }
    savedword = rack;

    map<wchar_t, list<wstring> > wordList;
    game->getDic().search7pl1(rack, wordList, config.getJokerPlus1());

    // Count the results
    int sum = 0;
    map<wchar_t, list<wstring> >::const_iterator it;
    for (it = wordList.begin(); it != wordList.end(); it++)
    {
        if (it->first)
            sum += 1;
        sum += it->second.size();
    }
    // For the line containing the rack
    sum += 1;

    noresult = (sum == 0);
    listbox->Clear();
    if (noresult)
        return;

    wxString *res = new wxString[sum];
    int resnum = 0;
    res[resnum++] = wxString(_("Rack: ")) + wxString(wxU(rack.c_str()));
    for (it = wordList.begin(); it != wordList.end(); it++)
    {
        if (it->first)
            res[resnum++] = wxString(wxT("+")) + wxU((wxString)it->first);
        list<wstring>::const_iterator itWord;
        for (itWord = it->second.begin(); itWord != it->second.end(); itWord++)
        {
            res[resnum++] = wxString(wxT("  ")) + wxU(itWord->c_str());
        }
    }
    listbox->Set(resnum, res);
    delete[] res;
    //debug("      Plus1Frame::refresh end\n");
}

/****************************************************************/
/*  BENJAMINS */
/****************************************************************/

BenjFrame::BenjFrame(wxFrame* p, Game* g)
    : AuxFrameList(p, ID_Frame_Benj, _("Benjamins"), FRAMEBENJ, g)
{
}


void
BenjFrame::refresh()
{
    if (game->getMode() != Game::kTRAINING)
        return;

    wstring word = static_cast<Training*>(game)->getTestPlayWord();
    if (savedword == word)
    {
        noresult = false; // keep old results
        return;
    }
    savedword = word;
    //debug("   BenjFrame::refresh : %s\n",word.c_str());
    list<wstring> wordList;
    game->getDic().searchBenj(word, wordList);

    wxString *res = new wxString[wordList.size()];
    int resnum = 0;
    list<wstring>::const_iterator it;
    for (it = wordList.begin(); it != wordList.end(); it++)
    {
        res[resnum++] = wxU(it->c_str());
        //debug("      BenjFrame : %s (%d)\n",wordlist[i],resnum);
        noresult = false;
    }
    listbox->Set(resnum, res);
    delete[] res;
}


/****************************************************************/
/* RACC FRAME */
/****************************************************************/

RaccFrame::RaccFrame(wxFrame* p, Game* g)
    : AuxFrameList(p, ID_Frame_Racc, _("Extensions"), FRAMERACC, g)
{
}


void
RaccFrame::refresh()
{
    if (game->getMode() != Game::kTRAINING)
        return;

    wstring word = static_cast<Training*>(game)->getTestPlayWord();
    if (savedword == word)
    {
        noresult = false; // keep old results
        return;
    }
    savedword = word;
    //debug("   RaccFrame::refresh : %s\n",word.c_str());
    list<wstring> wordList;
    game->getDic().searchRacc(word, wordList);

    wxString *res = new wxString[wordList.size()];
    int resnum = 0;
    list<wstring>::const_iterator it;
    for (it = wordList.begin(); it != wordList.end(); it++)
    {
        res[resnum++] = wxU(it->c_str());
        //debug("      RaccFrame : %s (%d)\n",wordlist[i],resnum);
        noresult = false;
    }
    listbox->Set(resnum, res);
    delete[] res;
}

/****************************************************************/
/* AUXFRAMETEXT */
/****************************************************************/

AuxFrameText::AuxFrameText(wxFrame* parent, int _id, wxString _name, wxString _classname, int _style):
  AuxFrame(parent, _id, _name, _classname)

{
    wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );

    wxFont font(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                wxFONTWEIGHT_NORMAL, false, wxString(wxT("Courier New")), wxFONTENCODING_SYSTEM);

    textbox = new wxTextCtrl(this, -1, wxT(""), wxDefaultPosition, wxDefaultSize, _style);
    textbox->SetFont(font);

    sizer_v->Add(textbox, 1, wxEXPAND | wxALL, 1);
    sizer->Add(sizer_v, 1, wxEXPAND, 0);

    SetAutoLayout(TRUE);
    SetSizer(sizer);
    sizer->Fit(this);
    sizer->SetSizeHints(this);
}

/****************************************************************/
/* GAME FRAME */
/****************************************************************/

GameFrame::GameFrame(wxFrame* parent, Game& iGame):
    AuxFrameText(parent, ID_Frame_Game, _("Game history"), FRAMEGAME, wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP),
    m_game(iGame)
{
    textbox->Clear();
    textbox->AppendText(wxT(""));
}

void
GameFrame::Refresh(refresh_t __UNUSED__ force)
{
    std::ostringstream mos;
    m_game.save(mos);
#ifdef DEBUG
    mos << std::string(30,'-') << std::endl;
    mos << "Player History\n";
    mos << convertToMb(m_game.getPlayer(0).toString());
    mos << std::string(30,'-') << std::endl;
    mos << "Game History\n";
    mos << convertToMb(m_game.getHistory().toString());
#endif
    textbox->Clear();
    textbox->AppendText( wxU( mos.str().c_str() ) );
}

/****************************************************************/
/* RESULT FRAME */
/****************************************************************/

BEGIN_EVENT_TABLE(ResultFrame, AuxFrame)
END_EVENT_TABLE()

ResultFrame::ResultFrame(wxFrame* parent, Game* iGame):
    AuxFrame(parent, ID_Frame_Result, _("Results"), FRAMERESULT)
{
    reslist = new GfxResult(this, (MainFrame*)parent, iGame);

    wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *sizer   = new wxBoxSizer(wxHORIZONTAL);

    sizer_v->Add(reslist, 1, wxEXPAND, 0);
    sizer->Add  (sizer_v, 1, wxEXPAND | wxALL, 2);

    SetAutoLayout(TRUE);
    SetSizer(sizer);
    sizer->Fit(this);
    sizer->SetSizeHints(this);
    //debug("ResultFrame created\n");
}

void
ResultFrame::Refresh(refresh_t WXUNUSED(force))
{
    if (reslist != NULL)
    {
        reslist->Show(false);
        //debug("ResultFrame refresh\n");
        reslist->Refresh();
        reslist->Show(true);
    }
}

void
ResultFrame::Search()
{
    if (reslist != NULL)
    {
        reslist->Search();
    }
}

int
ResultFrame::GetSelected()
{
    if (reslist != NULL)
    {
        return reslist->GetSelected();
    }
    return -1;
}

void
ResultFrame::OnSize(wxSizeEvent __UNUSED__ &e)
{
    int w,h;
    GetClientSize(&w,&h);
    //debug("ResultFrame::OnSize (%d,%d)\n",w,h);
}

/****************************************************************/
/****************************************************************/

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
