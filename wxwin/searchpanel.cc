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
#include "dic_exception.h"
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
  const Dictionary *dic;
  wxTextCtrl *t;
  wxListBox  *l;
  wxBoxSizer *sizer;

  int  check_dic();
  void check_end();
  void panel_build();
  virtual void panel_options() = 0;
public:
  SimpleSearchPanel(wxWindow* parent, int id, const Dictionary &d) : wxPanel(parent,id) { dic = &d; }
  virtual void compute_char(wxCommandEvent&) {}
  virtual void compute_enter(wxCommandEvent&) {}
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
      msg << _("No dictionary");
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
      l->Append(_("No result"));
    }
}

// ************************************************************
// ************************************************************
// ************************************************************

class PCross : public SimpleSearchPanel
{
protected:
  virtual void panel_options() {}
public:
  void compute_char(wxCommandEvent&) {}
  void compute_enter(wxCommandEvent&);
  PCross(wxWindow* parent, int id, const Dictionary &d) : SimpleSearchPanel(parent,id,d) { panel_build(); }
};

void
PCross::compute_enter(wxCommandEvent&)
{
    if (!check_dic())
        return;

    if (t->GetValue().Len() >= DIC_WORD_MAX)
    {
        wxString msg;
        msg.Printf(_("The search is limited to %d letters"), DIC_WORD_MAX - 1);
        l->Append(msg);
        return;
    }

    wchar_t rack[DIC_WORD_MAX];
    wcsncpy(rack, t->GetValue().wc_str(), DIC_WORD_MAX);

    vector<wstring> wordList;
    dic->searchCross(rack, wordList);

    int resnum = 0;
    wxString *res = new wxString[wordList.size()];
    vector<wstring>::const_iterator it;
    for (it = wordList.begin(); it != wordList.end(); it++)
        res[resnum++] =  wxU(it->c_str());
    l->Set(resnum,res);
    delete[] res;
    check_end();
}

// ************************************************************
// ************************************************************
// ************************************************************

class PPlus1 : public SimpleSearchPanel
{
protected:
  virtual void panel_options() {}
public:
  void compute_char(wxCommandEvent&) {}
  void compute_enter(wxCommandEvent&);
  PPlus1(wxWindow* parent, int id, const Dictionary &dic) : SimpleSearchPanel(parent,id,dic) { panel_build(); }
};

void
PPlus1::compute_enter(wxCommandEvent&)
{
    if (!check_dic())
        return;

    if (t->GetValue().Len() >= DIC_WORD_MAX)
    {
        wxString msg;
        msg.Printf(_("The search is limited to %d letters"), DIC_WORD_MAX - 1);
        l->Append(msg);
        return;
    }

    wstring rack = t->GetValue().wc_str();
    map<wchar_t, vector<wstring> > wordList;
    dic->search7pl1(rack, wordList, true);

    // Count the results
    int sum = 0;
    map<wchar_t, vector<wstring> >::const_iterator it;
    for (it = wordList.begin(); it != wordList.end(); it++)
    {
        if (it->first)
            sum += 1;
        sum += it->second.size();
    }

    wxString *res = new wxString[sum];
    int resnum = 0;
    for (it = wordList.begin(); it != wordList.end(); it++)
    {
        if (it->first)
            res[resnum++] = wxString(wxT("+")) + wxU((wxString)it->first);
        vector<wstring>::const_iterator itWord;
        for (itWord = it->second.begin(); itWord != it->second.end(); itWord++)
        {
            res[resnum++] = wxString(wxT("  ")) + wxU(itWord->c_str());
        }
    }
    l->Set(resnum, res);
    delete[] res;
    check_end();
}

// ************************************************************
// ************************************************************
// ************************************************************

class PRegExp : public SimpleSearchPanel
{
private:
  wxTextCtrl                *omin;
  wxTextCtrl                *omax;

  void panel_options();
public:
  void compute_char(wxCommandEvent&) {}
  void compute_enter(wxCommandEvent&);
  PRegExp(wxWindow* parent, int id, const Dictionary &d) : SimpleSearchPanel(parent,id,d) { panel_build(); }
};

void
PRegExp::panel_options()
{
  wxStaticText *otmin;
  wxStaticText *otmax;

  otmin = new wxStaticText(this,wxID_ANY,_("Minimum length"));
  omin  = new wxTextCtrl(this,ID_OPTION1,wxT("1"),wxDefaultPosition,wxDefaultSize,wxTE_PROCESS_ENTER);
  otmax = new wxStaticText(this,wxID_ANY,_("Maximum length"));
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
    if (!check_dic())
        return;

    wstring regexp = t->GetValue().wc_str();
    debug("PRegExp::compute_enter for %ls", regexp.c_str());

    int lmin = atoi((const char*)omin->GetValue().mb_str());
    int lmax = atoi((const char*)omax->GetValue().mb_str());
    if (lmax <= (DIC_WORD_MAX - 1) && lmin >= 1 && lmin <= lmax)
    {
        debug(" length %d,%d",lmin,lmax);
    }
    else
    {
        debug(" bad length -%s,%s-",
              (const char*)omin->GetValue().mb_str(),
              (const char*)omax->GetValue().mb_str());
        return;
    }
    debug("\n");

    vector<wstring> wordList;
    try
    {
        dic->searchRegExp(regexp, wordList, lmin, lmax);
    }
    catch (InvalidRegexpException &e)
    {
        wxString msg = _("Invalid regular expression: ") + wxU(e.what());
        l->Append(msg);
        return;
    }

    wxString *res = new wxString[wordList.size()];
    int resnum = 0;
    vector<wstring>::const_iterator it;
    for (it = wordList.begin(); it != wordList.end(); it++)
    {
        res[resnum++] =  wxU(it->c_str());
    }
    l->Set(resnum,res);
    delete[] res;
    check_end();
}

// ************************************************************
// ************************************************************
// ************************************************************

SearchPanel::SearchPanel(wxFrame *parent, const Dictionary &dic) :
  wxNotebook(parent, -1)
{
  AddPage(new PCross (this,ID_PANEL_CROSS ,dic), _("Cross words"));
  AddPage(new PPlus1 (this,ID_PANEL_PLUS1 ,dic), _("Plus 1"));
  AddPage(new PRegExp(this,ID_PANEL_REGEXP,dic), _("Regular expressions"));
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
