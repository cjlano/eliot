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

/* $Id: confsearch.cc,v 1.1 2004/04/08 09:43:06 afrab Exp $ */

#include "ewx.h"
#include "configdb.h"
#include "confsearch.h"

#include "wx/button.h"
#include "wx/sizer.h"

enum {
	Button_Ok,
	Button_Cancel,
	CheckBox_Joker,
	CheckBox_Rack
};

BEGIN_EVENT_TABLE(ConfSearchDlg,wxDialog)
  EVT_CLOSE    (ConfSearchDlg::OnCloseWindow)
  EVT_BUTTON   (Button_Ok,      ConfSearchDlg::OnButtonOk)
  EVT_BUTTON   (Button_Cancel,  ConfSearchDlg::OnButtonCancel)
END_EVENT_TABLE()


ConfSearchDlg::ConfSearchDlg(wxWindow* parent)
   	: wxDialog(parent,-1,wxString("Configuration recherche"))
{

  joker_searching = new wxCheckBox(this,CheckBox_Joker,"Recherche sur joker dans 7+1");
  rack_checking = new wxCheckBox(this,CheckBox_Rack,"Vérification de la validité des tirages");
  
  bcancel = new wxButton(this,Button_Cancel,"Annuler",wxPoint(-1,-1));
  bcancel->SetToolTip("Annuler les dernier changements et quitter");
  bok = new wxButton(this,Button_Ok,"Quitter",wxPoint(-1,-1));
  bok->SetToolTip("Enregistrer les changements et quitter");

  wxBoxSizer *bsizer = new wxBoxSizer( wxHORIZONTAL);
  bsizer->Add(bok, 1, wxALL, 1);
  bsizer->Add(bcancel, 1, wxALL, 1);

  wxBoxSizer *sizer = new wxBoxSizer( wxVERTICAL );
  sizer->Add(joker_searching, 1, wxEXPAND | wxALL, 2);
  sizer->Add(rack_checking, 1, wxEXPAND | wxALL, 2);
  sizer->Add(bsizer, 0, wxEXPAND, 0);

  SetAutoLayout(TRUE);
  SetSizer(sizer);
  sizer->Fit(this);
  sizer->SetSizeHints(this);
  readconf();
}

void
ConfSearchDlg::readconf()
{
  joker_searching->SetValue(config.getJokerPlus1());
  rack_checking->SetValue(config.getRackChecking());
}

void
ConfSearchDlg::writeconf()
{
  config.setJokerPlus1(joker_searching->GetValue());
  config.setRackChecking(rack_checking->GetValue());
}

void
ConfSearchDlg::OnCloseWindow(wxCloseEvent&)
{
  if (IsModal() == TRUE)
    EndModal(1);
  else
    this->Destroy();
}

void
ConfSearchDlg::OnButtonOk(wxCommandEvent&)
{
  writeconf();
  if (IsModal() == TRUE)
    EndModal(1);
  else
    this->Destroy();
}

void
ConfSearchDlg::OnButtonCancel(wxCommandEvent&)
{
  if (IsModal() == TRUE)
    EndModal(1);
  else
    this->Destroy();
}

