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

/* $Id: searchpanel.cc,v 1.7 2005/04/19 16:21:15 afrab Exp $ */

#include <string.h>
#include "wx/panel.h"
#include "wx/textctrl.h"
#include "wx/listbox.h"
#include "wx/sizer.h"
#include "wx/intl.h"

#include "ewx.h"
#include "dic.h"
#include "dic_search.h"
#include "regexp.h"
#include "searchpanel.h"
#include "tile.h"
#include "configdb.h"

enum {
  ID_PANEL_CROSS,
  ID_PANEL_PLUS1,
  ID_PANEL_REGEXP,

  ID_LIST,
  ID_TEXT
};

// ************************************************************
// ************************************************************
// ************************************************************

class SimpleSearchPanel : public wxPanel
{
protected:
  ConfigDB config;
  Dictionary dic_;
  wxTextCtrl* t;
  wxListBox* l;
  int check();
public:
  SimpleSearchPanel(wxWindow* parent, int id, Dictionary dic);
  virtual void compute_char(wxCommandEvent&) {};
  virtual void compute_enter(wxCommandEvent&) {};
  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(SimpleSearchPanel, wxPanel)
  EVT_TEXT      (ID_TEXT, SimpleSearchPanel::compute_char)
  EVT_TEXT_ENTER(ID_TEXT, SimpleSearchPanel::compute_enter)
END_EVENT_TABLE()

SimpleSearchPanel::SimpleSearchPanel(wxWindow* parent, int id, Dictionary dic)
  : wxPanel(parent,id)
{
  dic_ = dic;

  t = new wxTextCtrl(this,ID_TEXT,wxT(""),wxPoint(0,0),wxSize(-1,-1),wxTE_PROCESS_ENTER);
  t->SetFont(config.getFont(LISTFONT));
  l = new wxListBox(this,ID_LIST);
  l->SetFont(config.getFont(LISTFONT));
  l->Show(TRUE);

  wxBoxSizer *sizer = new wxBoxSizer( wxVERTICAL );
  sizer->Add(t, 0, wxEXPAND | wxALL, 0);
  sizer->Add(l, 1, wxEXPAND | wxALL, 0);

  SetAutoLayout(TRUE);
  SetSizer(sizer);
  sizer->Fit(this);
  sizer->SetSizeHints(this);
}

int
SimpleSearchPanel::check()
{
  wxString msg = wxT("");
  if (dic_ == NULL)
    {
      l->Clear();
      msg << wxT("Pas de dictionnaire");
      l->Append(msg);
      return 0;
    }
  return 1;
}


// ************************************************************
// ************************************************************
// ************************************************************

class PCross : public SimpleSearchPanel
{
private:
public:
  void compute_char(wxCommandEvent&) { };
  void compute_enter(wxCommandEvent&);
  PCross(wxWindow* parent, int id, Dictionary dic) : SimpleSearchPanel(parent,id,dic) {};
};

void
PCross::compute_enter(wxCommandEvent&)
{
  int  i;
  char rack[DIC_WORD_MAX];
  char buff[RES_CROS_MAX][DIC_WORD_MAX];

  if (!check())
    return;

  if (t->GetValue().Len() >= DIC_WORD_MAX)
    {
      wxString msg = wxT("");
      msg << wxT("La recherche est limitée à ") << DIC_WORD_MAX - 1 << wxT(" lettres");
      l->Append(msg);
      return;
    }

  strncpy(rack, t->GetValue().mb_str(), DIC_WORD_MAX);
  Dic_search_Cros(dic_,rack,buff);

  int resnum = 0;
  wxString res[RES_CROS_MAX];
  for(i=0; i < RES_CROS_MAX && buff[i][0]; i++)
    res[resnum++] =  wxU(buff[i]);
  l->Set(resnum,res);

  if (l->GetCount() == 0)
    l->Append(wxT("Aucun résultat"));
}

// ************************************************************
// ************************************************************
// ************************************************************

class PPlus1 : public SimpleSearchPanel
{
private:
public:
  void compute_char(wxCommandEvent&) { };
  void compute_enter(wxCommandEvent&);
  PPlus1(wxWindow* parent, int id, Dictionary dic) : SimpleSearchPanel(parent,id,dic) {};
};

void
PPlus1::compute_enter(wxCommandEvent&)
{
  int  i,j;
  char rack[DIC_WORD_MAX];
  char buff[DIC_LETTERS][RES_7PL1_MAX][DIC_WORD_MAX];

  if (!check())
    return;

  if (t->GetValue().Len() >= DIC_WORD_MAX)
    {
      wxString msg = wxT("");
      msg << wxT("La recherche est limitée à ") << DIC_WORD_MAX - 1 << wxT(" lettres");
      l->Append(msg);
      return;
    }

  strncpy(rack, t->GetValue().mb_str(), DIC_WORD_MAX);
  Dic_search_7pl1(dic_,rack,buff,TRUE);

  int resnum = 0;
  wxString res[DIC_LETTERS*(RES_7PL1_MAX+1)];
  for(i=0; i < DIC_LETTERS; i++)
    {
      if (i && buff[i][0][0])
          res[resnum++] = wxString(wxT("+")) + (wxChar)(i+'A'-1);
      for(j=0; j < RES_7PL1_MAX && buff[i][j][0]; j++)
          res[resnum++] = wxString(wxT("  ")) + wxU(buff[i][j]);
    }
  l->Set(resnum,res);

  if (l->GetCount() == 0)
    l->Append(wxT("Aucun résultat"));
}

// ************************************************************
// ************************************************************
// ************************************************************

class PRegExp : public SimpleSearchPanel
{
private:
  struct search_RegE_list_t llist;
  void build_letter_lists();
public:
  void compute_char(wxCommandEvent&) { };
  void compute_enter(wxCommandEvent&);
  PRegExp(wxWindow* parent, int id, Dictionary dic) : SimpleSearchPanel(parent,id,dic) {};
};

void
PRegExp::build_letter_lists()
{
  list<Tile> all_tiles;

  llist.symbl[0] = RE_ALL_MATCH;
  llist.symbl[1] = RE_VOWL_MATCH;
  llist.symbl[2] = RE_CONS_MATCH;
  llist.symbl[3] = RE_USR1_MATCH;
  llist.symbl[5] = RE_USR2_MATCH;

  llist.valid[0] = 1; // all letters
  llist.valid[1] = 1; // vowels
  llist.valid[2] = 1; // consonants
  llist.valid[3] = 0; // user defined list 1
  llist.valid[4] = 0; // user defined list 2

  const list<Tile>& allTiles = Tile::getAllTiles();
  list<Tile>::const_iterator it;
  for (it = allTiles.begin(); it != allTiles.end(); it++)
    {
      if (! it->isJoker() && ! it->isEmpty())
	{
	  llist.letters[0][it->toCode()] = 1;

	  if (it->isVowel())
	    {
	      llist.letters[1][it->toCode()] = 1;
	    }

	  if (it->isConsonant())
	    {
	      llist.letters[2][it->toCode()] = 1;
	    }
	}
    }
}

void
PRegExp::compute_enter(wxCommandEvent&)
{
  int  i;
  char re[DIC_WORD_MAX];
  char buff[RES_REGE_MAX][DIC_WORD_MAX];

  if (!check())
    return;

  build_letter_lists();
  strncpy(re, t->GetValue().mb_str(),DIC_WORD_MAX);
  Dic_search_RegE(dic_,re,buff,&llist);

  int resnum = 0;
  wxString res[RES_REGE_MAX];
  for(i=0; i < RES_REGE_MAX && buff[i][0]; i++)
    res[resnum++] =  wxU(buff[i]);

  l->Set(resnum,res);

  if (l->GetCount() == 0)
    l->Append(wxT("Aucun résultat"));
}

// ************************************************************
// ************************************************************
// ************************************************************

SearchPanel::SearchPanel(wxFrame *parent, Dictionary dic) :
  wxNotebook(parent, -1)
{
  AddPage(new PCross(this,ID_PANEL_CROSS,dic),wxT("Mots croisés"));
  AddPage(new PPlus1(this,ID_PANEL_PLUS1,dic),wxT("Plus 1"));
  AddPage(new PRegExp(this,ID_PANEL_REGEXP,dic),wxT("Exp. Rationnelle"));
  SetSelection(0);
}

SearchPanel::~SearchPanel()
{
}
