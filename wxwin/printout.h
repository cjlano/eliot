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

#ifndef _PRINTOUT_H
#define _PRINTOUT_H

#include <ctype.h>
#include "wx/print.h"

class GamePrintout : public wxPrintout
{
private:
    Game& m_game;
    ConfigDB config;

    void SetSpaces(wxString*, int);
    void DrawStringJustif(wxDC*,wxString*,long,long,long,enum Justif, int);
    void DrawHeadingLine(wxDC*, long, float);
    void DrawTextLine(wxDC*, int, long, long, float);
    void DrawGameLines(wxDC*, long, long, float, float, float);
public:

    GamePrintout(Game& iGame, char* title = APPNAME)
        : wxPrintout(wxU(title)), m_game(iGame) {}

    bool OnPrintPage(int);
    bool HasPage(int);
    bool OnBeginDocument(int startPage, int endPage);
    void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
    void DrawPage(wxDC *dc);
};

#endif

/****************************************************************/
/****************************************************************/

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// End:
