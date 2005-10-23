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

/* $Id: gfxboard.cc,v 1.5 2005/10/23 14:53:44 ipkiss Exp $ */

#include <string.h>
#include <math.h>
#include <ctype.h>

#include "wx/dcmemory.h"

#include "ewx.h"
#include "dic.h"
#include "game.h"
#include "configdb.h"
#include "gfxboard.h"


BEGIN_EVENT_TABLE(GfxBoard, wxWindow)
  EVT_PAINT(GfxBoard::OnPaint)
  EVT_SIZE(GfxBoard::OnSize)
END_EVENT_TABLE()


#define LINE_WIDTH 2
#define BOARD_SIZE 17


GfxBoard::GfxBoard(wxFrame *parent, Game &iGame) :
    wxWindow(parent, -1), m_game(iGame)
{
    bmp = NULL;
    board_size = 0;
    tile_size = 0;
    memset(paintedboard_char,0,sizeof(paintedboard_char));
    memset(paintedboard_attr,0,sizeof(paintedboard_attr));
}


GfxBoard::~GfxBoard(void)
{
    if (bmp)
    {
        delete bmp;
    }
}


void
GfxBoard::OnSize(wxSizeEvent& e)
{
  size = GetClientSize();

  board_size = size.GetWidth() < size.GetHeight() ? 
    size.GetWidth() : size.GetHeight();

  tile_size = (int)((float)board_size / (float)(BOARD_SIZE)) - LINE_WIDTH;

  TopLeft  = wxPoint((size.GetWidth()  - (board_size - tile_size/2)) / 2,
		     (size.GetHeight() - (board_size - tile_size/2)) / 2);

  if (bmp)
    {
      delete bmp;
      bmp = NULL;
    }
}


void
GfxBoard::CreateBMP()
{
    if (!bmp)
    {
        wxSize bs = GetClientSize();
        bmp=new wxBitmap(bs.x,bs.y);
        if (bmp)
        {
            wxMemoryDC memDC;
            memDC.SelectObject(* bmp);
            DrawBoard(&memDC);
            memDC.SelectObject(wxNullBitmap);
        }
    }
}


void
GfxBoard::OnPaint(wxPaintEvent&)
{
  wxPaintDC dc(this);

  CreateBMP();

  if (bmp)
    {
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
	  dc.Blit(vX,vY,vW,vH,&memDC,vX,vY,wxCOPY);
	  upd ++ ;
	}

      memDC.SelectObject(wxNullBitmap);
    }
  else
    {
      DrawBoard(&dc);
    }
}


void
GfxBoard::Refresh(board_refresh_t force)
{
  wxClientDC dc(this);

  CreateBMP();

  if (force == BOARD_FORCE_REFRESH)
    {
      if (bmp)
	{
	  wxMemoryDC memDC;
	  memDC.SelectObject(* bmp);
	  DrawBoard(&memDC);
	  dc.Blit(0,0,board_size,board_size,&memDC,0,0,wxCOPY);
	  memDC.SelectObject(wxNullBitmap);
	}
      else
	{
	  DrawBoard(&dc);
	}
      return;
    }

  if (bmp)
    {
      int vX,vY,vW,vH;
      wxMemoryDC memDC;
      memDC.SelectObject(* bmp);

      DrawBoard(&memDC);
      vX = (tile_size + LINE_WIDTH) * left;
      vY = (tile_size + LINE_WIDTH) * top;
      vW = (tile_size + 2*LINE_WIDTH) * (right - left + LINE_WIDTH);
      vH = (tile_size + 2*LINE_WIDTH) * (bottom - top + LINE_WIDTH);

      dc.Blit(vX,vY,vW,vH,&memDC,vX,vY,wxCOPY);

      memDC.SelectObject(wxNullBitmap);
    }
  else
    {
      DrawBoard(&dc);
    }
}


void
GfxBoard::DrawTile(wxDC *dc, wxString& wxs, int row, int column)
{
  int l;
  char c = 0;
  wxCoord width, height;
  wxCoord posx, posy;

  // redraw borders 
  if (row && column)
    dc->DrawRectangle(column*(tile_size+LINE_WIDTH) + TopLeft.x,
		      row*(tile_size+LINE_WIDTH)    + TopLeft.y,
		      tile_size + 2*LINE_WIDTH,
		      tile_size + 2*LINE_WIDTH);

  //   const char* ptr = wxs.mb_str();
  //   if (wxs.Len() && isalnum(*ptr))
  //     {

  const char* ptr = (const char*)wxs.c_str();
  l = strlen(ptr);
  if (ptr)
    c = *ptr;

  if (l > 0 && isalnum(c))
    {
      // we got a letter (or 2 chars for coordinates > 9)
      dc->GetTextExtent(wxs,&width,&height);
      posx = TopLeft.x + column*(tile_size+LINE_WIDTH) + LINE_WIDTH +
	(tile_size - width) / 2;
      posy = TopLeft.y +    row*(tile_size+LINE_WIDTH) + LINE_WIDTH + 
	(tile_size - height) / 2;
      dc->DrawText(wxs,posx,posy);
    }
}


void
GfxBoard::DrawBoard(wxDC *dc)
{
  wxString wxs;
  int attr;
  int row,column;

  left = BOARD_MAX;
  right = BOARD_MIN;
  top = BOARD_MAX;
  bottom = BOARD_MIN;

  wxFont font = config.getFont(BOARDFONT);

  wxColour colLines       = config.getColour(wxString(BCOLOURLINES));
  wxColour colLetters     = config.getColour(wxString(BCOLOURLETTERS));
  wxColour colTestLetters = config.getColour(wxString(BCOLOURTSTLETTERS));
  wxColour colBackground  = config.getColour(wxString(BCOLOURBACKGROUND));
  wxColour colWx3         = config.getColour(wxString(BCOLOURWX3));
  wxColour colWx2         = config.getColour(wxString(BCOLOURWX2));
  wxColour colLx3         = config.getColour(wxString(BCOLOURLX3));
  wxColour colLx2         = config.getColour(wxString(BCOLOURLX2));

  wxPen   *LinesPen = wxThePenList->FindOrCreatePen(colLines, 1, wxSOLID);
  wxBrush *BackgroundBrush = wxTheBrushList->FindOrCreateBrush(colBackground, 
							       wxSOLID);

  wxBrush *Wx3Brush = wxTheBrushList->FindOrCreateBrush(colWx3, wxSOLID);
  wxBrush *Wx2Brush = wxTheBrushList->FindOrCreateBrush(colWx2, wxSOLID);
  wxBrush *Lx3Brush = wxTheBrushList->FindOrCreateBrush(colLx3, wxSOLID);
  wxBrush *Lx2Brush = wxTheBrushList->FindOrCreateBrush(colLx2, wxSOLID);

  dc->SetFont(font);
  LinesPen->SetWidth(LINE_WIDTH);
  dc->SetPen(* LinesPen);
  dc->SetBrush(* BackgroundBrush);

  // background rectangle (entire frame)
  wxSize bs = GetClientSize();
  dc->DrawRectangle(0,0,bs.x,bs.y);

  // lines
  //  dc->DrawRectangle(TopLeft.x,TopLeft.y,board_size - tile_size/2, board_size - tile_size/2);
  for(row=BOARD_MIN; row < BOARD_MAX; row++)
    {
      // vertical
      dc->DrawLine(TopLeft.x + (row+1)*(tile_size+LINE_WIDTH),
		   TopLeft.y + tile_size + LINE_WIDTH,
		   TopLeft.x + (row+1)*(tile_size+LINE_WIDTH),
		   TopLeft.y + BOARD_MAX * (tile_size+LINE_WIDTH));
      // horizontal row <-> line
      dc->DrawLine(TopLeft.x + tile_size+LINE_WIDTH,
		   TopLeft.y + (row+1)*(tile_size+LINE_WIDTH),
		   TopLeft.x + BOARD_MAX * (tile_size+LINE_WIDTH),
		   TopLeft.y + (row+1)*(tile_size+LINE_WIDTH));
    }

  // 1 2 3 4 5 ...
  // A B C D
  for(row=BOARD_MIN; row <= BOARD_MAX; row++)
    {
      wxs.Printf(wxT("%d"), row);
      DrawTile(dc, wxs, 0, row);
      wxs.Printf(wxT("%c"), row + 'A' - 1);
      DrawTile(dc, wxs, row, 0);
    }

    dc->SetTextForeground(colLetters);
    for(row=BOARD_MIN; row <= BOARD_MAX; row++)
    {
        for (column = BOARD_MIN; column <= BOARD_MAX; column++)
        {
            if (m_game.getBoardLetterMultiplier(row, column) == 2)
            {
                dc->SetBrush(*Lx2Brush);
                dc->SetTextBackground(colLx2);
            }
            else if (m_game.getBoardLetterMultiplier(row, column) == 3)
            {
                dc->SetBrush(*Lx3Brush);
                dc->SetTextBackground(colLx3);
            }
            else if (m_game.getBoardWordMultiplier(row, column) == 2)
            {
                dc->SetBrush(*Wx2Brush);
                dc->SetTextBackground(colWx2);
            }
            else if (m_game.getBoardWordMultiplier(row, column) == 3)
            {
                dc->SetBrush(*Wx3Brush);
                dc->SetTextBackground(colWx3);
            }

            wxs = wxString((wxChar)m_game.getBoardChar(row, column));
            attr = m_game.getBoardCharAttr(row, column);
            if ((paintedboard_char[row - BOARD_MIN][column - BOARD_MIN] != wxs.GetChar(0)) ||
                (paintedboard_attr[row - BOARD_MIN][column - BOARD_MIN] != attr))
            {
                top = top < row ? top : row;
                bottom = bottom > row ? bottom : row;
                left = left < column ? left : column;
                right = right > column ? right : column;
                paintedboard_char[row - BOARD_MIN][column - BOARD_MIN] = wxs.GetChar(0);
                paintedboard_attr[row - BOARD_MIN][column - BOARD_MIN] = attr;
            }

            if (attr & ATTR_TEST)
            {
                dc->SetTextForeground(colTestLetters);
                DrawTile(dc,wxs,row,column);
                dc->SetTextForeground(colLetters);
            }
            else
            {
                DrawTile(dc,wxs,row,column);
            }
            dc->SetBrush(* BackgroundBrush);
            dc->SetTextBackground(colBackground);
        }
    }

    dc->SetFont(wxNullFont);
}
