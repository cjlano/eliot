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

/* $Id: configdb.h,v 1.1 2004/04/08 09:43:06 afrab Exp $ */

// -*-C++-*-

#ifndef _CONFIGDB_H
#define _CONFIGDB_H

#include "wx/dc.h"
#include "wx/font.h"
#include "wx/colour.h"
#include "wx/frame.h"
#include "wx/cmndata.h"
#include "wx/config.h"

#define BUTTON_FONT wxFont(8,wxDEFAULT,wxNORMAL,wxNORMAL)

#define BOARD             "Board"
#define BCOLOURLINES      BOARD"/Lines"
#define BCOLOURWX2        BOARD"/Wx2"
#define BCOLOURWX3        BOARD"/Wx3"
#define BCOLOURLX2        BOARD"/Lx2"
#define BCOLOURLX3        BOARD"/Lx3"
#define BCOLOURBACKGROUND BOARD"/Background"
#define BCOLOURLETTERS    BOARD"/Letters"
#define BCOLOURTSTLETTERS BOARD"/TstLetters"
#define BOARDFONT         BOARD"/Font"

#define LIST              "List"
#define LISTFONT          LIST"/Font"

#define PRINT             "Print"
#define PHEADER           PRINT"/Header"
#define PRINTHFONT        PHEADER"/Font"
#define PTEXT             PRINT"/Text"
#define PRINTTFONT        PTEXT"/Font"        

#define FRAME             "Frames/"
#define FRAMEBOARD        FRAME"Board"
#define FRAMEVERIF        FRAME"Verif"
#define FRAMESEARCH       FRAME"Search"
#define FRAMEPLUS1        FRAME"Plus1"
#define FRAMERACC         FRAME"Racc"
#define FRAMEBENJ         FRAME"Benj"
#define FRAMEBAG          FRAME"Bag"

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

  void setJokerPlus1(bool);
  void setRackChecking(bool);
  bool getJokerPlus1();
  bool getRackChecking();

  float getPrintLineScale();
  void setPrintLineScale(float);

  void setFirstDefault();
    
};

#endif

