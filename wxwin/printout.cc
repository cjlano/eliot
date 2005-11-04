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

#include <stdio.h>

#include "wx/wx.h"

#include "ewx.h"

#include "dic.h"
#include "game.h"

#include "configdb.h"
#include "printout.h"

bool
GamePrintout::OnPrintPage(int page)
{
    wxDC *dc = GetDC();
    if (dc)
    {
        if (page == 1)
            DrawPage(dc);

        return TRUE;
    }
    else
        return FALSE;
}

bool
GamePrintout::HasPage(int pageNum)
{
    return pageNum == 1;
}

bool
GamePrintout::OnBeginDocument(int startPage, int endPage)
{
    if (!wxPrintout::OnBeginDocument(startPage, endPage))
        return FALSE;

    return TRUE;
}

void
GamePrintout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
    *minPage = 1;
    *maxPage = 1;
    *selPageFrom = 1;
    *selPageTo = 1;
}

void
GamePrintout::SetSpaces(wxString* str, int spaces)
{
    size_t i;
    wxString strs = wxT("");
    if (str->Len() == 0)
        return ;
    for(i=0; i < (str->Len()-1); i++) {
        strs = strs + str->GetChar(i);
        strs = strs + wxString(wxChar(' '), spaces);
    }
    strs = strs + str->GetChar(str->Len() - 1);
    *str = strs;
}

void
GamePrintout::DrawStringJustif(wxDC *dc, wxString *str, long x, long y, long w,
                               enum Justif justif, int spaces)
{
    long  wtext,htext;

    SetSpaces(str,spaces);
    dc->GetTextExtent(*str,&wtext,&htext);

    switch (justif)
    {
        case LEFT:
            break;
        case CENTER:
            x = x + (w - wtext) / 2;
            break;
        case RIGHT:
            x = x + w - wtext;
            break;
    }
    dc->DrawText(*str,x,y);
}

void
GamePrintout::DrawHeadingLine(wxDC *dc, long heightH, float mmToLogical)
{
    long i,x,w,y;
    wxString str;

    x = config.getMarginX() + config.getDxBegin();
    y = config.getMarginY() + config.getDyT1();
    for (i = 0; i < 5; i++)
    {
        w = config.getDxText(i);
        str = config.getNameH(i);
        DrawStringJustif(dc,&str,
                         (long) (mmToLogical*x),
                         (long) (mmToLogical*y),
                         (long) (mmToLogical*w),
                         config.getJustifH(i),
                         config.getSpacesH(i));
        x += w + config.getDxEnd() + config.getDxBegin();
    }
}

void
GamePrintout::DrawTextLine(wxDC *dc, int numline, long basey, long heightT, float mmToLogical)
{
#define DIM(i)                                         \
     x += w + config.getDxEnd() + config.getDxBegin(); \
     w = config.getDxText(i)

#define DRW(i)                                         \
     DrawStringJustif(dc,&str,(long) (mmToLogical*x),  \
                              (long) (mmToLogical*y),  \
                              (long) (mmToLogical*w),  \
                              config.getJustifT(i),    \
                              config.getSpacesT(i))

    long x,y,w;
    wxString str;

    x = config.getMarginX() + config.getDxBegin();
    y = basey + config.getDyT1()
        + numline * (config.getDyT1() + heightT + config.getDyT2());
    w = config.getDxText(0);
    str = wxT("");
    // num
    if (numline < m_game.getNRounds())
    {
        str << (numline + 1);
        DRW(0);
    }
    // rack
    DIM(1);
    if (numline < m_game.getNRounds())
    {
        str = wxU(m_game.getPlayedRack(numline).c_str());
        DRW(1);
    }
    // word
    DIM(2);
    if ((numline > 0) && (numline <= m_game.getNRounds()))
    {
        str = wxU(m_game.getPlayedWord(numline - 1).c_str());
        DRW(2);
    }
    // pos
    DIM(3);
    if ((numline > 0) && (numline <= m_game.getNRounds()))
    {
        str = wxU(m_game.getPlayedCoords(numline - 1).c_str());
        DRW(3);
    }
    // pts
    DIM(4);
    if ((numline > 0) && (numline <= m_game.getNRounds()))
    {
        str << m_game.getPlayedPoints(numline - 1);
        DRW(4);
    }
    // total points
    if (numline == m_game.getNRounds() + 1)
    {
        str << m_game.getPlayerPoints(0);
        DRW(4);
    }
#undef DIM
#undef DRW
}

