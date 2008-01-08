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
 *  \file   auxframes.h
 *  \brief  Window Frames used in Eliot
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#ifndef _AUXFRAMES_H
#define _AUXFRAMES_H

#include "wx/frame.h"
#include "wx/listctrl.h"
#include "wx/textctrl.h"
#include "wx/stattext.h"
#include "wx/listbox.h"
#include "wx/button.h"

#include "searchpanel.h"
#include "gfxboard.h"
#include "gfxresult.h"

#define MIN_FRAME_ID 0
#define MAX_FRAME_ID 9

typedef enum {
    ID_Frame_Verif   = MIN_FRAME_ID + 0,
    ID_Frame_Search  = MIN_FRAME_ID + 1,
    ID_Frame_Plus1   = MIN_FRAME_ID + 2,
    ID_Frame_Racc    = MIN_FRAME_ID + 3,
    ID_Frame_Benj    = MIN_FRAME_ID + 4,
    ID_Frame_Bag     = MIN_FRAME_ID + 5,
    ID_Frame_Board   = MIN_FRAME_ID + 6,
    ID_Frame_Game    = MIN_FRAME_ID + 7,
    ID_Frame_Result  = MIN_FRAME_ID + 8
} frames_id_t;

/**
 * Generic AuxFrame :
 *       - BoardFrame
 *       - BagFrame
 *       - SearchFrame
 *       - VerifFrame
 *       - ResultFrame
 * derived to AuxFrameList :
 *       - Plus1Frame
 *       - BenjFrame
 *       - RaccFrame
 * derived to AuxFrameText :
 *       - GameFrame
 *
 */

class AuxFrame: public wxFrame
{
protected:
    int         show;
    frames_id_t frameid;
    wxString    name, classname;
    ConfigDB    config;

public:
    AuxFrame(wxFrame*, int, wxString, wxString);
    ~AuxFrame();

    typedef enum {
        REFRESH,
        FORCE_REFRESH
    } refresh_t;

    void SwitchDisplay();
    void Reload();
    virtual void Refresh(refresh_t __UNUSED__ force = REFRESH) {};
};

/**
 * Generic auxframe that includes a list and a """copy""" button
 */

class AuxFrameList: public AuxFrame
{
protected:
    bool      noresult;
    wstring   savedword;
    Game      *game;
    wxButton  *button;
    wxListBox *listbox;
    void Waiting();
    virtual void refresh() = 0;
public:
    AuxFrameList(wxFrame*, int, wxString, wxString, Game*);
    void OnCopy(wxCommandEvent& event);
    void Refresh(refresh_t force = REFRESH);
    DECLARE_EVENT_TABLE()
};

/**
 * Generic auxframe that includes a text area
 */

class AuxFrameText: public AuxFrame
{
 protected:
    wxTextCtrl *textbox;
 public:
    AuxFrameText(wxFrame*, int, wxString, wxString, int);
};

/**
 * Frame to display the game board
 */

class BoardFrame: public AuxFrame
{
protected:
    GfxBoard *board;
public:
    BoardFrame(wxFrame*, Game&);
    void Refresh(refresh_t force = REFRESH);
};

/**
 * Frame to display the tiles left in the bag
 */

class BagFrame: public AuxFrame
{
private:
    Game& m_game;
    wxListCtrl *tiles;
public:
    BagFrame(wxFrame*, Game&);
    void Refresh(refresh_t force = REFRESH);
};

/**
 * Several research tool in a panel widget within the frame
 */

class SearchFrame: public AuxFrame
{
private:
    SearchPanel *panel;
public:
    SearchFrame(wxFrame*, const Dictionary &);
    void Refresh(refresh_t force = REFRESH);
};

/**
 * Frame to verify if a word is in the dictionary
 */

class VerifFrame: public AuxFrame
{
protected:
    const Dictionary *dic;
    wxTextCtrl *word;
    wxStaticText *result;
    void verif();
public:
    VerifFrame(wxFrame*, const Dictionary&);
    void OnText(wxCommandEvent& event);
    void Refresh(refresh_t force = REFRESH);
    DECLARE_EVENT_TABLE()
};

/**
 * Displays the list of 7+1 for the current search
 */

class Plus1Frame: public AuxFrameList
{
protected:
    virtual void refresh();
public:
    Plus1Frame(wxFrame* p, Game* g) : AuxFrameList(p, ID_Frame_Plus1, wxT("Tirage + 1"), FRAMEPLUS1, g) {};
};

/**
 * Displays the list of benjamins for the current selected word
 */

class BenjFrame: public AuxFrameList
{
protected:
    virtual void refresh();
public:
    BenjFrame(wxFrame* p, Game* g) : AuxFrameList(p, ID_Frame_Benj, wxT("benjamins"), FRAMEBENJ, g) {};
};

/**
 * Displays the list of possible glue letter for the current selected word
 */

class RaccFrame: public AuxFrameList
{
protected:
    virtual void refresh();
public:
    RaccFrame(wxFrame* p, Game* g) : AuxFrameList(p, ID_Frame_Racc, wxT("raccords"), FRAMERACC, g) {};
};

/**
 * Displays the current game
 */

class GameFrame: public AuxFrameText
{
protected:
    Game& m_game;
public:
    GameFrame(wxFrame*, Game&);
    void Refresh(refresh_t force = REFRESH);
};

/**
 * Displays the list of possible words searched for the given rack and board
 */

class ResultFrame: public AuxFrame
{
 protected:
    GfxResult *reslist;
 public:
    ResultFrame(wxFrame*, Game*);
    void Search();
    int  GetSelected();
    void Refresh(refresh_t force = REFRESH);
    void OnSize(wxSizeEvent& e);
    DECLARE_EVENT_TABLE()
};

#endif

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
