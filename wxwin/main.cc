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

/* $Id: main.cc,v 1.5 2005/02/05 11:14:56 ipkiss Exp $ */

#ifdef WIN32 // mingw32 hack
#   undef Yield
#   undef CreateDialog
#endif

#include <stdlib.h>
#include <time.h>
#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/app.h>
#include <wx/intl.h>

#include "ewx.h"
#include "configdb.h"
#include "mainframe.h"

#include "eliot.xpm"

class EliotApp : public wxApp
{
private:
protected:
#ifdef ENABLE_LOCALE
  wxLocale locale;
#endif
public:
  virtual bool OnInit();
  virtual int  OnExit();
};

IMPLEMENT_APP(EliotApp)

bool
EliotApp::OnInit()
{
    srand(time(NULL));
    SetVendorName(wxT("Afrab"));
    SetAppName(wxString(wxT("eliot")) + wxT("-") + wxT(VERSION));
    SetClassName(wxT("eliot"));

    wxConfigBase* config = wxConfigBase::Get();
    config = NULL;
#ifdef ENABLE_LOCALE
    locale.Init(wxLocale::GetSystemLanguage(),
                wxLOCALE_LOAD_DEFAULT | wxLOCALE_CONV_ENCODING);
#endif
    ConfigDB configdb;
    configdb.setFirstDefault();
    MainFrame *mainframe = new MainFrame(configdb.getFramePos(wxT(APPNAME)),
                                         configdb.getFrameSize(wxT(APPNAME)));
    mainframe->SetIcon(wxICON(eliot));
    mainframe->Show(TRUE);
    SetTopWindow(mainframe);
    return TRUE;
}

int
EliotApp::OnExit()
{
    delete wxConfigBase::Set((wxConfigBase *) NULL);
    return 0;
}


