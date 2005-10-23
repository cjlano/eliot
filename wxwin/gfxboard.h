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

/* $Id: gfxboard.h,v 1.4 2005/10/23 14:53:44 ipkiss Exp $ */

/**
 *  \file gfxboard.h
 *  \brief  Game board graphical view
 *  \author Antoine Fraboulet
 *  \date   2002
 */

#ifndef _GFXBOARD_H
#define _GFXBOARD_H

/*
  paintedboard_char is the matrix of played tiles
  paintedboard_attr is the matrix of special attributes for tiles, for
    instance it can store if a tile is a test tile (placed but not played).
 */

typedef enum {
  BOARD_REFRESH,
  BOARD_FORCE_REFRESH
} board_refresh_t;


class GfxBoard : public wxWindow
{
private:
     Game &m_game;
     int top,bottom,left,right;
     char paintedboard_char[BOARD_DIM][BOARD_DIM];
     char paintedboard_attr[BOARD_DIM][BOARD_DIM];
     int board_size; 
     int tile_size;
     wxPoint TopLeft;
     wxSize size;
     wxBitmap *bmp;
     void CreateBMP();
     void DrawTile(wxDC*,wxString&,int,int);
     void DrawBoard(wxDC*);

     ConfigDB config;
public:
     GfxBoard(wxFrame*, Game&);
     ~GfxBoard(void);

     void Refresh(board_refresh_t force = BOARD_REFRESH);
     void OnPaint(wxPaintEvent& event);
     void OnSize(wxSizeEvent& event);
     DECLARE_EVENT_TABLE()
};

#endif


