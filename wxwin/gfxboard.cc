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
 *  \file   gfxboard.cc
 *  \brief  Game board graphical view
 *  \author Antoine Fraboulet
 *  \date   2002
 */

#include <string.h>
#include <math.h>
#include <ctype.h>

#include "wx/dcmemory.h"

#include "ewx.h"
#include "dic.h"
#include "game.h"
#include "configdb.h"
#include "gfxboard.h"

#ifdef DEBUG_
#   define GFXDEBUG(x) x
#else
#   define GFXDEBUG(x)
#endif

BEGIN_EVENT_TABLE(GfxBoard, wxWindow)
    EVT_PAINT(GfxBoard::OnPaint)
    EVT_SIZE(GfxBoard::OnSize)
END_EVENT_TABLE()

#define LINE_WIDTH 1
#define BOARD_SIZE (BOARD_DIM + 2)

    /* ************************************************** */
    /* ************************************************** */

GfxBoard::GfxBoard(wxFrame *parent, Game &iGame) :
	wxWindow(parent, wxWindowID(-1), wxDefaultPosition, wxDefaultSize,
		 wxNO_BORDER | wxFULL_REPAINT_ON_RESIZE, wxT("gfxboard")),
	m_game(iGame)
{
    bmp          = NULL;
    board_size   = 0;
    tile_size    = 0;
#if defined(MSW_RESIZE_BUG)
    just_resized = false;
#endif
    for(int i=0; i<BOARD_DIM; i++)
	{
	    for(int j=0; j < BOARD_DIM; j++)
		{
		    paintedboard_char[i][j] = wxT(' ');
		    paintedboard_attr[i][j] = 0;
		}
	}
}

    /* ************************************************** */
    /* ************************************************** */

GfxBoard::~GfxBoard(void)
{
    if (bmp)
	{
	    delete bmp;
	    bmp = NULL;
	}
}

/**
 * Make new dimensions available for the next OnPaint
 * event. The BMP is deleted if it exists.
 */

void
GfxBoard::OnSize(wxSizeEvent __UNUSED__ &e)
{
    GFXDEBUG(std::cerr << "On size : ");

    wxSize cs = GetClientSize();
    board_size = cs.GetWidth() < cs.GetHeight() ? cs.GetWidth() : cs.GetHeight();
    tile_size  = (int)((float)board_size / (float)(BOARD_SIZE)) - LINE_WIDTH;

    GFXDEBUG(std::cerr << "(" << cs.GetWidth() << "," << cs.GetHeight() << ")");
    GFXDEBUG(std::cerr << " tile size " << tile_size << endl);

    TopLeft  = wxPoint((cs.GetWidth()  - (board_size - tile_size/2)) / 2,
		       (cs.GetHeight() - (board_size - tile_size/2)) / 2);

#if defined(MSW_RESIZE_BUG)
    just_resized = true;
#endif

    if (bmp)
	{
	    delete bmp;
	    bmp = NULL;
	}
}

/**
 * Creates a BMP in memory and draws the board inside
 */

void
GfxBoard::CreateBMP()
{
    GFXDEBUG(std::cerr << "Create BMP ");
    if (!bmp)
	{
	    wxSize bs = GetClientSize();
	    bmp=new wxBitmap(bs.x,bs.y);
	    GFXDEBUG(std::cerr << " new bmp (" << bs.x << "," << bs.y << ")");
	    if (bmp)
		{
		    wxMemoryDC memDC;
		    memDC.SelectObject(* bmp);
		    DrawBoard(&memDC);
		    memDC.SelectObject(wxNullBitmap);
		}
	}
    GFXDEBUG(std::cerr << endl);
}


/**
 * Update the full BMP and copy only the requested area
 * to the ClientDC
 */

void
GfxBoard::RefreshSquare(wxRect &r)
{
    wxClientDC dc(this);

    if (bmp)
	{
	    int vX,vY,vW,vH;
	    wxMemoryDC memDC;
	    memDC.SelectObject(* bmp);
	    DrawBoard(&memDC);
	    vX = r.x;
	    vY = r.y;
	    vW = r.width;
	    vH = r.height;
	    GFXDEBUG(std::cerr << " refresh (" << vX << "," << vY << "," << vW << "," << vH << ") ");
	    dc.Blit(vX,vY,vW,vH,&memDC,vX,vY,wxCOPY);
	    memDC.SelectObject(wxNullBitmap);
	}
    else
	{
	    DrawBoard(&dc);
	}
}

/**
 * Force a full refresh of the board
 */

void
GfxBoard::Refresh(board_refresh_t WXUNUSED(force))
{
    wxSize cs = GetClientSize();
    board_size = cs.GetWidth() < cs.GetHeight() ? cs.GetWidth() : cs.GetHeight();
    tile_size  = (int)((float)board_size / (float)(BOARD_SIZE)) - LINE_WIDTH;
    wxRect r (0,0,cs.GetWidth(),cs.GetHeight());
    RefreshSquare(r);
}

/**
 * Window manager OnPaint event handler.
 */

void
GfxBoard::OnPaint(wxPaintEvent&)
{
    wxPaintDC dc(this);

    CreateBMP();

    GFXDEBUG(std::cerr << "OnPaint : ");

    if (bmp)
	{
#if defined(MSW_RESIZE_BUG)
	    Refresh(BOARD_FORCE_REFRESH);
	    if (just_resized == true)
		{
		    just_resized = false;
		}
#else
	    // we keep that code for wxgtk
	    // it does not work under wxmsw, don't know why
	    // all the onsize/repaint should be checked ... later
	    int vX,vY,vW,vH;
	    wxMemoryDC memDC;
	    memDC.SelectObject(* bmp);
	    wxRegionIterator upd(GetUpdateRegion());
	    while (upd)
		{
		    vX = upd.GetX();
		    vY = upd.GetY();
		    vW = upd.GetW();
		    vH = upd.GetH();
		    GFXDEBUG(std::cerr << "+(" << vX << "," << vY << "," << vW << "," << vH << ")");
		    dc.Blit(vX,vY,vW,vH,&memDC,vX,vY,wxCOPY);
		    upd ++ ;
		}
	    memDC.SelectObject(wxNullBitmap);
#endif
	}
    else
	{
	    GFXDEBUG(std::cerr << " call to DrawBoard ");
	    DrawBoard(&dc);
	}

    GFXDEBUG(std::cerr << "End of OnPaint" << endl);
}


void
GfxBoard::DrawTileBack(wxDC* dc, int top, int left, int size, bool testtile)
{
    //    void DrawRoundedRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height, double radius = 20)
    wxBrush oldbrush = dc->GetBrush();
    wxColour colBackground;

    if (testtile)
	{
	    colBackground = config.getColour(wxString(BTSTTILEBACKGROUND));
	}
    else
	{
	    colBackground = config.getColour(wxString(BTILEBACKGROUND));
	}

    wxBrush *BackgroundBrush = wxTheBrushList->FindOrCreateBrush(colBackground, wxSOLID);
    dc->SetBrush(* BackgroundBrush);
    dc->DrawRoundedRectangle(left,top,size,size,std::max(2,size/6));
    dc->SetBrush(oldbrush);
}

/**
 * Draw a tile to the wxDC object.
 */

#define TILE_LEFT(col) (col*(tile_size+LINE_WIDTH) + TopLeft.x)
#define TILE_TOP(row) (row*(tile_size+LINE_WIDTH) + TopLeft.y)

/*
   TODO
   - ajuster avec une LINE_SIZE differente
   - calculer la taille de la police
*/

/*    TILE_LEFT
      |
      |  TILE_LEFT + LINE_WIDTH
      |  |
      +++++++++-- TILE_TOP
      +++++++++
      +++++++++
      +++   +++-------------- TILE_TOP + LINE_WIDTH
      +++   +++ | = tile_size
      +++   +++--
      +++++++++--
      +++++++++ | = LINE_WIDTH
      +++++++++--
      | |
      |_|
       |
       LINE_WIDTH
*/

void
GfxBoard::DrawTile(wxDC *dc, wxString& wxs, int row, int column, bool testtile, bool drawtileback)
{
    wxColour colour;
    wxCoord width, height;
    wxCoord posx, posy;
    wxCoord left,top;
    // redraw borders
    left = TILE_LEFT(column);
    top  = TILE_TOP(row);

    if (wxs.length() > 0 && *wxs.GetData())
	{
	    // we got a letter (or 2 chars for coordinates > 9)
	    // draw plastic tile
	    if (drawtileback)
		{
		    DrawTileBack(dc,
				 top  + LINE_WIDTH,
				 left + LINE_WIDTH,
				 tile_size, testtile);
		}
	    // draw letter
	    if (testtile)
		{
		    colour = config.getColour(wxString(BCOLOURTSTLETTERS));
		}
	    else
		{
		    colour = config.getColour(wxString(BCOLOURLETTERS));
		}

	    dc->SetTextForeground(colour);
	    dc->GetTextExtent(wxs,&width,&height);
	    posx = left + LINE_WIDTH + (tile_size - width) / 2;
	    posy = top  + LINE_WIDTH + (tile_size - height) / 2;
	    dc->DrawText(wxs,posx,posy);
	}
}

