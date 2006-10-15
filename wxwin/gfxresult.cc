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
 *  \file   reslist.cc
 *  \brief  Search results list view
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#include <wx/wx.h>

#include "ewx.h"
#include "dic.h"
#include "game.h"
#include "player.h"
#include "training.h"
#include "configdb.h"
#include "gfxresult.h"
#include "mainframe.h"

/* ************************************************** */
/* ************************************************** */

enum {
    ListCtrl_ID                               = 11000
};

BEGIN_EVENT_TABLE(GfxResult, wxControl)
    EVT_SIZE(GfxResult::OnSize)
    EVT_LIST_ITEM_SELECTED  (ListCtrl_ID, GfxResult::OnListCtrlSelected)
    EVT_LIST_ITEM_ACTIVATED (ListCtrl_ID, GfxResult::OnListCtrlActivated)
END_EVENT_TABLE()

/* ************************************************** */
/* ************************************************** */

GfxResult::GfxResult(wxFrame *parent, MainFrame* _mf, Game* _game) :
    wxControl(parent, wxWindowID(234), wxDefaultPosition, wxDefaultSize,
	      wxNO_BORDER | wxFULL_REPAINT_ON_RESIZE)
{
    mf = _mf;
    game = _game;
    savedrack = L"";
    results = new wxListCtrl(this, ListCtrl_ID);
#if defined(ENABLE_LC_NO_HEADER)
    results->SetSingleStyle(wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL);
#else
    results->SetSingleStyle(wxLC_REPORT | wxLC_SINGLE_SEL);
#endif
    results->InsertColumn(0, wxT("Sol"));
    results->InsertColumn(1, wxT("*"));
    results->InsertColumn(2, wxT("Pos"));
    results->InsertColumn(3, wxT("Pts"));
    results->SetToolTip(wxT("Resultats de la recherche"));

    wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *sizer_h = new wxBoxSizer(wxHORIZONTAL);

    sizer_v->Add(results, 1, wxEXPAND, 0);
    sizer_h->Add(sizer_v, 1, wxEXPAND, 0);

    SetAutoLayout(TRUE);
    SetSizer(sizer_h);
    sizer_h->Fit(this);
    sizer_h->SetSizeHints(this);
}

/* ************************************************** */
/* ************************************************** */

GfxResult::~GfxResult(void)
{
    //debug("   GfxResult::~GfxResult\n");
    Show(false);
    Show(true);
}

/* ************************************************** */
/* ************************************************** */

void
GfxResult::SetGame(Game* g)
{
    game = g;
    savedrack = L"";
    results->DeleteAllItems();
}

/* ************************************************** */
/* ************************************************** */

void
GfxResult::Refresh()
{
    if (game == NULL)
        return;

    debug("   GfxResult::Refresh : ");
    std::wstring rack = game->getCurrentPlayer().getCurrentRack().toString();

    if (savedrack != rack || static_cast<Training*>(game)->getResults().size() != results->GetItemCount())
    {
        debug("changed (%ls -> %ls)",savedrack.c_str(),rack.c_str());
        savedrack = rack;
        results->DeleteAllItems();
    }
    else
    {
        debug("unchanged");
    }
    debug("\n");
}

/* ************************************************** */
/* ************************************************** */

void
GfxResult::Search()
{
    debug("GfxResult::Search()\n");
    if (game == NULL)
        return;

    ((Training*)game)->search();

    results->DeleteAllItems();
    results->SetFont(config.getFont(LISTFONT));

    const Results &res = ((Training*)game)->getResults();
    debug("   GfxResult::Search size = %d\n",res.size());
    for (int i = 0; i < res.size(); i++)
    {
        Round r = res.get(i);
        //debug("    adding %s\n",r.toString().c_str());
        wxString pts;
        wxString word   = wxU(r.getWord().c_str());
        wxString coords = wxU(r.getCoord().toString().c_str());
        wxChar   bonus  = r.getBonus() ?  wxT('*') : wxT(' ');
        pts << r.getPoints();

        long tmp = results->InsertItem(i, word);
        results->SetItemData(tmp, i);
        tmp = results->SetItem(i, 1, bonus);
        tmp = results->SetItem(i, 2, coords);
        tmp = results->SetItem(i, 3, pts);
    }

    for (int i = 0; i < 4; i++)
        results->SetColumnWidth(i, wxLIST_AUTOSIZE);

    //results->Show();

    if (res.size() > 0)
    {
        results->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED | wxLIST_MASK_STATE);
        ((Training*)game)->testPlay(0);
    }
}

/* ************************************************** */
/* ************************************************** */

int
GfxResult::GetSelected()
{
    int item = -1;
    item = results->GetNextItem(item,wxLIST_NEXT_ALL,wxLIST_STATE_SELECTED);
    //debug("GfxResult::GetSelected = %d\n",item);
    return item;
}

/* ************************************************** */
/* ************************************************** */

void
GfxResult::OnListCtrlSelected(wxListEvent& event)
{
    //debug("   GfxResult::OnListCtrlSelected\n");
    if (event.m_itemIndex > -1)
    {
        mf->TestPlay(event.m_itemIndex);
    }
}

/* ************************************************** */
/* ************************************************** */

void
GfxResult::OnListCtrlActivated(wxListEvent& event)
{
    //debug("   GfxResult::OnListCtrlActivated");
    if (event.m_itemIndex > -1)
    {
        mf->Play(1);
        results->DeleteAllItems();
    }
}

/* ************************************************** */
/* ************************************************** */

void
GfxResult::OnSize(wxSizeEvent& e)
{
    int w,h;
    GetClientSize(&w,&h);
    results->SetClientSize(w,h);
    //debug("   GfxResult::OnSize (%d,%d)\n",w,h);
}

/* ************************************************** */
/* ************************************************** */

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
