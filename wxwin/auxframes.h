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

/* $Id: auxframes.h,v 1.2 2005/02/05 11:14:56 ipkiss Exp $ */

// -*- C++ -*-
#ifndef _AUXFRAMES_H
#define _AUXFRAMES_H

#include "searchpanel.h"
#include "gfxboard.h"
#include "wx/frame.h"
#include "wx/listctrl.h"
#include "wx/textctrl.h"
#include "wx/stattext.h"
#include "wx/listbox.h"
#include "wx/button.h"

#define MIN_FRAME_ID 0
#define MAX_FRAME_ID 7

typedef enum {
    REFRESH,
    FORCE_REFRESH
} refresh_t;

typedef enum {
    ID_Frame_Verif   = MIN_FRAME_ID + 0,
    ID_Frame_Search  = MIN_FRAME_ID + 1,
    ID_Frame_Plus1   = MIN_FRAME_ID + 2, 
    ID_Frame_Racc    = MIN_FRAME_ID + 3,
    ID_Frame_Benj    = MIN_FRAME_ID + 4,
    ID_Frame_Bag     = MIN_FRAME_ID + 5,
    ID_Frame_Board   = MIN_FRAME_ID + 6
} frames_id_t;

/** ******************************
 *
 ****************************** */

class AuxFrame: public wxFrame
{
protected:
    int show;
    frames_id_t frameid;
    wxString name, classname;
    ConfigDB   config;

public:
    AuxFrame(wxFrame*, int, wxString, wxString);
    ~AuxFrame();

    void SwitchDisplay();
    void Reload();
    virtual void Refresh(refresh_t force = REFRESH) {};
};

/** ******************************
 *
 ****************************** */

class BoardFrame: public AuxFrame
{
protected:
    GfxBoard* board;
public:
    BoardFrame(wxFrame*, Game&);
    void Refresh(refresh_t force = REFRESH);
};

/** ******************************
 *
 ****************************** */

class BagFrame: public AuxFrame
{
private:
    Game& m_game;
    wxListCtrl *tiles;
public:
    BagFrame(wxFrame*, Game&);
    void Refresh(refresh_t force = REFRESH);
};

/** ******************************
 *
 ****************************** */

class SearchFrame: public AuxFrame
{
private:
    SearchPanel *panel;
public:
    SearchFrame(wxFrame*, Dictionary);
    void Refresh(refresh_t force = REFRESH);
};

/** ******************************
 *
 ****************************** */

class VerifFrame: public AuxFrame
{
private:
    Dictionary dic;
    wxTextCtrl *word;
    wxStaticText *result;
    void verif();
public:
    VerifFrame(wxFrame*, Dictionary);
    void OnText(wxCommandEvent& event);
    void Refresh(refresh_t force = REFRESH);
    DECLARE_EVENT_TABLE()
};


/** **********************************************************
**************************************************************
*********************************************************** */

class AuxFrameList: public AuxFrame
{
protected:
    wxListBox *listbox;
    wxButton *button;
public:
    AuxFrameList(wxFrame*, int, wxString, wxString);
    void OnCopy(wxCommandEvent& event);
    void Waiting();
    DECLARE_EVENT_TABLE()
};

/** ******************************
 *
 ****************************** */

class Plus1Frame: public AuxFrameList
{
protected:
    Game& m_game;
    string m_rack;
public:
    Plus1Frame(wxFrame*, Game&);
    void Refresh(refresh_t force = REFRESH);
};

/** ******************************
 *
 ****************************** */

class BenjFrame: public AuxFrameList
{
protected:
    Game& m_game;
    wxListCtrl* results;
public:
    BenjFrame(wxFrame*, Game&, wxListCtrl*);
    void Refresh(refresh_t force = REFRESH);
};

/** ******************************
 *
 ****************************** */

class RaccFrame: public AuxFrameList
{
protected:
    Game& m_game;
    wxListCtrl* results;
public:
    RaccFrame(wxFrame*, Game&, wxListCtrl*);
    void Refresh(refresh_t force = REFRESH);
};

#endif
