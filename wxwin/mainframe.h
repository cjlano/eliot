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

/**
 *  \file   mainframe.h
 *  \brief  Main frame for the Eliot GUI
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#ifndef _MAINFRAME_H_
#define _MAINFRAME_H_

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
#ifdef ENABLE_RESLIST_IN_MAIN
    GfxResult   *reslist;
#endif

    wxButton    *b_play;
    wxButton    *b_rackrandomset;
    wxButton    *b_rackrandomnew;
    wxButton    *b_search;
    wxButton    *b_back;

    wxStatusBar *statusbar;

    void InitFrames();
    void InitMenu();
    void UpdateStatusBar();

public:
    MainFrame(wxPoint,wxSize);
    virtual ~MainFrame();
    
    // *******
    // Actions
    // *******
    void SetRack(Game::set_rack_mode, wxString = wxT(""));
    void Search();
    void Play(int);
    void TestPlay(int);

    void UpdateFrames(AuxFrame::refresh_t force = AuxFrame::REFRESH);
    
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
    void OnTextEnter(wxCommandEvent& event);

    // *******
    // Objects
    // *******
    void OnCloseWindow       (wxCloseEvent& event);

    DECLARE_EVENT_TABLE()
};

#endif

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// End:
