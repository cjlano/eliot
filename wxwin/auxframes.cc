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
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

/* $Id: auxframes.cc,v 1.2 2004/06/20 20:00:09 afrab Exp $ */

#include <iostream>
using namespace std;

#include "ewx.h"

#include "dic.h"
#include "dic_search.h"
#include "game.h"

#include "configdb.h"
#include "auxframes.h"
#include "mainframe.h"
#include "searchpanel.h"

#include "wx/sizer.h"
#include "wx/button.h"
#include "wx/intl.h"

#include "wx/clipbrd.h"
#include "wx/dataobj.h"

/****************************************************************/
/* AUXFRAME */
/****************************************************************/
  
AuxFrame::AuxFrame(wxFrame* parent, int _id, wxString _name, wxString _classname):
  wxFrame(parent, -1, wxString("Eliot: ") + _name, wxPoint(-1,-1), wxSize(-1,-1),
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
  config.setFramePos(classname,p);
  config.setFrameSize(classname,s);
  config.setFrameShow(classname,show);
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
  if (show) { Show(FALSE); Show(TRUE); }
}

/****************************************************************/
/* BOARD FRAME */
/****************************************************************/

BoardFrame::BoardFrame(wxFrame* parent, Game _game):
  AuxFrame(parent, ID_Frame_Board, "Grille", FRAMEBOARD)
{
  board = new GfxBoard(this,_game);

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

BagFrame::BagFrame(wxFrame* parent, Game _game):
  AuxFrame(parent, ID_Frame_Bag, "sac", FRAMEBAG)
{
  game = _game;
  tiles = new wxListCtrl(this,-1);
  tiles->SetSingleStyle(wxLC_LIST);
  tiles->SetColumnWidth(0,wxLIST_AUTOSIZE);
  tiles->SetFont(config.getFont(LISTFONT));
  tiles->SetToolTip("Lettre, nombre restant");

  wxBoxSizer *sizer = new wxBoxSizer( wxHORIZONTAL );
  sizer->Add(tiles, 1, wxEXPAND | wxALL, 1);
  SetAutoLayout(TRUE);
  SetSizer(sizer);
  sizer->Fit(this);
}

void
BagFrame::Refresh(refresh_t force)
{
  char c;
  wxString buf;
  wxChar format[] = "%c:%2d";

  tiles->ClearAll();
  
  buf.Printf(format,'?',Game_getcharinbag(game,'?'),0);
  tiles->InsertItem(0,buf);
  
  for(c = 'A'; c <= 'Z'; c++) 
    {
      buf.Printf(format,c,Game_getcharinbag(game,c));
      tiles->InsertItem(1 + c - 'A',buf);
    }
}

/****************************************************************/
/* RECHERCHE */
/****************************************************************/

SearchFrame::SearchFrame(wxFrame *parent, Dictionary _dic):
  AuxFrame(parent,ID_Frame_Search,"recherche",FRAMESEARCH)
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
  AuxFrame(parent, ID_Frame_Verif, "vérification",FRAMEVERIF)
{
  dic = _dic;
  word = new wxTextCtrl(this,Word_Id,wxString(""));
  word->SetFont(config.getFont(LISTFONT));
  word->SetToolTip("Mot à vérifier");
  result = new wxStaticText(this,Result_Id,wxString(""));
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
      result->SetLabel(wxString("pas de dictionnaire"));
      return;
    }
  if (Dic_search_word(dic,(const char*) word->GetValue()))
    result->SetLabel(wxString("existe"));
  else
    result->SetLabel(wxString("n'existe pas"));
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
  EVT_BUTTON        (ButtonCopyID ,AuxFrameList::OnCopy)
END_EVENT_TABLE()

AuxFrameList::AuxFrameList(wxFrame* parent, int _id, wxString _name, wxString _classname):
  AuxFrame(parent, _id, _name, _classname)

{
  wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
  listbox = new wxListBox(this,ListBoxID);
  listbox->SetFont(config.getFont(LISTFONT));
  listbox->SetToolTip(name);
  sizer_v->Add(listbox, 1, wxEXPAND | wxALL, 1);

  button = new wxButton(this,ButtonCopyID,"Copier",wxPoint(0,0),wxSize(-1,-1));
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
      textdata = "";
      for(int i=0; i < listbox->Number(); i++)
	{
	  textdata << listbox->GetString(i) << "\n";
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

Plus1Frame::Plus1Frame(wxFrame* parent, Game _game):
  AuxFrameList(parent,ID_Frame_Plus1,wxString("Tirage + 1"),FRAMEPLUS1) 
{
  rack[0] = '\0';
  game = _game;
}

void
Plus1Frame::Refresh(refresh_t force)
{
  int  i,j;
  char rack2[RACK_SIZE_MAX];
  char buff[LETTERS][RES_7PL1_MAX][DIC_WORD_MAX];

  if (Game_getdic(game) == NULL)
    return;
  
  Game_getplayedrack(game,Game_getnrounds(game),rack2);
  
  if (strcmp(rack,rack2) == 0)
    return;

  strcpy(rack,rack2);

  Waiting();
  Dic_search_7pl1(Game_getdic(game),rack,buff,config.getJokerPlus1()); 

  int resnum = 0;
  wxString res[LETTERS*(RES_7PL1_MAX+1)];
  res[resnum++] = wxString("Tirage: ") + wxString(rack);
  for(i=0; i < LETTERS; i++)
    {
      if (i && buff[i][0][0])
	res[resnum++] = wxString("+") + wxString((char)(i+'A'-1));
      for(j=0; j < RES_7PL1_MAX && buff[i][j][0]; j++)
	res[resnum++] = wxString("  ") + wxString(buff[i][j]);
    }
  listbox->Set(resnum,res);
}

/****************************************************************/
/*  BENJAMINS */
/****************************************************************/

BenjFrame::BenjFrame(wxFrame* parent,Game _game, wxListCtrl* _results):
  AuxFrameList(parent,ID_Frame_Benj,wxString("benjamins"), FRAMEBENJ) 
{
  game = _game;
  results = _results;
}

void
BenjFrame::Refresh(refresh_t force)
{
  int i;
  char word[WORD_SIZE_MAX + 1];
  char wordlist[RES_BENJ_MAX][DIC_WORD_MAX];
  long item = -1;
  item = results->GetNextItem(item,wxLIST_NEXT_ALL,wxLIST_STATE_SELECTED);
  // item can be -1

  if (item < 0)
    {
      listbox->Clear();
      return;
    }

  Waiting();
  Game_getsearchedword(game,item,word);
  if (Game_getdic(game) == NULL)
    return;
  Dic_search_Benj(Game_getdic(game),word,wordlist);

  int resnum = 0;
  wxString res[RES_BENJ_MAX];
  for(i=0; (i < RES_BENJ_MAX) && (wordlist[i][0]); i++)
    res[resnum++] = wxString(wordlist[i]);
  listbox->Set(resnum,res);
}


/****************************************************************/
/* RACC FRAME */
/****************************************************************/

RaccFrame::RaccFrame(wxFrame* parent,Game _game, wxListCtrl* _results):
  AuxFrameList(parent,ID_Frame_Racc,wxString("raccords"), FRAMERACC)
{
  game = _game;
  results = _results;
}

void
RaccFrame::Refresh(refresh_t force)
{
  int i;
  char word[WORD_SIZE_MAX + 1];
  char wordlist[RES_RACC_MAX][DIC_WORD_MAX];
  long item = -1;
  item = results->GetNextItem(item,wxLIST_NEXT_ALL,wxLIST_STATE_SELECTED);
  // item can be -1

  if (item < 0)
    {
      listbox->Clear();
      return;
    }

  Waiting();
  Game_getsearchedword(game,item,word);
  if (Game_getdic(game) == NULL)
    return;
  Dic_search_Racc(Game_getdic(game),word,wordlist);

  int resnum = 0;
  wxString res[RES_RACC_MAX];
  for(i=0; (i < RES_RACC_MAX) && (wordlist[i][0]); i++)
    {
      res[resnum++] = wxString(wordlist[i]);
    }
  listbox->Set(resnum,res);
}


/****************************************************************/
/****************************************************************/
