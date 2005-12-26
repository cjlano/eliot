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
 *  \file   searchpanel.cc
 *  \brief  Panel used in Eliot search window
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#include <string.h>
#include "wx/wx.h"

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
  ID_TEXT,
  ID_OPTION1,
  ID_OPTION2,
  ID_OPTION3
};

// ************************************************************
// ************************************************************
// ************************************************************

class SimpleSearchPanel : public wxPanel
{
protected:
  ConfigDB   config;
  Dictionary dic;
  wxTextCtrl *t;
  wxListBox  *l;
  wxBoxSizer *sizer;

  int  check_dic();
  void check_end();
  void panel_build();
  virtual void panel_options() = 0;
public:
  SimpleSearchPanel(wxWindow* parent, int id, Dictionary d) : wxPanel(parent,id) { dic = d; };
  virtual void compute_char(wxCommandEvent&) {};
  virtual void compute_enter(wxCommandEvent&) {};
  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(SimpleSearchPanel, wxPanel)
  EVT_TEXT      (ID_TEXT   , SimpleSearchPanel::compute_char)
  EVT_TEXT_ENTER(ID_TEXT   , SimpleSearchPanel::compute_enter)
  EVT_TEXT_ENTER(ID_OPTION1, SimpleSearchPanel::compute_enter)
  EVT_TEXT_ENTER(ID_OPTION2, SimpleSearchPanel::compute_enter)
  EVT_TEXT_ENTER(ID_OPTION3, SimpleSearchPanel::compute_enter)
END_EVENT_TABLE()

void SimpleSearchPanel::panel_build()
{
  t = new wxTextCtrl(this,ID_TEXT,wxT(""),wxPoint(0,0),wxSize(-1,-1),wxTE_PROCESS_ENTER);
  t->SetFont(config.getFont(LISTFONT));
  l = new wxListBox(this,ID_LIST);
  l->SetFont(config.getFont(LISTFONT));
  l->Show(TRUE);

  sizer = new wxBoxSizer( wxVERTICAL );
  sizer->Add(t, 0, wxEXPAND | wxALL, 0);
  panel_options();
  sizer->Add(l, 1, wxEXPAND | wxALL, 0);

  SetAutoLayout(TRUE);
  SetSizer(sizer);
  sizer->Fit(this);
  sizer->SetSizeHints(this);
}

int
SimpleSearchPanel::check_dic()
{
  wxString msg = wxT("");
  if (dic == NULL)
    {
      l->Clear();
      msg << wxT("Pas de dictionnaire");
      l->Append(msg);
      return 0;
    }
  return 1;
}

void
SimpleSearchPanel::check_end()
{
  if (l->GetCount() == 0)
    {
      l->Append(wxT("Aucun resultat"));
    }
}

// ************************************************************
// ************************************************************
// ************************************************************

class PCross : public SimpleSearchPanel
{
protected:
  virtual void panel_options() {};
public:
  void compute_char(wxCommandEvent&) { };
  void compute_enter(wxCommandEvent&);
  PCross(wxWindow* parent, int id, Dictionary d) : SimpleSearchPanel(parent,id,d) { panel_build(); };
};

void
PCross::compute_enter(wxCommandEvent&)
{
  int  i;
  char rack[DIC_WORD_MAX];
  char buff[RES_CROS_MAX][DIC_WORD_MAX];

  if (!check_dic())
    return;

  if (t->GetValue().Len() >= DIC_WORD_MAX)
    {
      wxString msg = wxT("");
// XXX:      msg << wxT("La recherche est limitée à ") << DIC_WORD_MAX - 1 << wxT(" lettres");
      msg << wxT("La recherche est limitee a ") << DIC_WORD_MAX - 1 << wxT(" lettres");
      l->Append(msg);
      return;
    }

  strncpy(rack, t->GetValue().mb_str(), DIC_WORD_MAX);
  Dic_search_Cros(dic,rack,buff);

  int resnum = 0;
  wxString res[RES_CROS_MAX];
  for(i=0; i < RES_CROS_MAX && buff[i][0]; i++)
    res[resnum++] =  wxU(buff[i]);
  l->Set(resnum,res);
  check_end();
}

// ************************************************************
// ************************************************************
// ************************************************************

class PPlus1 : public SimpleSearchPanel
{
protected:
  virtual void panel_options() {};
public:
  void compute_char(wxCommandEvent&) { };
  void compute_enter(wxCommandEvent&);
  PPlus1(wxWindow* parent, int id, Dictionary dic) : SimpleSearchPanel(parent,id,dic) { panel_build(); };
};

void
PPlus1::compute_enter(wxCommandEvent&)
{
  int  i,j;
  char rack[DIC_WORD_MAX];
  char buff[DIC_LETTERS][RES_7PL1_MAX][DIC_WORD_MAX];

  if (!check_dic())
    return;

  if (t->GetValue().Len() >= DIC_WORD_MAX)
    {
      wxString msg = wxT("");
// XXX:      msg << wxT("La recherche est limitée à ") << DIC_WORD_MAX - 1 << wxT(" lettres");
      msg << wxT("La recherche est limitee a ") << DIC_WORD_MAX - 1 << wxT(" lettres");
      l->Append(msg);
      return;
    }

  strncpy(rack, t->GetValue().mb_str(), DIC_WORD_MAX);
  Dic_search_7pl1(dic,rack,buff,TRUE);

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
  check_end();
}

// ************************************************************
// ************************************************************
// ************************************************************

class PRegExp : public SimpleSearchPanel
{
protected:
  wxTextCtrl                *omin;
  wxTextCtrl                *omax;
  struct search_RegE_list_t llist;

