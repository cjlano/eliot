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

/* $Id: mainframe.h,v 1.3 2005/03/29 07:00:39 afrab Exp $ */

// -*- C++ -*-
#ifndef _MAINFRAME_H
#define _MAINFRAME_H

#include "dic.h"
#include "game.h"
#include "auxframes.h"

class MainFrame: public wxFrame
{
private:

    Dictionary  m_dic;
    Game        *m_game;
    ConfigDB    config;
    AuxFrame    *auxframes_ptr[MAX_FRAME_ID];

    wxTextCtrl  *rack;
    wxListCtrl  *results;
    wxButton    *b_play;
    wxButton    *b_rackrandomset;
    wxButton    *b_rackrandomnew;
    wxButton    *b_search;
    wxButton    *b_back;
    wxStatusBar *statusbar;

    void Play(int);
    void Search();

    void InitFrames();
    void InitMenu();
    void UpdateStatusBar();
    void UpdateFrames(refresh_t force = REFRESH);

public:
    MainFrame(wxPoint,wxSize);
    virtual ~MainFrame();

    // *****
    // Menus
    // *****
    void OnMenuGameNew          (wxCommandEvent& event);
    void OnMenuGameOpen         (wxCommandEvent& event);
    void OnMenuGameSave         (wxCommandEvent& event);
    void OnMenuGamePrint        (wxCommandEvent& event);
    void OnMenuGamePrintPreview (wxCommandEvent& event);
    void OnMenuGamePrintPS      (wxCommandEvent& event);

    void OnMenuConfGameDic      (wxCommandEvent& event);
    void OnMenuConfGameSearch   (wxCommandEvent& event);

    void OnMenuConfPrint        (wxCommandEvent& event);

    void OnMenuConfAspectFont        (wxCommandEvent& event);
    void OnMenuConfAspectBoardColour (wxCommandEvent& event);

    void OnMenuShowFrame        (wxCommandEvent& event);

    void OnMenuQuitApropos      (wxCommandEvent& event);
    void OnMenuQuitConfirm      (wxCommandEvent& event);

    // *******
    // Buttons
    // *******
    void OnPlay     (wxCommandEvent& event);
    void OnSetRack  (wxCommandEvent& event);
    void OnSearch   (wxCommandEvent& event);
    void OnPlayBack (wxCommandEvent& event);


    // *******
    // Objects
    // *******
    void OnListCtrlSelected  (wxListEvent& event);
    void OnListCtrlActivated (wxListEvent& event);

    void OnCloseWindow       (wxCloseEvent& event);

    DECLARE_EVENT_TABLE()
};

#endif