/**
 * Draw the complete board in the wxDC.
 */

void
GfxBoard::DrawBoard(wxDC *dc)
{
    Board board;

    wxString wxs;
    int row,column;

    wxFont   font            = config.getFont(BOARDFONT);
    wxColour colForeground   = config.getColour(wxString(BCOLOURLINES));
    wxColour colBackground   = config.getColour(wxString(BCOLOURBACKGROUND));

    wxBrush *BackgroundBrush = wxTheBrushList->FindOrCreateBrush(colBackground, wxSOLID);
    wxPen   *LinesPen        = wxThePenList->FindOrCreatePen(colForeground, LINE_WIDTH, wxSOLID);

    dc->SetFont (font);
    dc->SetPen  (* LinesPen);
    dc->SetBrush(* BackgroundBrush);

    // background rectangle (entire frame)
    wxSize bs = GetClientSize();
    dc->DrawRectangle(0,0,bs.x,bs.y);

    // lines
    for(row=BOARD_MIN; row < BOARD_MAX; row++)
	{
	    // vertical
	    dc->DrawLine(TILE_LEFT(row+1),
			 TILE_TOP(1),
			 TILE_LEFT(row+1),
			 TILE_TOP(BOARD_MAX));
	    // horizontal row <-> line
	    dc->DrawLine(TILE_LEFT(1),
			 TILE_TOP(row+1),
			 TILE_LEFT(BOARD_MAX),
			 TILE_TOP(row+1));
	}

    // 1 2 3 4 5 ...
    // A B C D ...
    for(row=BOARD_MIN; row <= BOARD_MAX; row++)
	{
	    wxs.Printf(wxT("%d"), row);
	    DrawTile(dc, wxs, 0, row);
	    wxs.Printf(wxT("%c"), row + 'A' - 1);
	    DrawTile(dc, wxs, row, 0);
	}

    // Board Background
    wxColour colWx3         = config.getColour(wxString(BCOLOURWX3));
    wxColour colWx2         = config.getColour(wxString(BCOLOURWX2));
    wxColour colLx3         = config.getColour(wxString(BCOLOURLX3));
    wxColour colLx2         = config.getColour(wxString(BCOLOURLX2));

    wxBrush *Wx3Brush = wxTheBrushList->FindOrCreateBrush(colWx3, wxSOLID);
    wxBrush *Wx2Brush = wxTheBrushList->FindOrCreateBrush(colWx2, wxSOLID);
    wxBrush *Lx3Brush = wxTheBrushList->FindOrCreateBrush(colLx3, wxSOLID);
    wxBrush *Lx2Brush = wxTheBrushList->FindOrCreateBrush(colLx2, wxSOLID);

    board = m_game.getBoard();
    for(row=BOARD_MIN; row <= BOARD_MAX; row++)
	{
	    for (column = BOARD_MIN; column <= BOARD_MAX; column++)
		{
		    if (board.GetLetterMultiplier(row, column) == 2)
			{
			    dc->SetBrush(*Lx2Brush);
			}
		    else if (board.GetLetterMultiplier(row, column) == 3)
			{
			    dc->SetBrush(*Lx3Brush);
			}
		    else if (board.GetWordMultiplier(row, column) == 2)
			{
			    dc->SetBrush(*Wx2Brush);
			}
		    else if (board.GetWordMultiplier(row, column) == 3)
			{
			    dc->SetBrush(*Wx3Brush);
			}
		    else
			{
			    dc->SetBrush(*BackgroundBrush);
			}

		    if (row && column)
			{
			    dc->DrawRectangle(TILE_LEFT(column), TILE_TOP(row),
					      tile_size + 2*LINE_WIDTH,
					      tile_size + 2*LINE_WIDTH);
			}
		}
	}

    // Tiles
    LinesPen->SetWidth(1);
    dc->SetPen  (* LinesPen);
    bool drawtiles = config.getDrawTile();
    for(row=BOARD_MIN; row <= BOARD_MAX; row++)
	{
	    for (column = BOARD_MIN; column <= BOARD_MAX; column++)
		{
		    int attr = board.getCharAttr(row, column);
		    wxs  = wxString((wxChar)board.getChar(row, column));

		    paintedboard_char[row - BOARD_MIN][column - BOARD_MIN] = *wxs.GetData();
		    paintedboard_attr[row - BOARD_MIN][column - BOARD_MIN] = attr;

		    DrawTile(dc,wxs,row,column,attr & ATTR_TEST,drawtiles);
		}
	}

    dc->SetFont(wxNullFont);
}

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