  virtual void build_letter_lists();
  virtual void panel_options();
public:
  void compute_char(wxCommandEvent&) { };
  void compute_enter(wxCommandEvent&);
  PRegExp(wxWindow* parent, int id, Dictionary d) : SimpleSearchPanel(parent,id,d) { panel_build(); };
};

void
PRegExp::build_letter_lists()
{
  int i;
  std::list<Tile> all_tiles;

  memset (&llist,0,sizeof(llist));

  llist.minlength = 1;
  llist.maxlength = 15;

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

  for(i=0; i < DIC_SEARCH_REGE_LIST; i++)
    {
      memset(llist.letters[i],0,sizeof(llist.letters[i]));
    }

  const std::list<Tile>& allTiles = Tile::getAllTiles();
  std::list<Tile>::const_iterator it;
  for (it = allTiles.begin(); it != allTiles.end(); it++)
    {
      if (! it->isJoker() && ! it->isEmpty())
	{
	  // all tiles
	  llist.letters[0][it->toCode()] = 1;
	  // vowels
	  if (it->isVowel())
	    {
	      llist.letters[1][it->toCode()] = 1;
	    }
	  // consonants
	  if (it->isConsonant())
	    {
	      llist.letters[2][it->toCode()] = 1;
	    }
	}
    }
}

void
PRegExp::panel_options()
{
  wxStaticText *otmin;
  wxStaticText *otmax;

  otmin = new wxStaticText(this,wxID_ANY,wxT("Longueur min."));
  omin  = new wxTextCtrl(this,ID_OPTION1,wxT( "1"),wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
  otmax = new wxStaticText(this,wxID_ANY,wxT("max."));
  omax  = new wxTextCtrl(this,ID_OPTION2,wxT("15"),wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);

  wxBoxSizer *s = new wxBoxSizer( wxHORIZONTAL );
  s->Add(otmin, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT | wxRIGHT, 4);
  s->Add(omin , 1, wxALIGN_CENTRE_VERTICAL, 0);
  s->Add(otmax, 0, wxALIGN_CENTRE_VERTICAL | wxLEFT | wxRIGHT, 4);
  s->Add(omax , 1, wxALIGN_CENTRE_VERTICAL, 0);
  sizer->Add(s, 0, wxEXPAND | wxALL, 1);
}


#define DIC_RE_MAX (3*DIC_WORD_MAX) // yes, it's 3

void
PRegExp::compute_enter(wxCommandEvent&)
{
  char re[DIC_RE_MAX];
  char buff[RES_REGE_MAX][DIC_WORD_MAX];

  if (!check_dic())
    return;

  build_letter_lists();
  strncpy(re, t->GetValue().mb_str(),DIC_RE_MAX);
  debug("PRegExp::compute_enter for %s",re);

  int lmin = atoi((const char*)omin->GetValue().mb_str());
  int lmax = atoi((const char*)omax->GetValue().mb_str());
  if (lmax <= (DIC_WORD_MAX - 1) && lmin >= 1 && lmin <= lmax)
    {
      llist.minlength = lmin;
      llist.maxlength = lmax;
      debug(" length %d,%d",lmin,lmax);
    }
  else
    {
      debug(" bad length -%s,%s-",
	    (const char*)omin->GetValue().mb_str(),
	    (const char*)omax->GetValue().mb_str());
    }
  debug("\n");

  Dic_search_RegE(dic,re,buff,&llist);

  int resnum = 0;
  wxString res[RES_REGE_MAX];
  for(int i=0; i < RES_REGE_MAX && buff[i][0]; i++)
    res[resnum++] =  wxU(buff[i]);

  l->Set(resnum,res);
  check_end();
}

// ************************************************************
// ************************************************************
// ************************************************************

SearchPanel::SearchPanel(wxFrame *parent, Dictionary dic) :
  wxNotebook(parent, -1)
{
  AddPage(new PCross (this,ID_PANEL_CROSS ,dic),wxT("Mots croises"));
  AddPage(new PPlus1 (this,ID_PANEL_PLUS1 ,dic),wxT("Plus 1"));
  AddPage(new PRegExp(this,ID_PANEL_REGEXP,dic),wxT("Exp. Rationnelle"));
  SetSelection(2);
}

SearchPanel::~SearchPanel()
{
}

// ************************************************************
// ************************************************************
// ************************************************************


/// Local Variables:
/// mode: hs-minor
/// c-basic-offset: 4
/// End:
