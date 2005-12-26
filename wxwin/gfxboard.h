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
 *  \file   gfxboard.h
 *  \brief  Game board graphical view
 *  \author Antoine Fraboulet
 *  \date   2002
 */

#ifndef _GFXBOARD_H
#define _GFXBOARD_H

/**
 * gfxboard is a wxWindow widget that draws a Scrabble board
 */

class GfxBoard : public wxWindow
{
 private:
    /**
     * reference on the played game
     */
    Game &m_game;

    /**
     *	paintedboard_char is the matrix of played tiles
     */
    wxChar paintedboard_char[BOARD_DIM][BOARD_DIM];

    /**
     * paintedboard_attr is the matrix of special attributes for tiles, for
     * instance it can store if a tile is a test tile (placed but not played).
     */
    char   paintedboard_attr[BOARD_DIM][BOARD_DIM];

    /**
     * size in pixels for the board.
     * board_size = min(width,height)
     */
    int board_size;

    /**
     * tile_size = size in pixels of a tile
     */
    int tile_size;

    /**
     * there is a bug when doing an OnSize under windows : the window
     * has to be fully refreshed (UpdateRegion does not seem to work).
     */
#if defined(MSW_RESIZE_BUG)
    bool just_resized;
#endif

    /**
     * top left point used to draw the lines, used to keep the board
     * centered horizontally and vertically
     */
    wxPoint  TopLeft;

    /**
     * Board bitmap, created by CreateBMP
     */
    wxBitmap *bmp;

    void CreateBMP();
    void DrawTileBack(wxDC*,int,int,int, bool testtile);
    void DrawTile(wxDC*,wxString&,int,int,bool testtile = false, bool drawtileback = false);
    void DrawBoard(wxDC*);
    void RefreshSquare(wxRect&);

    ConfigDB config;

 public:

    GfxBoard(wxFrame* parent, Game& game);
    ~GfxBoard(void);

    void OnPaint (wxPaintEvent& event);
    void OnSize  (wxSizeEvent&  event);

    typedef enum {
	BOARD_REFRESH,
	BOARD_FORCE_REFRESH
    } board_refresh_t;

    void Refresh (board_refresh_t force = BOARD_REFRESH);

    DECLARE_EVENT_TABLE()
};

#endif


/// Local Variables:
/// mode: hs-minor
/// c-basic-offset: 4
/// End:
