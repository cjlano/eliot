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
 *  \file   configdb.h
 *  \brief  Access to Eliot persistant configuration data
 *  \author Antoine Fraboulet
 *  \date   2002
 */

#ifndef _CONFIGDB_H
#define _CONFIGDB_H

#include "wx/dc.h"
#include "wx/font.h"
#include "wx/colour.h"
#include "wx/frame.h"
#include "wx/cmndata.h"
#include "wx/config.h"

#define BUTTON_FONT wxFont(8,wxDEFAULT,wxNORMAL,wxNORMAL)

#define BOARD              "Board"
#define BCOLOURLINES       wxT(BOARD"/Lines")
#define BCOLOURWX2         wxT(BOARD"/Wx2")
#define BCOLOURWX3         wxT(BOARD"/Wx3")
#define BCOLOURLX2         wxT(BOARD"/Lx2")
#define BCOLOURLX3         wxT(BOARD"/Lx3")
#define BCOLOURBACKGROUND  wxT(BOARD"/Background")
#define BCOLOURLETTERS     wxT(BOARD"/Letters")
#define BCOLOURTSTLETTERS  wxT(BOARD"/TstLetters")
#define BOARDFONT          wxT(BOARD"/Font")

#define BDRAWBACKGROUND    wxT(BOARD"/DrawTile")
#define BTILEBACKGROUND    wxT(BOARD"/TileBG")
#define BTSTTILEBACKGROUND wxT(BOARD"/TstTileBG")

#define LIST               "List"
#define LISTFONT           wxT(LIST"/Font")

#define PRINT             "Print"
#define PHEADER           PRINT"/Header"
#define PRINTHFONT        wxT(PHEADER"/Font")
#define PTEXT             PRINT"/Text"
#define PRINTTFONT        wxT(PTEXT"/Font")

#define FRAME             "Frames/"
#define FRAMEBOARD        wxT(FRAME"Board")
#define FRAMEVERIF        wxT(FRAME"Verif")
#define FRAMESEARCH       wxT(FRAME"Search")
#define FRAMEPLUS1        wxT(FRAME"Plus1")
#define FRAMERACC         wxT(FRAME"Racc")
#define FRAMEBENJ         wxT(FRAME"Benj")
#define FRAMEBAG          wxT(FRAME"Bag")
#define FRAMEGAME         wxT(FRAME"Game")
#define FRAMERESULT       wxT(FRAME"Result")

enum Justif { LEFT, CENTER, RIGHT };

class ConfigDB
{
private:
  wxConfigBase* pConfig;
  enum Justif StrToJust(const wxString&);
  wxString JustToStr(enum Justif);

  enum Justif getJustif(const wxString&);
  void setJustif(const wxString&, enum Justif);

  bool Read(const wxString&, bool);
  long Read(const wxString&, long);
  wxFont Read(const wxString&, wxFont);
  wxColour Read(const wxString&, wxColour);
  wxString ReadStr(const wxString&, wxString);

  void Write(const wxString&, bool);
  void Write(const wxString&, long);
  void Write(const wxString&, wxFont);
  void Write(const wxString&, wxColour);
  void Write(const wxString&, wxString);

public:
  ConfigDB();

  wxFont ChooseFont(wxFrame*,wxFont);
  wxColour ChooseColour(wxFrame*,wxColour);

  // Dictionary
  wxString getDicPath();
  wxString getDicName();
  wxString getTilePath();
  wxString getTileName();
  void setDicPath(wxString name,wxString name);
  void setTilePath(wxString path,wxString name);

  // page setup
  long getOrientation();
  void setOrientation(long);

  // game drawing
  long getDxBegin();
  long getDxText(int);
  long getDxEnd();
  void setDxBegin(long);
  void setDxText(int,long);
  void setDxEnd(long);

  long getDyH1();
  long getDyH2();
  long getDyT1();
  long getDyT2();
  void setDyH1(long);
  void setDyH2(long);
  void setDyT1(long);
  void setDyT2(long);

  int  getSpacesH(int);
  int  getSpacesT(int);
  void setSpacesH(int,int);
  void setSpacesT(int,int);

  enum Justif getJustifH(int);
  enum Justif getJustifT(int);
  void setJustifH(int, enum Justif);
  void setJustifT(int, enum Justif);

  wxString getNameH(int);
  void setNameH(int, wxString);

  long getMarginX();
  long getMarginY();
  void setMarginX(long);
  void setMarginY(long);

  wxFont getFont(wxString);
  void setFont(wxString,wxFont);
  void setFontDefault();

  wxColour getColour(wxString);
  void setColour(wxString,wxColour);
  void setColourDefault();

  wxPrintData getPrintData();
  void setPrintData(wxPrintData);
  wxPageSetupData getPageSetupData();
  void setPageSetupData(wxPageSetupData);

  /**
   * frames
   */
  wxSize  getFrameSize(wxString);
  wxPoint getFramePos(wxString);
  int     getFrameShow(wxString);
  void setFrameSize(wxString,wxSize);
  void setFramePos(wxString,wxPoint);
  void setFrameShow(wxString,int);
  void setFrameDefault();

  void setRackChecking(bool);
  bool getRackChecking();

  void setJokerPlus1(bool);
  bool getJokerPlus1();

  void setDrawTile(bool);
  bool getDrawTile();

  float getPrintLineScale();
  void setPrintLineScale(float);

  void setFirstDefault();

};

#endif

