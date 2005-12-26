/* Eliot                                                                     */
/* Copyright (C) 2005  Antoine Fraboulet                                     */
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
 *  \file   reslist.h
 *  \brief  Search results list view
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#ifndef _RESLIST_H
#define _RESLIST_H

/**
 *
 */
#include <wx/listctrl.h>

class MainFrame;

class GfxResult : public wxControl
{
 private:
    MainFrame    *mf;
    std::string  savedrack;
    Game         *game;
    wxListCtrl   *results;
    ConfigDB     config;

 public:
    GfxResult(wxFrame*, MainFrame*, Game*);
    ~GfxResult();

    void SetGame(Game*);
    void Search();
    int  GetSelected();
    void Refresh();

    void OnSize(wxSizeEvent& e);
    void OnListCtrlSelected(wxListEvent& event);
    void OnListCtrlActivated(wxListEvent& event);

    DECLARE_EVENT_TABLE()
};


#endif

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// End:
