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

#include "wx/printdlg.h"

#include "ewx.h"

#include "dic.h"
#include "game.h"

#include "configdb.h"
#include "printout.h"
#include "confdimdlg.h"

enum {
  Button_Ok,
  Button_Cancel,
  Button_Printer,
  Button_Page,
  Button_Save,
  Button_FontHeader,
  Button_FontText
};

BEGIN_EVENT_TABLE(ConfDimDlg,wxDialog)
  EVT_CLOSE (ConfDimDlg::OnCloseWindow)
  EVT_BUTTON(Button_Ok,         ConfDimDlg::OnButtonOk)
  EVT_BUTTON(Button_Save,       ConfDimDlg::OnButtonSave)
  EVT_BUTTON(Button_Ok,         ConfDimDlg::OnButtonOk)
  EVT_BUTTON(Button_Cancel,     ConfDimDlg::OnButtonCancel)
  EVT_BUTTON(Button_Printer,    ConfDimDlg::OnConfPrinter)
  EVT_BUTTON(Button_Page,       ConfDimDlg::OnConfPage)
  EVT_BUTTON(Button_FontHeader, ConfDimDlg::OnConfFontHead)
  EVT_BUTTON(Button_FontText,   ConfDimDlg::OnConfFontText)
END_EVENT_TABLE()


static int
max(int i,int j)
{
  return i>j ? i : j;
}


ConfDimDlg::ConfDimDlg(wxWindow* parent, wxPrintData pd, wxPageSetupData psd)
  : wxDialog(parent, -1, wxString(wxT("Eliot : Impression")))
{
  int i;
  wxString choices[3];
  choices[0] = wxT("gauche");
// XXX:  choices[1] = wxT("centré");
  choices[1] = wxT("centre");
  choices[2] = wxT("droite");
  wxStaticText* Hcomment[5];
  wxRect        Hcommentrect[5];
  wxRect        Htitlerect[5];
  wxRect        Hjustrect[5];
  wxRect        Hspacesrect[5];
  wxStaticText* Tcomment[5];
  wxRect        Tcommentrect[5];
  wxRect        Tdimrect[5];
  wxStaticText* Tunit[5];
  wxRect        Tjustrect[5];
  wxRect        Tspacesrect[5];

  printdata = pd;
  pagesetupdata = psd;

  //
  //     Head
  //        Hcomment Htitle Hjust Hspaces
  //     Text
  //        TComment Tdim Tjust Tspaces
  //

#define VSPACE       10
#define HSPACE       5

  // --------------
  // Header Section
  // --------------
#define HFONT        wxPoint(HSPACE,VSPACE)
#define HCOMMENTY(i) (i==0 ? bfontheadrect.GetBottom() + VSPACE : \
                     VSPACE + Hcommentrect[i-1].GetBottom())
#define HCOMMENT(i)  wxPoint(HSPACE,HCOMMENTY(i)+2)
#define HTITLE(i)    wxPoint(2*HSPACE+max(Hcommentrect[0].GetRight(), \
                         Tcommentrect[0].GetRight()),HCOMMENTY(i))
#define HJUST(i)     wxPoint(Htitlerect[0].GetRight() + HSPACE,HCOMMENTY(i))
#define HSPACES(i)   wxPoint(Hjustrect[0].GetRight() + HSPACE,HCOMMENTY(i))

  // ------------
  // Text Section
  // ------------
#define TFONT        wxPoint(HSPACE, Hcommentrect[4].GetBottom() + 2*VSPACE)
#define TCOMMENTY(i) (i==0 ? bfonttextrect.GetBottom() + VSPACE : \
		      VSPACE + Tcommentrect[i-1].GetBottom())
#define TCOMMENT(i)  wxPoint(HSPACE, TCOMMENTY(i)+2)
#define TDIM(i)      wxPoint(2*HSPACE+max(Tcommentrect[0].GetRight(), \
                         Hcommentrect[0].GetRight()),TCOMMENTY(i))
#define TJUST(i)     wxPoint(Htitlerect[0].GetRight() + HSPACE, TCOMMENTY(i))
#define TSPACES(i)   wxPoint(Hjustrect[0].GetRight() + HSPACE, TCOMMENTY(i))


#define ESPSIZE      wxSize(spacerect.GetRight() - spacerect.GetLeft(),-1)


  wxStaticText* justif = new wxStaticText(this,-1,wxT("Justification"),wxPoint(0,0));
  wxRect justifrect = justif->GetRect();
  wxStaticText* space = new wxStaticText(this,-1,wxT("Espacement"),wxPoint(0,0));
  wxRect spacerect = space->GetRect();

  // heading, first part
// XXX:  bfontheader = new wxButton(this,Button_FontHeader,wxT("Caractères"),HFONT);
  bfontheader = new wxButton(this,Button_FontHeader,wxT("Caracteres"),HFONT);
  wxRect bfontheadrect = bfontheader->GetRect();
  for(i=0; i<5; i++)
    {
      wxString txt;
      txt << wxT("Titre colonne ") << (i+1);
      Hcomment[i] = new wxStaticText(this,-1,txt,HCOMMENT(i));
      Hcommentrect[i] = Hcomment[i]->GetRect();
    }

  // text, first part
// XXX:  bfonttext = new wxButton(this,Button_FontText,wxT("Caractères"),TFONT);
  bfonttext = new wxButton(this,Button_FontText,wxT("Caracteres"),TFONT);
  wxRect bfonttextrect = bfonttext->GetRect();
  for(i=0; i<5; i++)
    {
      wxString txt;
      txt << wxT("Texte colonne ") << (i+1);
      Tcomment[i] = new wxStaticText(this,-1,txt,TCOMMENT(i),wxSize(-1,-1));
      Tcommentrect[i] = Tcomment[i]->GetRect();
    }

  // heading, second part
  for(i=0; i<5; i++)
    {
      Htitle[i] = new wxTextCtrl(this,-1,wxT(""),HTITLE(i),wxSize(100,-1));
      Htitlerect[i] = Htitle[i]->GetRect();
      Htitle[i]->SetToolTip(wxT("Texte du titre de la colonne"));

      Hjust[i] = new wxChoice(this,-1,HJUST(i),wxSize(-1,-1),3,choices);
      Hjustrect[i] = Hjust[i]->GetRect();
      Hjust[i]->SetToolTip(wxT("Justification du titre de la colonne"));

      Hspaces[i] = new wxTextCtrl(this,-1,wxT("00"),HSPACES(i),ESPSIZE);
      Hspacesrect[i] = Hspaces[i]->GetRect();
// XXX:      Hspaces[i]->SetToolTip(wxT("Espacement des caractères du titre"));
      Hspaces[i]->SetToolTip(wxT("Espacement des caracteres du titre"));
    }


  // text, second part
  for(i=0; i<5; i++)
    {
      Tdim[i] = new wxTextCtrl(this,-1,wxT(""),TDIM(i),wxSize(50,-1));
      Tdimrect[i] = Tdim[i]->GetRect();
// XXX:      Tdim[i]->SetToolTip(wxT("Dimension intérieure de la colonne (en mm)"));
      Tdim[i]->SetToolTip(wxT("Dimension interieure de la colonne (en mm)"));

      Tunit[i] = new wxStaticText(this,-1,wxT("mm"),
				  wxPoint(Tdimrect[i].GetRight()+
					  HSPACE,TCOMMENTY(i)+2),
				  wxSize(-1,-1));

      Tjust[i] = new wxChoice(this,-1,TJUST(i),wxSize(-1,-1),3,choices);
      Tjustrect[i] = Tjust[i]->GetRect();
      Tjust[i]->SetToolTip(wxT("Justification du texte de la colonne"));

      Tspaces[i] = new wxTextCtrl(this,-1,wxT(""),TSPACES(i),ESPSIZE);
      Tspacesrect[i] = Tspaces[i]->GetRect();
// XXX:      Tspaces[i]->SetToolTip(wxT("Espacement des caractères"));
      Tspaces[i]->SetToolTip(wxT("Espacement des caracteres"));
    }

  justif->Move(wxPoint(Tjustrect[0].GetLeft(),bfontheadrect.GetBottom()
		       - (justifrect.GetBottom() - justifrect.GetTop())));
  justifrect = justif->GetRect();
  space->Move(wxPoint(Tspacesrect[0].GetLeft(),justifrect.GetTop()));
  spacerect = space->GetRect();



#define YPOS(i) (2*VSPACE + Tcommentrect[4].GetBottom() + \
      i*(VSPACE+Tspacesrect[0].GetBottom()-Tspacesrect[0].GetTop()))
  //#define YPOS(i) TCOMMENTY(i+1)
  //#define COL2START (4*HSPACE + Tspacesrect[0].GetRight())
#define COL2START HSPACE

  // 370x270 image

#define XPOSLEFT (HSPACE + max(dyh1textrect.GetRight(), \
        max(dyt1textrect.GetRight(), dxbegintextrect.GetRight() )))
#define XPOSRIGHT (HSPACE + max(dyh2textrect.GetRight(), \
        max(dyt2textrect.GetRight(), dxendtextrect.GetRight() )))

#define DYH1COMMENT wxPoint(COL2START, YPOS(0)+2)
#define DYH1TEXT wxPoint(XPOSLEFT, YPOS(0))
#define DYH1UNIT  wxPoint(dyh1rect.GetRight() + HSPACE, YPOS(0))

#define DYH2COMMENT wxPoint(3*HSPACE + dyh1mmrect.GetRight(), YPOS(0)+2)
#define DYH2TEXT wxPoint(XPOSRIGHT, YPOS(0))
#define DYH2UNIT wxPoint(dyh2rect.GetRight() + HSPACE, YPOS(0))

#define DYT1COMMENT wxPoint(COL2START, YPOS(1)+2)
#define DYT1TEXT wxPoint(XPOSLEFT, YPOS(1))
#define DYT1UNIT wxPoint(dyt1rect.GetRight() + HSPACE, YPOS(1))

#define DYT2COMMENT wxPoint(3*HSPACE + dyt1mmrect.GetRight(), YPOS(1)+2)
#define DYT2TEXT wxPoint(XPOSRIGHT, YPOS(1))
#define DYT2UNIT wxPoint(dyt2rect.GetRight() + HSPACE, YPOS(1))

#define DXBEGINCOMMENT wxPoint(COL2START, YPOS(2)+2)
#define DXBEGINTEXT wxPoint(XPOSLEFT, YPOS(2))
#define DXBEGINUNIT wxPoint(dxbeginrect.GetRight() + HSPACE, YPOS(2))

#define DXENDCOMMENT wxPoint(3*HSPACE + dxbeginmmrect.GetRight(), YPOS(2)+2)
#define DXENDTEXT wxPoint(XPOSRIGHT, YPOS(2))
#define DXENDUNIT wxPoint(dxendrect.GetRight() + HSPACE, YPOS(2))

#define DIM wxSize(30,-1)

  // Left part
  wxStaticText* dyh1text = new wxStaticText(this,-1,wxT("Titre esp. sup."),DYH1COMMENT,wxSize(-1,-1));
  wxRect dyh1textrect = dyh1text->GetRect();
  wxStaticText* dyt1text = new wxStaticText(this,-1,wxT("Texte esp. sup."),DYT1COMMENT,wxSize(-1,-1));
  wxRect dyt1textrect = dyt1text->GetRect();
  wxStaticText* dxbegintext = new wxStaticText(this,-1,wxT("Texte esp. gauche."),DXBEGINCOMMENT,wxSize(-1,-1));
  wxRect dxbegintextrect = dxbegintext->GetRect();

  dyh1 = new wxTextCtrl(this,-1,wxT("00"),DYH1TEXT,DIM);
  wxRect dyh1rect = dyh1->GetRect();
  wxStaticText* dyh1mm = new wxStaticText(this,-1,wxT("mm"),DYH1UNIT,wxSize(-1,-1));
  wxRect dyh1mmrect = dyh1mm->GetRect();
  dyt1 = new wxTextCtrl(this,-1,wxT("00"),DYT1TEXT,DIM);
  wxRect dyt1rect = dyt1->GetRect();
  wxStaticText* dyt1mm = new wxStaticText(this,-1,wxT("mm"),DYT1UNIT,wxSize(-1,-1));
  wxRect dyt1mmrect = dyt1mm->GetRect();
  dxbegin = new wxTextCtrl(this,-1,wxT("00"),DXBEGINTEXT,DIM);
  wxRect dxbeginrect = dxbegin->GetRect();
  wxStaticText* dxbeginmm = new wxStaticText(this,-1,wxT("mm"),DXBEGINUNIT,wxSize(-1,-1));
  wxRect dxbeginmmrect = dxbeginmm->GetRect();

  // Right part
  wxStaticText* dyh2text = new wxStaticText(this,-1,wxT("Titre esp. inf."),DYH2COMMENT,wxSize(-1,-1));
  wxRect dyh2textrect = dyh2text->GetRect();
  wxStaticText* dyt2text = new wxStaticText(this,-1,wxT("Texte esp. inf."),DYT2COMMENT,wxSize(-1,-1));
  wxRect dyt2textrect = dyt2text->GetRect();
  wxStaticText* dxendtext = new wxStaticText(this,-1,wxT("Texte esp. droit."),DXENDCOMMENT,wxSize(-1,-1));
  wxRect dxendtextrect = dxendtext->GetRect();

  dyh2 = new wxTextCtrl(this,-1,wxT("00"),DYH2TEXT,DIM);
  wxRect dyh2rect = dyh2->GetRect();
  wxStaticText* dyh2mm = new wxStaticText(this,-1,wxT("mm"),DYH2UNIT,wxSize(-1,-1));
  wxRect dyh2mmrect = dyh2mm->GetRect();
  dyt2 = new wxTextCtrl(this,-1,wxT("00"),DYT2TEXT,DIM);
  wxRect dyt2rect = dyt2->GetRect();
  wxStaticText* dyt2mm = new wxStaticText(this,-1,wxT("mm"),DYT2UNIT,wxSize(-1,-1));
  wxRect dyt2mmrect = dyt2mm->GetRect();
  dxend = new wxTextCtrl(this,-1,wxT("00"),DXENDTEXT,DIM);
  wxRect dxendrect = dxend->GetRect();
  wxStaticText* dxendmm = new wxStaticText(this,-1,wxT("mm"),DXENDUNIT,wxSize(-1,-1));
  wxRect dxendmmrect = dxendmm->GetRect();

  // shutdown warnings
  {int t = dyh2mmrect.GetTop() + dyt2mmrect.GetTop() + dxendmmrect.GetTop(); t++; }

#define BHSPACE       1
#define BPOS	      (2*VSPACE + dxendrect.GetBottom())

#define BPRINTERPOINT wxPoint(COL2START,BPOS)
#define BPAGEPOINT    wxPoint(bprinterrect.GetRight() + BHSPACE,BPOS)
#define BOKPOINT      wxPoint(bcancelrect.GetLeft() - (bokrect.GetRight() \
			       - bokrect.GetLeft() + BHSPACE),BPOS)
#define CANCELWIDTH   (bcancelrect.GetRight() - bcancelrect.GetLeft())
#define BCANCELPOINT wxPoint(Tspacesrect[0].GetRight() - CANCELWIDTH,BPOS)
  //#define BCANCELPOINT  wxPoint(dxendmmrect.GetRight() - CANCELWIDTH,BPOS)

  bprinter = new wxButton(this,Button_Printer,wxT("Imprimante"),BPRINTERPOINT);
  wxRect bprinterrect = bprinter->GetRect();
  bprinter->SetToolTip(wxT("Configurer l'imprimante"));

  bpage = new wxButton(this,Button_Page,wxT("Page"),BPAGEPOINT);
  //  wxRect bpagerect = bpage->GetRect();
  bpage->SetToolTip(wxT("Configurer la taille de page"));

  bcancel = new wxButton(this,Button_Cancel,wxT("Annuler"));
  wxRect bcancelrect = bcancel->GetRect();
  bcancel->Move(BCANCELPOINT);
  bcancelrect = bcancel->GetRect();
  bcancel->SetToolTip(wxT("Annuler les dernier changements et quitter"));

  bok = new wxButton(this,Button_Ok,wxT("OK"));
  wxRect bokrect = bok->GetRect();
  bok->Move(BOKPOINT);
  bokrect = bok->GetRect();
  bok->SetToolTip(wxT("Enregistrer les changements et quitter"));

#define DLGWIDTH     (bcancelrect.GetRight() + HSPACE)
#define DLGHEIGHT    (bokrect.GetBottom() + VSPACE)

  SetClientSize(DLGWIDTH,DLGHEIGHT);
  readconf();
}

ConfDimDlg::~ConfDimDlg()
{
}

void
ConfDimDlg::set(wxTextCtrl *t, long val)
{
  wxString str;
  str << (int)val;
  t->SetValue(str);
}

long
ConfDimDlg::get(wxTextCtrl* t)
{
  wxString str;
  str = t->GetValue();
  return atoi(str.mb_str());
}

void
ConfDimDlg::readconf()
{
  int i;
  for(i=0; i<5; i++) {
    Htitle[i]->SetValue(config.getNameH(i));
    switch (config.getJustifH(i)) {
    case LEFT: Hjust[i]->SetSelection(0); break;
    case CENTER: Hjust[i]->SetSelection(1); break;
    case RIGHT: Hjust[i]->SetSelection(2); break;
    }
    set(Hspaces[i],config.getSpacesH(i));
  }
  for(i=0; i<5; i++) {
    wxString str;
    str << (int)config.getDxText(i);
    Tdim[i]->SetValue(str);
    switch (config.getJustifT(i)) {
    case LEFT: Tjust[i]->SetSelection(0); break;
    case CENTER: Tjust[i]->SetSelection(1); break;
    case RIGHT: Tjust[i]->SetSelection(2); break;
    }
    set(Tspaces[i],config.getSpacesT(i));
  }
  set(dyh1,config.getDyH1());
  set(dyh2,config.getDyH2());
  set(dyt1,config.getDyT1());
  set(dyt2,config.getDyT2());
  set(dxbegin,config.getDxBegin());
  set(dxend,config.getDxEnd());

  headfont = config.getFont(PRINTHFONT);
  textfont = config.getFont(PRINTTFONT);
}

void
ConfDimDlg::writeconf()
{
  int i;
  for(i=0; i<5; i++) {
    config.setNameH(i,Htitle[i]->GetValue());
    switch (Hjust[i]->GetSelection()) {
    case -1: break;
    case 0: config.setJustifH(i,LEFT); break;
    case 1: config.setJustifH(i,CENTER); break;
    case 2: config.setJustifH(i,RIGHT); break;
    }
    config.setSpacesH(i,get(Hspaces[i]));
  }
  for(i=0; i<5; i++) {
    wxString str;
    str = Tdim[i]->GetValue();
    config.setDxText(i, atoi(str.mb_str()));
    switch (Tjust[i]->GetSelection()) {
    case -1: break;
    case 0: config.setJustifT(i,LEFT); break;
    case 1: config.setJustifT(i,CENTER); break;
    case 2: config.setJustifT(i,RIGHT); break;
    }
    config.setSpacesT(i,get(Tspaces[i]));
  }
  config.setDyH1(get(dyh1));
  config.setDyH2(get(dyh2));
  config.setDyT1(get(dyt1));
  config.setDyT2(get(dyt2));
  config.setDxBegin(get(dxbegin));
  config.setDxEnd(get(dxend));

  config.setFont(PRINTHFONT,headfont);
  config.setFont(PRINTTFONT,textfont);
}

void
ConfDimDlg::OnCloseWindow(wxCloseEvent __UNUSED__ &event)
{
  if (IsModal() == TRUE)
    EndModal(1);
  else
    this->Destroy();
}

void
ConfDimDlg::OnButtonOk(wxCommandEvent __UNUSED__ &event)
{
  writeconf();
  if (IsModal() == TRUE)
    EndModal(1);
  else
    this->Destroy();
}

void
ConfDimDlg::OnButtonCancel(wxCommandEvent __UNUSED__ &event)
{
  if (IsModal() == TRUE)
    EndModal(1);
  else
    this->Destroy();
}

void
ConfDimDlg::OnButtonSave(wxCommandEvent __UNUSED__ &event)
{
  writeconf();
}

void
ConfDimDlg::OnConfPage(wxCommandEvent __UNUSED__ &event)
{
  //#if defined(__WXGTK__)
  pagesetupdata = printdata;
  wxPageSetupDialog pageSetupDialog(this, &pagesetupdata);
  pageSetupDialog.ShowModal();
  printdata = pageSetupDialog.GetPageSetupData().GetPrintData();
  pagesetupdata = pageSetupDialog.GetPageSetupData();
//  #else
//    wxPageSetupDialog pageSetupDialog(this, & pagesetupdata);
//    pageSetupDialog.ShowModal();
//    pagesetupdata = pageSetupDialog.GetPageSetupData();
//  #endif
}

void
ConfDimDlg::OnConfPrinter(wxCommandEvent __UNUSED__ &event)
{
  //#if defined(__WXGTK__)
  wxPrintDialogData printDialogData(printdata);
  wxPrintDialog printerDialog(this, & printDialogData);
  printerDialog.GetPrintDialogData().SetSetupDialog(TRUE);
  printerDialog.ShowModal();
  printdata = printerDialog.GetPrintDialogData().GetPrintData();
//  #else
//    wxPrintDialog printerDialog(this, & printdata);
//    printerDialog.GetPrintData().SetSetupDialog(TRUE);
//    printerDialog.ShowModal();
//    printdata = printerDialog.GetPrintData();
//  #endif
}

void
ConfDimDlg::OnConfFontHead(wxCommandEvent __UNUSED__ &event)
{
  headfont = config.ChooseFont((wxFrame*)this,headfont);
}

void
ConfDimDlg::OnConfFontText(wxCommandEvent __UNUSED__ &event)
{
  textfont = config.ChooseFont((wxFrame*)this,textfont);
}

wxPrintData
ConfDimDlg::getPrintData()
{
  return printdata;
}

wxPageSetupData
ConfDimDlg::getPageSetupData()
{
  return pagesetupdata;
}
