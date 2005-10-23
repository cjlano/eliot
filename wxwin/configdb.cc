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

/* $Id: configdb.cc,v 1.5 2005/10/23 14:53:44 ipkiss Exp $ */

#include <iostream>
#include "ewx.h"
#include "configdb.h"
#include "wx/colordlg.h"
#include "wx/fontdlg.h"
#include "wx/settings.h"

using namespace std;

#define DIM 200
#define PREFIX  "/"

#define DICPATH  wxT(PREFIX"Fichiers/Dictionnaire_Chemin")
#define DICNAME  wxT(PREFIX"Fichiers/Dictionnaire_Nom")
#define TILEPATH wxT(PREFIX"Fichiers/Lettres_Chemin")
#define TILENAME wxT(PREFIX"Fichiers/Lettres_Nom")

///////////////////////////
//
// Print
//
///////////////////////////
#define ORIENT  wxT(PREFIX"Print/Page_Orientation")
#define MARGINX wxT(PREFIX"Print/Page_MargeGauche")
#define MARGINY wxT(PREFIX"Print/Page_MargeHaute")

#define HNAME   wxT(PREFIX"Print/Header/Nom/")
#define HDIM    wxT(PREFIX"Print/Header/Dimensions/")
#define HJUST   wxT(PREFIX"Print/Header/Justification/")
#define HSPACE  wxT(PREFIX"Print/Header/Espacement/")

#define TDIM    wxT(PREFIX"Print/Text/Dimensions/")
#define TJUST   wxT(PREFIX"Print/Text/Justification/")
#define TSPACE  wxT(PREFIX"Print/Text/Espacement/")

#define MISC    wxT(PREFIX"Divers/")

ConfigDB::ConfigDB()
{
   pConfig = wxConfigBase::Get();
}


////////////////////////////////////////////////////////
//
// Overload of the Read function
// bool, long, wxFont, wxColour, wxString
//
// bool HasEntry(wxString&)
// bool Exists(wxString&)
//
////////////////////////////////////////////////////////

bool ConfigDB::Read(const wxString& key, bool def)
{
  bool res;
  if (pConfig->Exists(key))
    pConfig->Read(key,&res,def);
  else
    res = def;
  return res;
}

long ConfigDB::Read(const wxString& key, long def)
{
  long res;
  if (pConfig->Exists(key))
    pConfig->Read(key,&res,def);
  else
    res = def;
  return res;
}

#define FPOINTSIZE wxT(".PointSize")
#define FFAMILY    wxT(".Family")
#define FSTYLE     wxT(".Style")
#define FWEIGHT    wxT(".Weight")
#define FUNDERLINE wxT(".Underline")
#define FFACENAME  wxT(".FaceName")
#define FENCODING  wxT(".Encoding")

wxFont ConfigDB::Read(const wxString& key, wxFont def)
{
  return wxFont(Read(key + FPOINTSIZE,(long)def.GetPointSize()),
		Read(key + FFAMILY   ,(long)def.GetFamily()),
		Read(key + FSTYLE    ,(long)def.GetStyle()),
		Read(key + FWEIGHT   ,(long)def.GetWeight()),
		Read(key + FUNDERLINE,(long)def.GetUnderlined()),
		ReadStr(key + FFACENAME ,def.GetFaceName())
		//,Read(key + FENCODING ,def.GetDefaultEncoding())
		);
}

void ConfigDB::Write(const wxString& key, wxFont font)
{
  pConfig->Write(key + FPOINTSIZE,(long)font.GetPointSize());
  pConfig->Write(key + FFAMILY   ,(long)font.GetFamily());
  pConfig->Write(key + FSTYLE    ,(long)font.GetStyle());
  pConfig->Write(key + FWEIGHT   ,(long)font.GetWeight());
  pConfig->Write(key + FUNDERLINE,(long)font.GetUnderlined());
  pConfig->Write(key + FFACENAME ,font.GetFaceName());
  //pConfig->Write(key + FENCODING ,font.GetDefaultEncoding());
}

#define CR wxT(".R")
#define CG wxT(".G")
#define CB wxT(".B")

wxColour ConfigDB::Read(const wxString& key, wxColour def)
{
    return wxColour(Read(key + CR,(long)def.Red()),
                    Read(key + CG,(long)def.Green()),
                    Read(key + CB,(long)def.Blue()));
}


void ConfigDB::Write(const wxString& key, wxColour colour)
{
    pConfig->Write(key + CR,(long)colour.Red());
    pConfig->Write(key + CG, (long)colour.Green());
    pConfig->Write(key + CB, (long)colour.Blue());
}

wxString ConfigDB::ReadStr(const wxString& key, wxString def)
{
  wxString res;

  /*
  wxString msg;
  msg << "want to read -" << key << "-";
  wxMessageBox(msg, "Eliot configDB", wxICON_INFORMATION | wxOK);
  cout << msg << "\n";
  */

  if (pConfig->Read(key,&res) == FALSE)
    res = def;
  return res;
}

////////////////////////////////////////////////////////
//
// Overload of the Write function
// bool, long, wxFont, wxColour, wxString
//
////////////////////////////////////////////////////////

void ConfigDB::Write(const wxString& key, bool val)
{
  pConfig->Write(key,val);
}

void ConfigDB::Write(const wxString& key, long val)
{
  pConfig->Write(key,val);
}


void ConfigDB::Write(const wxString& key, wxString str)
{
  pConfig->Write(key,str);
}

////////////////////////////////////////////////////////
//
// Dictionary
//
////////////////////////////////////////////////////////

wxString ConfigDB::getDicPath()
{
  return ReadStr(DICPATH,wxT("//"));
}

wxString ConfigDB::getDicName()
{
  return ReadStr(DICNAME,wxT("Aucun Dictionnaire"));
}

wxString ConfigDB::getTilePath()
{
  return ReadStr(TILEPATH,wxT("//"));
}

wxString ConfigDB::getTileName()
{
  return ReadStr(TILENAME,wxT(""));
}

void ConfigDB::setDicPath(wxString dpath, wxString dname)
{
  Write(DICPATH,dpath);
  Write(DICNAME,dname);
}

void ConfigDB::setTilePath(wxString dpath, wxString dname)
{
  Write(TILEPATH,dpath);
  Write(TILENAME,dname);
}

////////////////////////////////////////////////////////
//
// Text length
//
////////////////////////////////////////////////////////

long ConfigDB::getDxBegin()
{
  return Read(TDIM"debut",1L);
}

void ConfigDB::setDxBegin(long d)
{
  Write(TDIM"debut",d);
}

long ConfigDB::getDxText(int i)
{
  long int res;
  switch (i)
    {
    case 0: res = Read(TDIM"texte/1",10L); break;
    case 1: res = Read(TDIM"texte/2",30L); break;
    case 2: res = Read(TDIM"texte/3",30L); break;
    case 3: res = Read(TDIM"texte/4",10L); break;
    case 4: res = Read(TDIM"texte/5",10L); break;
    default:res = 0; break;
    }
  return res;
}

void ConfigDB::setDxText(int i, long v)
{
  wxString key;
  key << TDIM << wxT("texte/") << (i+1);
  if (i<5)
    Write(key,v);
}

long ConfigDB::getDxEnd()
{
  return Read(TDIM"fin",1L);
}

void ConfigDB::setDxEnd(long d)
{
  Write(TDIM"fin",d);
}

long ConfigDB::getDyT1()
{
  return Read(TDIM"haut",1L);
}

void ConfigDB::setDyT1(long d)
{
  Write(TDIM"haut",d);
}

long ConfigDB::getDyT2()
{
  return Read(TDIM"bas",1L);
}

void ConfigDB::setDyT2(long d)
{
  Write(TDIM"bas",d);
}

long ConfigDB::getDyH1()
{
  return Read(HDIM"haut",1L);
}

void ConfigDB::setDyH1(long d)
{
  Write(HDIM"haut",d);
}

long ConfigDB::getDyH2()
{
  return Read(HDIM"bas",1L);
}

void ConfigDB::setDyH2(long d)
{
  Write(HDIM"bas",d);
}

wxString ConfigDB::getNameH(int i)
{
  wxString res;
  switch (i)
    {
    case 0: res = ReadStr(wxString(HNAME) + wxT("1"), wxT("Num")); break;
    case 1: res = ReadStr(wxString(HNAME) + wxT("2"), wxT("Tirage")); break;
    case 2: res = ReadStr(wxString(HNAME) + wxT("3"), wxT("Solution")); break;
    case 3: res = ReadStr(wxString(HNAME) + wxT("4"), wxT("Pos")); break;
    case 4: res = ReadStr(wxString(HNAME) + wxT("5"), wxT("Pts")); break;
    default: res = wxT(""); break;
  }
  return res;
}

void ConfigDB::setNameH(int i, wxString str)
{
  wxString key;
  key << HNAME << (i+1);
  Write(key, str);
}

////////////////////////////////////////////////////////
//
// Text justification
//
////////////////////////////////////////////////////////

enum Justif ConfigDB::StrToJust(const wxString& str)
{
  enum Justif res;
  if (str.CmpNoCase(wxT("gauche")) == 0)
    res = LEFT;
  else if (str.CmpNoCase(wxT("centre")) == 0)
    res = CENTER;
  else if (str.CmpNoCase(wxT("droite")) == 0)
    res = RIGHT;
  else
    res = LEFT;
  return res;
}

wxString ConfigDB::JustToStr(enum Justif j)
{
  wxString res;
  switch (j) {
  case LEFT:   res = wxT("gauche"); break;
  case CENTER: res = wxT("centre"); break;
  case RIGHT:  res = wxT("droite"); break;
  }
  return res;
}

enum Justif ConfigDB::getJustif(const wxString& key)
{
  return StrToJust(ReadStr(key,wxT("gauche")));
}

void ConfigDB::setJustif(const wxString& key, enum Justif j)
{
  Write(key,JustToStr(j));
}

enum Justif ConfigDB::getJustifH(int i)
{
  wxString key;
  key << HJUST << (i+1);
  return getJustif(key);
}

enum Justif ConfigDB::getJustifT(int i)
{
  wxString key;
  key << TJUST << (i+1);
  return getJustif(key);
}

void ConfigDB::setJustifH(int i, enum Justif j)
{
  wxString key;
  key << HJUST << (i+1);
  setJustif(key,j);
}

void ConfigDB::setJustifT(int i, enum Justif j)
{
  wxString key;
  key << TJUST << (i+1);
  setJustif(key,j);
}

////////////////////////////////////////////////////////
//
// Text Spaces
//
////////////////////////////////////////////////////////

int ConfigDB::getSpacesH(int i)
{
  wxString key;
  key << HSPACE << (i+1);
  return Read(key,0L);
}

int ConfigDB::getSpacesT(int i)
{
  wxString key;
  key << TSPACE << (i+1);
  return Read(key,0L);
}

void ConfigDB::setSpacesH(int i, int spaces)
{
  wxString key;
  key << HSPACE << (i+1);
  Write(key,(long)spaces);
}

void ConfigDB::setSpacesT(int i, int spaces)
{
  wxString key;
  key << TSPACE << (i+1);
  Write(key,(long)spaces);
}

////////////////////////////////////////////////////////
//
// Fonts
//
////////////////////////////////////////////////////////

wxFont ConfigDB::ChooseFont(wxFrame* frame,wxFont initfont)
{
  wxFont retfont = initfont;
  wxFontData data;
  data.SetInitialFont(initfont);
  wxFontDialog *dialog = new wxFontDialog(frame, &data);
  if (dialog->ShowModal() == wxID_OK) {
    wxFontData retdata = dialog->GetFontData();
    retfont = retdata.GetChosenFont();
  }
  dialog->Close();
  return retfont;
}

#define FHEADERDEF wxFont(12, wxSWISS, wxNORMAL, wxBOLD)
#define FTEXTDEF wxFont(10, wxSWISS, wxNORMAL, wxNORMAL)

void ConfigDB::setFontDefault()
{
  wxFont fsys;
  fsys = wxSystemSettings::GetSystemFont(wxSYS_DEFAULT_GUI_FONT);

  setFont(BOARDFONT ,fsys);
  setFont(LISTFONT  ,fsys);
  setFont(PRINTHFONT,FHEADERDEF);
  setFont(PRINTTFONT,FTEXTDEF);
}

void ConfigDB::setFont(wxString key, wxFont font)
{
  Write(key,font);
}

wxFont ConfigDB::getFont(wxString key)
{
  return Read(key,wxFont(12,wxMODERN,wxNORMAL,wxNORMAL));
}

////////////////////////////////////////////////////////
//
// Colours
//
////////////////////////////////////////////////////////

wxColour ConfigDB::ChooseColour(wxFrame* frame,wxColour initcolour)
{
     wxColour retcolour = initcolour;
     wxColourData data;
     data.SetColour(initcolour);
     wxColourDialog *dialog = new wxColourDialog(frame, &data);
     if (dialog->ShowModal() == wxID_OK) {
	  wxColourData retdata = dialog->GetColourData();
	  retcolour = retdata.GetColour();
     }
     dialog->Close();
     return retcolour;
}

#define LINESDEF   wxColour(101,101,101)
#define WX2DEF     wxColour(255,147,196)
#define WX3DEF     wxColour(240, 80, 94)
#define LX2DEF     wxColour( 34,189,240)
#define LX3DEF     wxColour( 29,104,240)
#define BACKDEF    wxColour(255,255,255)
#define LETTDEF    wxColour(  0,  0,  0)
#define TSTLETTDEF wxColour(  0,  0,  0)

void ConfigDB::setColourDefault()
{
  setColour(wxString(BCOLOURLINES),LINESDEF);
  setColour(wxString(BCOLOURWX2),WX2DEF);
  setColour(wxString(BCOLOURWX3),WX3DEF);
  setColour(wxString(BCOLOURLX2),LX2DEF);
  setColour(wxString(BCOLOURLX3),LX3DEF);
  setColour(wxString(BCOLOURBACKGROUND),BACKDEF);
  setColour(wxString(BCOLOURLETTERS),LETTDEF);
}

void
ConfigDB::setColour(wxString key, wxColour col)
{
  Write(key,col);
}

wxColour
ConfigDB::getColour(wxString key)
{
  return Read(key,wxColour(0,0,0));
}

////////////////////////////////////////////////////////
//
// PRINTING
//
////////////////////////////////////////////////////////

long ConfigDB::getMarginX()
{
  return Read(MARGINX,10L);
}

long ConfigDB::getMarginY()
{
  return Read(MARGINY,10L);
}

void ConfigDB::setMarginX(long x)
{
  Write(MARGINX,x);
}

void ConfigDB::setMarginY(long y)
{
  Write(MARGINY,y);
}

long ConfigDB::getOrientation()
{
  long res;
  wxString str;

  str = ReadStr(ORIENT,wxT("paysage"));
  if (str.CmpNoCase(wxT("portrait")) == 0)
    res = wxPORTRAIT;
  else if (str.CmpNoCase(wxT("paysage")) == 0)
    res = wxLANDSCAPE;
  else
    res = wxPORTRAIT;
  return res;
}

void ConfigDB::setOrientation(long o)
{
  switch (o)
    {
    case wxLANDSCAPE: Write(ORIENT,wxT("paysage")); break;
    case wxPORTRAIT: // fall through
    default: Write(ORIENT,wxT("portrait")); break;
    }
}

wxPrintData ConfigDB::getPrintData()
{
  wxPrintData pd;
  pd.SetOrientation(getOrientation());
  return pd;
}

void ConfigDB::setPrintData(wxPrintData pd)
{
  setOrientation(pd.GetOrientation());
}

wxPageSetupData ConfigDB::getPageSetupData()
{
  wxPageSetupData pd;
  wxPoint margin(getMarginX(),getMarginY());
  pd.SetMarginTopLeft(margin);
  return pd;
}

void ConfigDB::setPageSetupData(wxPageSetupData pd)
{
  setMarginX(pd.GetMarginTopLeft().x);
  setMarginY(pd.GetMarginTopLeft().y);
}

float ConfigDB::getPrintLineScale()
{
  return 0.2;
}

void ConfigDB::setPrintLineScale(float s)
{
}

////////////////////////////////////////////////////////
//
// Frame dimensions
//
////////////////////////////////////////////////////////

#define PX wxT("/x")
#define PY wxT("/y")
#define SW wxT("/w")
#define SH wxT("/h")
#define SHOW wxT("/show")

#define CONFIG_DEFAULT_X 150L
#define CONFIG_DEFAULT_Y 150L
#define CONFIG_DEFAULT_W 150L
#define CONFIG_DEFAULT_H 200L

wxPoint ConfigDB::getFramePos(wxString frame)
{
  wxPoint pos;
  wxString keyX(frame + PX);
  wxString keyY(frame + PY);

  pos.x = Read(keyX,CONFIG_DEFAULT_X);
  pos.x = pos.x < 0 ? 0 : pos.x;

  pos.y = Read(keyY,CONFIG_DEFAULT_Y);
  pos.y = pos.y < 0 ? 0 : pos.y;
#ifdef FRAME_TRACE
  cerr << "configdb::getFramePos  " << frame
       << " \tx:" << pos.x << " y:" << pos.y << endl;
#endif
  return pos;
}

void ConfigDB::setFramePos(wxString frame, wxPoint pos)
{
#ifdef FRAME_TRACE
  cerr << "configdb::setFramePos  " << frame
       << " \tx:" << pos.x << " y:" << pos.y << endl;
#endif
  wxString keyX(frame + PX);
  wxString keyY(frame + PY);
  Write(keyX,(long) (pos.x < 0 ? 0 : pos.x));
  Write(keyY,(long) (pos.y < 0 ? 0 : pos.y));
}

