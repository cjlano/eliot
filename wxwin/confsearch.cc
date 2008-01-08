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

#include "ewx.h"
#include "wx/wx.h"
#include "wx/sizer.h"
#include "confsearch.h"


BEGIN_EVENT_TABLE(ConfSearchDlg, wxDialog)
  EVT_CLOSE    (ConfSearchDlg::OnCloseWindow)
  EVT_BUTTON   (wxID_OK,      ConfSearchDlg::OnButtonOk)
  EVT_BUTTON   (wxID_CANCEL,  ConfSearchDlg::OnButtonCancel)
END_EVENT_TABLE()


ConfSearchDlg::ConfSearchDlg(wxWindow* parent)
    : wxDialog(parent, -1, wxString(wxT("Configuration recherche")))
{

    joker_searching = new wxCheckBox(this, wxID_ANY, _("Search on joker in 7+1 panel"));
    rack_checking = new wxCheckBox(this, wxID_ANY, _("Check rack validity"));

    bcancel = new wxButton(this, wxID_CANCEL);
    bcancel->SetToolTip(_("Cancel last changes"));
    bok = new wxButton(this, wxID_OK);
    bok->SetToolTip(_("Save the changes"));

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