void
GamePrintout::DrawPage(wxDC *dc)
{
/*
 * Scaling.
 */
     // Get the logical pixels per inch of screen and printer
     int dcSizeW, dcSizeH;
     int pageWidthPix, pageHeightPix;
     int ppiScreenX, ppiScreenY;
     int ppiPrinterX, ppiPrinterY;

     GetPPIScreen(&ppiScreenX, &ppiScreenY);
     GetPPIPrinter(&ppiPrinterX, &ppiPrinterY);
     dc->GetSize(&dcSizeW, &dcSizeH);
     GetPageSizePixels(&pageWidthPix, &pageHeightPix);

     // This scales the DC so that the printout roughly represents the
     // the screen scaling. The text point size _should_ be the right size
     // but in fact is too small for some reason. This is a detail that will
     // need to be addressed at some point but can be fudged for the
     // moment.
     float scale = (float)((float)ppiPrinterX/(float)ppiScreenX);
     // Now we have to check in case our real page size is reduced
     // (e.g. because we're drawing to a print preview memory DC)

     // If printer pageWidth == current DC width, then this doesn't
     // change. But w might be the preview bitmap width, so scale down.
     float overallScaleX = scale * (float)(dcSizeW/(float)pageWidthPix);
     float overallScaleY = scale * (float)(dcSizeH/(float)pageHeightPix);
     dc->SetUserScale(overallScaleX, overallScaleY);
     float mmToLogical = (float)(ppiPrinterX/(scale*25.1));

     long i,basey;
     long heightH, heightT;
     wxFont Hfont = config.getFont(PRINTHFONT);
     wxFont Tfont = config.getFont(PRINTTFONT);

#if wxCHECK_VERSION(2,5,0)
     wxColour wxBlack = wxTheColourDatabase->Find(wxT("BLACK"));
     wxColour wxWhite = wxTheColourDatabase->Find(wxT("WHITE"));
     wxPen    *blackPen = wxThePenList->FindOrCreatePen(wxBlack, 1, wxSOLID);
     wxBrush  *whiteBrush = wxTheBrushList->FindOrCreateBrush(wxWhite, wxSOLID);
#else
     wxColour *wxBlack = wxTheColourDatabase->FindColour(wxT("BLACK"));
     wxColour *wxWhite = wxTheColourDatabase->FindColour(wxT("WHITE"));
     wxPen    *blackPen = wxThePenList->FindOrCreatePen(*wxBlack, 1, wxSOLID);
     wxBrush  *whiteBrush = wxTheBrushList->FindOrCreateBrush(*wxWhite, wxSOLID);
#endif


     dc->SetPen(* blackPen);
     dc->SetBrush(* whiteBrush);
     dc->SetFont(Hfont);
     heightH = (long) (dc->GetCharHeight() / mmToLogical);
     DrawHeadingLine(dc,heightH,mmToLogical);
     basey = config.getMarginY() + config.getDyH1() + heightH + config.getDyH2();
     dc->SetFont(Tfont);
     heightT = (long) (dc->GetCharHeight() / mmToLogical);
     for(i=0; i < (m_game.getNRounds()+3);i++)
     {
         DrawTextLine(dc,i,basey,heightT,mmToLogical);
     }
     dc->SetFont(wxNullFont);
     DrawGameLines(dc,heightH,heightT,mmToLogical,overallScaleX,overallScaleY);
}

void
GamePrintout::DrawGameLines(wxDC *dc, long heightH, long heightT,
                            float mmToLogical, float overallScaleX,
                            float overallScaleY)
{
    int i, nTextLines;
    long col,lin, StartX, StartY;
    long HeadHeight, LineHeight;
    long TextStart, TextHeight, TextBottom, TextRight;

    float SCALE = config.getPrintLineScale();
    dc->SetUserScale(SCALE,SCALE);

    nTextLines = m_game.getNRounds() + 2;
    StartX = config.getMarginX();
    StartY = config.getMarginY();

    HeadHeight = config.getDyH1() + heightH + config.getDyH2();
    LineHeight = config.getDyT1() + heightT + config.getDyT2();
    TextStart  = StartY + HeadHeight;
    TextHeight = nTextLines * LineHeight;
    TextBottom = TextStart  + TextHeight;

    //
    // columns
    //
    float scalefactorX = mmToLogical * overallScaleX / SCALE;
    float scalefactorY = mmToLogical * overallScaleY / SCALE;

    col = StartX;
    for (i = 0; i <= 5; i++)
    {
        dc->DrawLine((long) (col * scalefactorX),
                     (long) (StartY * scalefactorY),
                     (long) (col * scalefactorX),
                     (long) (TextBottom * scalefactorY));
        col += i == 5 ? 0 : config.getDxBegin() + config.getDxText(i) + config.getDxEnd();
    }
    TextRight = col;
    //
    // rows
    //
    lin = StartY;
    for (i = 0; i <= (nTextLines + 1); i++)
    {
        dc->DrawLine((long) (StartX * scalefactorX),
                     (long) (lin * scalefactorY),
                     (long) (TextRight * scalefactorX),
                     (long) (lin * scalefactorY));
        lin = StartY + HeadHeight + i * LineHeight;
    }
}
