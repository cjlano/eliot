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

/**
 *  \file   main.cc
 *  \brief  Eliot main entry point
 *  \author Antoine Fraboulet
 *  \date   2005
 */

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
#include "game_factory.h"

#include "eliot.xpm"

class EliotApp : public wxApp
{
private:
    MainFrame *m_mainFrame;
protected:
    wxLocale locale;
public:
    virtual bool OnInit();
    virtual int  OnExit();
};

IMPLEMENT_APP(EliotApp)

bool
EliotApp::OnInit()
{
    wxApp::OnInit();
    srand(time(NULL));
    SetVendorName(wxT("Afrab"));
    SetAppName(wxString(wxT("eliot")) + wxT("-") + wxT(VERSION));
    SetClassName(wxT("eliot"));

    wxConfigBase::Get();

    locale.Init();

    // Search for translations in the installation directory
    wxString catalogPath;
#ifdef WIN32
    // Get the absolute path, as returned by GetFullPathName()
    wchar_t path[MAX_PATH];
    GetFullPathName(wstring(argv[0]).c_str(), MAX_PATH, path, NULL);
    wchar_t *pos = wcsrchr(path, L'\\');
    if (pos)
        *pos = '\0';
    catalogPath = wxU(path) + wxT("/locale");
#else
    catalogPath = wxT(LOCALEDIR);
#endif

    wxLocale::AddCatalogLookupPathPrefix(catalogPath);
    // No need to search in the current directory, it is already done by default
    // wxLocale::AddCatalogLookupPathPrefix(wxT("."));
    locale.AddCatalog(wxT("eliot"));
#ifdef __LINUX__
    {
        wxLogNull noLog;
        locale.AddCatalog(wxT("fileutils"));
    }
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
    GameFactory::Destroy();
    delete wxConfigBase::Set(NULL);
    return wxApp::OnExit();
}


/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
