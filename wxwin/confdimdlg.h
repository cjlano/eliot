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

/* $Id: confdimdlg.h,v 1.2 2005/10/23 14:53:44 ipkiss Exp $ */

#ifndef _CONFDIMDLG_H
#define _CONFDIMDLG_H

#include "wx/textctrl.h"
#include "wx/stattext.h"
#include "wx/choice.h"
#include "wx/button.h"
#include "wx/dialog.h"

class ConfDimDlg : public wxDialog
{
private:
  ConfigDB config;
  wxPrintData printdata;
  wxPageSetupData pagesetupdata;

  wxFont headfont;
  wxFont textfont;

  wxButton *bok;
  wxButton *bcancel;
  wxButton *bprinter;
  wxButton *bpage;
  wxButton *bfontheader;
  wxButton *bsave;
  wxButton *bfonttext;

// Heading
  wxTextCtrl*   Htitle[5];
  wxChoice*     Hjust[5];
  wxTextCtrl*   Hspaces[5];
// Text
  wxTextCtrl*   Tdim[5];
  wxChoice*     Tjust[5];
  wxTextCtrl*   Tspaces[5];
// Dim
  wxTextCtrl*   dxbegin;
  wxTextCtrl*   dxend;
  wxTextCtrl*   dyt1;
  wxTextCtrl*   dyt2;
  wxTextCtrl*   dyh1;
  wxTextCtrl*   dyh2;

  void readconf();
  void writeconf();
  void set(wxTextCtrl*,long);
  long get(wxTextCtrl*);

public:
  ConfDimDlg(wxWindow* parent, wxPrintData, wxPageSetupData);
  ~ConfDimDlg();
  void OnButtonOk(wxCommandEvent& event);
  void OnButtonCancel(wxCommandEvent& event);
  void OnButtonSave(wxCommandEvent& event);
  void OnCloseWindow  (wxCloseEvent& event);
  void OnConfFontHead(wxCommandEvent& event);
  void OnConfFontText(wxCommandEvent& event);
  void OnConfPage(wxCommandEvent& event);
  void OnConfPrinter(wxCommandEvent& event);
  
  wxPrintData getPrintData();
  wxPageSetupData getPageSetupData();

  DECLARE_EVENT_TABLE()
};
#endif