wxSize ConfigDB::getFrameSize(wxString frame)
{
  wxSize size;
  wxString keyX(frame + SW);
  wxString keyY(frame + SH);

  size.x = Read(keyX,CONFIG_DEFAULT_W);
  size.x = size.x < 0 ? 0 : size.x;

  size.y = Read(keyY,CONFIG_DEFAULT_H);
  size.y = size.y < 0 ? 0 : size.y;
#ifdef FRAME_TRACE
  cerr << "configdb::getFrameSize " << frame
       << " \tw:" << size.x << " h:" << size.y << endl;
#endif
  return size;
}

void ConfigDB::setFrameSize(wxString frame, wxSize size)
{
#ifdef FRAME_TRACE
  cerr << "configdb::setFrameSize " << frame
       << " \tw:" << size.x << " h:" << size.y << endl;
#endif
  wxString keyX(frame + SW);
  wxString keyY(frame + SH);
  Write(keyX,(long) (size.x < 0 ? 0 : size.x));
  Write(keyY,(long) (size.y < 0 ? 0 : size.y));
}

int ConfigDB::getFrameShow(wxString frame)
{
  wxString key(frame + SHOW);
  return Read(key,0L);
}

void ConfigDB::setFrameShow(wxString frame, int s)
{
  wxString key(frame + SHOW);
  Write(key,(long)s);
}

void ConfigDB::setFrameDefault()
{
  setFrameSize(FRAMEBOARD  ,wxSize(450,450));
  setFrameSize(FRAMEVERIF  ,wxSize(150,50));
  setFrameSize(FRAMESEARCH ,wxSize(350,300));
  setFrameSize(FRAMEPLUS1  ,wxSize(CONFIG_DEFAULT_W,CONFIG_DEFAULT_H));
  setFrameSize(FRAMERACC   ,wxSize(CONFIG_DEFAULT_W,CONFIG_DEFAULT_H));
  setFrameSize(FRAMEBENJ   ,wxSize(CONFIG_DEFAULT_W,CONFIG_DEFAULT_H));
  setFrameSize(FRAMEBAG    ,wxSize(150,40));
  setFrameSize(wxT(APPNAME),wxSize(410,200));

  setFramePos(FRAMEBOARD  ,wxPoint(58,76));
  setFramePos(FRAMEVERIF  ,wxPoint(CONFIG_DEFAULT_X,CONFIG_DEFAULT_Y));
  setFramePos(FRAMESEARCH ,wxPoint(CONFIG_DEFAULT_X,CONFIG_DEFAULT_Y));
  setFramePos(FRAMEPLUS1  ,wxPoint(490,300));
  setFramePos(FRAMERACC   ,wxPoint(CONFIG_DEFAULT_X,CONFIG_DEFAULT_Y));
  setFramePos(FRAMEBENJ   ,wxPoint(CONFIG_DEFAULT_X,CONFIG_DEFAULT_Y));
  setFramePos(FRAMEBAG    ,wxPoint(CONFIG_DEFAULT_X,CONFIG_DEFAULT_Y));
  setFramePos(wxT(APPNAME),wxPoint(500,9));

  setFrameShow(FRAMEBOARD ,1L);
  setFrameShow(FRAMEVERIF ,0L);
  setFrameShow(FRAMESEARCH,0L);
  setFrameShow(FRAMEPLUS1 ,1L);
  setFrameShow(FRAMERACC  ,0L);
  setFrameShow(FRAMEBENJ  ,0L);
  setFrameShow(FRAMEBAG   ,0L);
#ifdef FRAME_TRACE
  cerr << endl;
#endif
}

////////////////////////////////////////////////////////
//
// Game Options
//
////////////////////////////////////////////////////////

void ConfigDB::setJokerPlus1(bool val)
{
  wxString key;
  key = wxString(MISC) + wxT("JokersDans7plus1");
  Write(key,val);
}

bool ConfigDB::getJokerPlus1()
{
  wxString key;
  key = wxString(MISC) + wxT("JokersDans7plus1");
  return Read(key,(bool)FALSE);
}

void ConfigDB::setRackChecking(bool val)
{
  wxString key;
  key = wxString(MISC) + wxT("VerificationTirages");
  Write(key,val);
}

bool ConfigDB::getRackChecking()
{
  wxString key;
  key = wxString(MISC) + wxT("VerificationTirages");
  return Read(key,(bool)FALSE);
}

////////////////////////////////////////////////////////
//
//
//
////////////////////////////////////////////////////////

#define INIT wxT("/Initialized")

void
ConfigDB::setFirstDefault()
{
  if (Read(wxString(INIT),0L))
    return;

  setFontDefault();
  setColourDefault();
  setFrameDefault();

  Write(wxString(INIT),1L);
}

