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

#include "wx/sizer.h"
#include "wx/button.h"
#include "wx/intl.h"

#include "wx/clipbrd.h"
#include "wx/dataobj.h"

#include "ewx.h"

#include "dic.h"
#include "dic_search.h"
#include "training.h"
#include "player.h"
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
    //debug("  %s::Reload() - %s\n",(const char*)classname.mb_str(),(const char*)name.mb_str());

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
    //debug("    BoardFrame::Refresh\n");
    if (force == REFRESH)
        board->Refresh(GfxBoard::BOARD_REFRESH);
    else
        board->Refresh(GfxBoard::BOARD_FORCE_REFRESH);
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
    //tiles->SetColumnWidth(0, wxLIST_AUTOSIZE);
    //tiles->SetFont(config.getFont(LISTFONT));
    //tiles->SetToolTip(wxT("Lettre, nombre restant"));

    wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
    sizer->Add(tiles, 1, wxEXPAND | wxALL, 1);
    SetAutoLayout(TRUE);
    SetSizer(sizer);
    sizer->Fit(this);
}

void
BagFrame::Refresh(refresh_t force)
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

    std::list<Tile>::const_iterator it;
    const std::list<Tile>& allTiles = Tile::getAllTiles();
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
    //debug("    SearchFrame::Refresh\n");
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
// XXX:  AuxFrame(parent, ID_Frame_Verif, wxT("vérification"), FRAMEVERIF)
  AuxFrame(parent, ID_Frame_Verif, wxT("verification"), FRAMEVERIF)
{
    dic = _dic;
    word = new wxTextCtrl(this, Word_Id, wxT(""));
    word->SetFont(config.getFont(LISTFONT));
// XXX:    word->SetToolTip(wxT("Mot à vérifier"));
    word->SetToolTip(wxT("Mot a verifier"));
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
    savedword = "";

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

void
AuxFrameList::Refresh(refresh_t force)
{
    //debug("    %s : Refresh start\n",(const char*)name.mb_str());
    if (game == NULL)
	{
	    listbox->Clear();
	    listbox->Append(wxT("Pas de partie en cours"));
	    //debug("  %s : Refresh end - no game\n",(const char*)name.mb_str());
	    return;
	}
    if (game->getDic() == NULL)
	{
	    listbox->Clear();
	    listbox->Append(wxT("Pas de dictionnaire"));
	    //debug("  %s : Refresh end - no dictionnary\n",(const char*)name.mb_str());
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
	    listbox->Append(wxT("Aucun resultat"));
	}
    //debug("  %s : Refresh end\n",(const char*)name.mb_str());
}

/****************************************************************/
/* PLUS1 FRAME */
/****************************************************************/

void
Plus1Frame::refresh()
{
    std::string rack;
    //debug("      Plus1Frame::refresh start\n");
    rack = game->getCurrentPlayer().getCurrentRack().toString();
    //debug("         CurrentPlayer -> rack : %s\n",rack.c_str());

    if (savedword == rack)
	{
	    noresult = false; // keep old results
	    //debug("      Plus1Frame::refresh end, no change\n");
	    return;
	}
    savedword = rack;

    char buff[DIC_LETTERS][RES_7PL1_MAX][DIC_WORD_MAX];
    Dic_search_7pl1(game->getDic(), rack.c_str(), buff, config.getJokerPlus1());

    listbox->Clear();
    wxString res[DIC_LETTERS*(RES_7PL1_MAX+1)];
    int resnum = 0;
    res[resnum++] = wxString(wxT("Tirage: ")) + wxString(wxU(rack.c_str()));
    for (int i = 0; i < DIC_LETTERS; i++)
    {
        if (i && buff[i][0][0])
	    {
		res[resnum++] = wxString(wxT("+")) + (wxChar)(i + 'A' - 1);
		noresult = false;
	    }
	for (int j = 0; j < RES_7PL1_MAX && buff[i][j][0]; j++)
	    {
		res[resnum++] = wxString(wxT("  ")) + wxU(buff[i][j]);
		noresult = false;
	    }
    }
    listbox->Set(resnum, res);
    //debug("      Plus1Frame::refresh end\n");
}

/****************************************************************/
/*  BENJAMINS */
/****************************************************************/

void
BenjFrame::refresh()
{
    std::string word;

    if (game->getMode() != Game::kTRAINING)
	return;

    word = ((Training*)game)->getTestPlayWord();
    if (savedword == word)
	{
	    noresult = false; // keep old results
	    return;
	}
    savedword = word;
    //debug("   BenjFrame::refresh : %s\n",word.c_str());
    char wordlist[RES_BENJ_MAX][DIC_WORD_MAX];
    Dic_search_Benj(game->getDic(), word.c_str(), wordlist);

    wxString res[RES_BENJ_MAX];
    int resnum = 0;
    for (int i = 0; (i < RES_BENJ_MAX) && (wordlist[i][0]); i++)
	{
	    res[resnum++] = wxU(wordlist[i]);
	    //debug("      BenjFrame : %s (%d)\n",wordlist[i],resnum);
	    noresult = false;
	}
    listbox->Set(resnum, res);
}


/****************************************************************/
/* RACC FRAME */
/****************************************************************/

void
RaccFrame::refresh()
{
    std::string word;

    if (game->getMode() != Game::kTRAINING)
	return;

    word = ((Training*)game)->getTestPlayWord();
    if (savedword == word)
	{
	    noresult = false; // keep old results
	    return;
	}
    savedword = word;
    //debug("   RaccFrame::refresh : %s\n",word.c_str());
    char wordlist[RES_RACC_MAX][DIC_WORD_MAX];
    Dic_search_Racc(game->getDic(), word.c_str(), wordlist);

    wxString res[RES_RACC_MAX];
    int resnum = 0;
    for (int i = 0; (i < RES_RACC_MAX) && (wordlist[i][0]); i++)
	{
	    res[resnum++] = wxU(wordlist[i]);
	    //debug("      RaccFrame : %s (%d)\n",wordlist[i],resnum);
	    noresult = false;
	}
    listbox->Set(resnum, res);
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
    AuxFrameText(parent, ID_Frame_Game, wxT("partie"), FRAMEGAME, wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP),
    m_game(iGame)
{
    textbox->Clear();
    textbox->AppendText(wxT(""));
}

void
GameFrame::Refresh(refresh_t force)
{
    std::ostringstream mos;
    m_game.save(mos);
#ifdef DEBUG
    mos << std::string(30,'-') << std::endl;
    mos << "Player History\n";
    mos << m_game.getPlayer(0).toString();
    mos << std::string(30,'-') << std::endl;
    mos << "Game History\n";
    mos << m_game.getHistory().toString();
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
    AuxFrame(parent, ID_Frame_Result, wxT("recherche"), FRAMERESULT)
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
ResultFrame::OnSize(wxSizeEvent& e)
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
/// End:
