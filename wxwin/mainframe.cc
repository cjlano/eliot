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
 *  \file   mainframe.cc
 *  \brief  Main frame for the Eliot GUI
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <exception>

using namespace std;

#include "wx/intl.h"
#include "wx/menu.h"
#include "wx/statusbr.h"
#include "wx/sizer.h"
#include "wx/filedlg.h"
#include "wx/msgdlg.h"

#include "ewx.h"

#include "dic.h"
#include "game.h"
#include "training.h"
#include "game_factory.h"
#include "player.h"

#include "configdb.h"
#include "confdimdlg.h"
#include "confsearch.h"
#include "printout.h"
#include "mainframe.h"

#ifdef ENABLE_SAVE_POSTSCRIPT
#include "wx/dcps.h"
#endif

enum
{
    Menu_Game_New                             = 1000,
    Menu_Game_NewJoker,
    Menu_Game_Open,
    Menu_Game_Save,
    Menu_Game_Print,
    Menu_Game_PrintPreview,
    Menu_Game_PrintPS,

    Menu_Conf_Game                            = 2000,
    Menu_Conf_Game_Dic,
    Menu_Conf_Game_Tiles,
    Menu_Conf_Game_Search,
    Menu_Conf_Print,
    Menu_Conf_Aspect                          = 2100,
    Menu_Conf_Tile,
    Menu_Conf_Aspect_BoardColour_DrawTiles,
    Menu_Conf_Aspect_Font,
    Menu_Conf_Aspect_Font_Search              = 2110,
    Menu_Conf_Aspect_Font_Board               = 2111,
    Menu_Conf_Aspect_Font_Default             = 2112,
    Menu_Conf_Aspect_BoardColour              = 2200,
    Menu_Conf_Aspect_BoardColour_Lines        = 2201,
    Menu_Conf_Aspect_BoardColour_Wx2          = 2202,
    Menu_Conf_Aspect_BoardColour_Wx3          = 2203,
    Menu_Conf_Aspect_BoardColour_Lx2          = 2204,
    Menu_Conf_Aspect_BoardColour_Lx3          = 2205,
    Menu_Conf_Aspect_BoardColour_Background   = 2206,
    Menu_Conf_Aspect_BoardColour_Letters      = 2207,
    Menu_Conf_Aspect_BoardColour_TestLetters  = 2208,
    Menu_Conf_Aspect_BoardColour_TileBack     = 2209,
    Menu_Conf_Aspect_BoardColour_TestTileBack = 2210,
    Menu_Conf_Aspect_BoardColour_Default      = 2211,

#define IDBASE 3300
    Menu_ShowVerif                            = (IDBASE + ID_Frame_Verif),
    Menu_ShowSearch                           = (IDBASE + ID_Frame_Search),
    Menu_ShowPlus1                            = (IDBASE + ID_Frame_Plus1),
    Menu_ShowRacc                             = (IDBASE + ID_Frame_Racc),
    Menu_ShowBenj                             = (IDBASE + ID_Frame_Benj),
    Menu_ShowBag                              = (IDBASE + ID_Frame_Bag),
    Menu_ShowBoard                            = (IDBASE + ID_Frame_Board),
    Menu_ShowGame                             = (IDBASE + ID_Frame_Game),
    Menu_ShowResult                           = (IDBASE + ID_Frame_Result),

    Button_SetRack                            = 10000,
    Button_SetNew,
    Button_SetManual,
    Button_Search,
    Button_Play,
    Button_PlayBack,

    ListCtrl_ID                               = 11000,
    Rack_ID,
    Status_ID
};

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
    //
    EVT_MENU(Menu_Game_New,          MainFrame::OnMenuGameNew)
    EVT_MENU(Menu_Game_NewJoker,     MainFrame::OnMenuGameNew)
    EVT_MENU(Menu_Game_Open,         MainFrame::OnMenuGameOpen)
    EVT_MENU(Menu_Game_Save,         MainFrame::OnMenuGameSave)
    EVT_MENU(Menu_Game_Print,        MainFrame::OnMenuGamePrint)
    EVT_MENU(Menu_Game_PrintPreview, MainFrame::OnMenuGamePrintPreview)
    EVT_MENU(Menu_Game_PrintPS,      MainFrame::OnMenuGamePrintPS)
    EVT_MENU(wxID_EXIT,              MainFrame::OnMenuGameQuit)
    //
    EVT_MENU(Menu_Conf_Game_Dic,     MainFrame::OnMenuConfGameDic)
    EVT_MENU(Menu_Conf_Game_Search,  MainFrame::OnMenuConfGameSearch)
    //
    EVT_MENU(Menu_Conf_Print,        MainFrame::OnMenuConfPrint)
    //
    EVT_MENU_RANGE(Menu_Conf_Aspect_Font_Search, Menu_Conf_Aspect_Font_Default, MainFrame::OnMenuConfAspectFont)
    EVT_MENU_RANGE(Menu_Conf_Aspect_BoardColour_Lines, Menu_Conf_Aspect_BoardColour_Default, MainFrame::OnMenuConfAspectBoardColour)
    EVT_MENU_RANGE(Menu_ShowVerif, Menu_ShowResult, MainFrame::OnMenuShowFrame)
    //
    EVT_MENU(wxID_ABOUT, MainFrame::OnMenuHelpAbout)
    //
    EVT_BUTTON(Button_Play,     MainFrame::OnPlay)
    EVT_BUTTON(Button_SetRack,  MainFrame::OnSetRack)
    EVT_BUTTON(Button_SetNew,   MainFrame::OnSetRack)
    EVT_BUTTON(Button_Search,   MainFrame::OnSearch)
    EVT_BUTTON(Button_PlayBack, MainFrame::OnPlay)
    //
    EVT_TEXT_ENTER(Rack_ID,    MainFrame::OnTextEnter)
    //
    EVT_CLOSE(MainFrame::OnCloseWindow)
    //
    //EVT_MENU(Menu_Help,              MainFrame::OnMenuHelp)
END_EVENT_TABLE()

// ******************************
//
// ******************************

MainFrame::MainFrame(wxPoint __UNUSED__ pos_, wxSize size_)
    : wxFrame((wxFrame *) NULL, -1, wxT(APPNAME), wxPoint(-1, -1),
              size_, wxDEFAULT_FRAME_STYLE, wxT(APPNAME)),
    m_dic(NULL), m_game(NULL)
{
#if defined(ENABLE_RESLIST_IN_MAIN)
    reslist = NULL;
#endif
    statusbar = NULL;
    for (int i = 0 ; i < MAX_FRAME_ID; i++)
        auxframes_ptr[i] = NULL;

    wxBoxSizer *listsizer = new wxBoxSizer(wxVERTICAL);
    rack = new wxTextCtrl(this, Rack_ID, wxU(""), wxPoint(-1, -1), wxSize(-1, -1), wxTE_PROCESS_ENTER);
    listsizer->Add(rack, 0, wxEXPAND | wxALL, 1);
    rack->SetToolTip(_("Rack"));
    rack->Enable(false);
#ifdef ENABLE_RESLIST_IN_MAIN
    reslist = new GfxResult(this,(MainFrame*)this,m_game);
    listsizer->Add(reslist, 1, wxEXPAND | wxLEFT | wxRIGHT, 1);
#endif

    InitMenu();

    statusbar = CreateStatusBar(2, 0, Status_ID);
    int ww[2] = {-1, 160};
    statusbar->SetStatusWidths(2, ww);
    UpdateStatusBar();

    b_rackrandomset = new wxButton(this, Button_SetRack,  _(" Rack "));
    b_rackrandomnew = new wxButton(this, Button_SetNew,   _(" Complement "));
    b_search        = new wxButton(this, Button_Search,   _(" Search "));
    b_back          = new wxButton(this, Button_PlayBack, _(" Back "));
    b_play          = new wxButton(this, Button_Play,     _(" Play "));

    b_rackrandomset->SetToolTip(_("Random rack"));
    b_rackrandomset->Enable(false);
    b_rackrandomnew->SetToolTip(_("Random complement of the rack"));
    b_rackrandomnew->Enable(false);
    b_search->SetToolTip(       _("Search with the current rack"));
    b_search->Enable(false);
    b_back->SetToolTip(         _("Go back one turn"));
    b_back->Enable(false);
    b_play->SetToolTip(         _("Play the selected word"));
    b_play->Enable(false);

    wxBoxSizer *buttonsizer = new wxBoxSizer(wxHORIZONTAL);
    buttonsizer->Add(b_rackrandomset, 1, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT , 1);
    buttonsizer->Add(b_rackrandomnew, 1, wxEXPAND | wxTOP | wxBOTTOM          , 1);
    buttonsizer->Add(b_search,        1, wxEXPAND | wxTOP | wxBOTTOM          , 1);
    buttonsizer->Add(b_back,          1, wxEXPAND | wxTOP | wxBOTTOM          , 1);
    buttonsizer->Add(b_play,          1, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 1);

    wxBoxSizer *mainsizer = new wxBoxSizer(wxVERTICAL);
    mainsizer->Add(listsizer  , 1, wxEXPAND | wxVERTICAL, 0);
    mainsizer->Add(buttonsizer, 0, wxEXPAND             , 0);

    SetAutoLayout(TRUE);
    SetSizer(mainsizer);
    mainsizer->Fit(this);
    mainsizer->SetSizeHints(this);

    SetClientSize(size_);
    Move(config.getFramePos(wxT(APPNAME)));

    wxString dicpath = config.getDicPath();
    try
    {
        Dictionary *dic = new Dictionary(dicpath.mb_str().data());
        m_dic = dic;
        m_game = GameFactory::Instance()->createTraining(*m_dic);
        if (m_game)
        {
            m_game->start();
        }
    }
    catch (std::exception &e)
    {
        wxCommandEvent event;
        // This will also start a new training game indirectly
        OnMenuConfGameDic(event);
    }

    InitFrames();
}

// ******************************
//
// ******************************

MainFrame::~MainFrame()
{
    config.setFramePos(wxT(APPNAME), GetPosition());
    config.setFrameSize(wxT(APPNAME), GetClientSize());

    if (m_game != NULL)
    {
        GameFactory::Instance()->releaseGame(*m_game);
        m_game = NULL;
    }

    delete m_dic;
}

// ******************************
//
// ******************************

void
MainFrame::InitMenu()
{
    // menus
    wxMenu *menu_game = new wxMenu;
    menu_game->Append(Menu_Game_New, _("&New game\tctrl+n"), _("Start a new game"));
    menu_game->Append(Menu_Game_NewJoker, _("New &joker game\tctrl+j"), _("Start a new joker game"));
    menu_game->AppendSeparator();
    menu_game->Append(Menu_Game_Open, _("&Load...\tctrl+l"), _("Load a game"));
    menu_game->Append(Menu_Game_Save, _("&Save as...\tctrl+s"), _("Save the current game"));
    menu_game->AppendSeparator();
    menu_game->Append(Menu_Game_Print, _("&Print...\tctrl+p"), _("Print this game"));
    menu_game->Append(Menu_Game_PrintPreview, _("Print pre&view..."), _("Print preview of the game"));
#ifdef ENABLE_SAVE_POSTSCRIPT
    menu_game->Append(Menu_Game_PrintPS, _("Print in PostS&cript..."), _("Print in a PostScript file"));
#endif
    menu_game->AppendSeparator();
    menu_game->Append(wxID_EXIT, _("&Quit"), _("Quit Eliot"));
    //
    wxMenu *menu_conf_game = new wxMenu;
    menu_conf_game->Append(Menu_Conf_Game_Dic, _("&Dictionary..."), _("Choose a dictionary"));
    menu_conf_game->Append(Menu_Conf_Game_Search, _("&Search..."), _("Search options"));
    //
    wxMenu *menu_conf_board_colour = new wxMenu;
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Background, _("&Background..."), _("Background color"));
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Lines, _("L&ines..."), _("Color of the lines"));
    menu_conf_board_colour->AppendSeparator();
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Letters, _("&Played letters..."), _("Color of the letters played on the board"));
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_TestLetters, _("&Temporary letters..."), _("Color of the letters of the temporary word"));
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_TileBack    , _("B&ackground of played letters..."), _("Background color of the letters played on the board"));
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_TestTileBack, _("Ba&ckground of temporary letters..."), _("Background color of the temporary letters on the board"));
    menu_conf_board_colour->AppendSeparator();
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Wx2, _("Double &letter..."), _("Color of the \"double letter\" squares"));
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Wx3, _("Triple l&etter..."), _("Color of the \"triple letter\" squares"));
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Lx2, _("Double &word..."), _("Color of the \"double word\" squares"));
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Lx3, _("Triple w&ord..."), _("Color of the \"triple word\" squares"));
    menu_conf_board_colour->AppendSeparator();
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Default, _("&Default colors"), _("Restore the default colors"));
    //
    wxMenu *menu_conf_board_font = new wxMenu;
    menu_conf_board_font->Append(Menu_Conf_Aspect_Font_Search, _("&Search letters..."), _("Font for the search"));
    //
    wxMenu *menu_conf = new wxMenu;
    menu_conf->Append(Menu_Conf_Game, _("&Game"), menu_conf_game, _("Configuration of the game"));
    menu_conf->Append(Menu_Conf_Aspect_Font, _("&Fonts"), menu_conf_board_font, _("Configuration of the fonts"));
    menu_conf->Append(Menu_Conf_Aspect_BoardColour, _("&Colors"), menu_conf_board_colour, _("Configuration of the colors"));
    menu_conf->Append(Menu_Conf_Print, _("&Printing..."), _("Configuration of the printing parameters"));
    //
    wxMenu *menu_frame = new wxMenu;
    menu_frame->AppendCheckItem(Menu_ShowBoard, _("&Board"), _("Game board"));
    menu_frame->AppendCheckItem(Menu_ShowBag, _("Ba&g"), _("Remaining letters in the bag"));
    menu_frame->AppendCheckItem(Menu_ShowVerif, _("&Check"), _("Check a word in the dictionary"));
    menu_frame->AppendCheckItem(Menu_ShowSearch, _("&Search"), _("Search in the dictionary"));
    menu_frame->AppendSeparator();
    menu_frame->AppendCheckItem(Menu_ShowPlus1, _("&Rack + 1"), _("Letters of the rack plus one"));
    menu_frame->AppendCheckItem(Menu_ShowRacc, _("R&accords"), _("Raccords on a word of the search"));
    menu_frame->AppendCheckItem(Menu_ShowBenj, _("&Benjamins"), _("Benjamins on a word of the search"));
    menu_frame->AppendSeparator();
    menu_frame->AppendCheckItem(Menu_ShowGame, _("Game &history"), _("Game history"));
#ifndef ENABLE_RESLIST_IN_MAIN
    menu_frame->Append(Menu_ShowResult, _("R&esults"), _("Results"));
#endif
    //
    wxMenu *menu_help = new wxMenu;
    menu_help->Append(wxID_ABOUT, _("&About..."), _("About Eliot"));
    //
    wxMenuBar *menu_bar = new wxMenuBar;
    menu_bar->Append(menu_game, _("&Game"));
    menu_bar->Append(menu_conf, _("&Settings"));
    menu_bar->Append(menu_frame, _("&Windows"));
    menu_bar->Append(menu_help, _("&Help"));

    SetMenuBar(menu_bar);
}

// *******************
//
// *******************

void
MainFrame::OnCloseWindow(wxCloseEvent&)
{
    this->Destroy();
}

// *******************
// NEW
// *******************

void
MainFrame::OnMenuGameNew(wxCommandEvent& event)
{
    if (m_dic == NULL)
    {
        wxMessageBox(_("No dictionary selected"), _("Eliot: error"),
                     wxICON_INFORMATION | wxOK);
        return;
    }

    //    TODO("selection du type de partie dans OnMenuGameNew\n");

    if (m_game != NULL)
    {
        GameFactory::Instance()->releaseGame(*m_game);
        m_game = NULL;
    }

    m_game = GameFactory::Instance()->createTraining(*m_dic);
    // Joker game?
    if (event.GetId() == Menu_Game_NewJoker)
        m_game->setVariant(Game::kJOKER);

    m_game->start();
    rack->SetValue(wxT(""));
    InitFrames();
#ifdef ENABLE_RESLIST_IN_MAIN
    reslist->SetGame(m_game);
#endif
    // Re-enable the main buttons
    b_rackrandomset->Enable(true);
    b_rackrandomnew->Enable(true);
    b_search->Enable(true);
    b_back->Enable(true);
    b_play->Enable(true);
    rack->Enable(true);


    UpdateStatusBar();
    UpdateFrames(AuxFrame::FORCE_REFRESH);
}

// *******************
// OPEN
// *******************

void
MainFrame::OnMenuGameOpen(wxCommandEvent&)
{
    wxFileDialog dialog(this, _("Load a game"), wxT(""), wxT(""), wxT("*"), wxOPEN);
    if (m_dic == NULL)
    {
        wxMessageBox(_("No dictionary selected"), _("Eliot: error"),
                     wxICON_INFORMATION | wxOK);
        return;
    }
    if (dialog.ShowModal() != wxID_OK)
    {
        return;
    }

    if (m_game != NULL)
    {
        GameFactory::Instance()->releaseGame(*m_game);
        m_game = NULL;
    }

    FILE* fin;

    if ((fin = fopen(dialog.GetPath().mb_str(), "rb")) == NULL)
    {
        wxString txt;
        txt << _("Cannot open ") << dialog.GetPath();
        wxMessageDialog msg(this, txt, _("Load a game"));
        msg.ShowModal();
        return ;
    }

    m_game = Game::load(fin, *m_dic);
    fclose(fin);

    if (m_game == NULL)
    {
        wxMessageDialog msg(this,
                            _("Error while loading the game"),
                            _("Invalid game"));
        msg.ShowModal();
        return;
    }

    if (m_game->getHistory().getSize() == 0)
    {
        wxMessageDialog msg(this,
                            _("Error while loading the game"),
                            _("The game is empty"));
        msg.ShowModal();
        return;
    }

    std::wstring r = m_game->getCurrentPlayer().getCurrentRack().toString();

    rack->SetValue(wxU(r.c_str()));
    // update gfxboard and all frames
    InitFrames();
    // update status bar
#ifdef ENABLE_RESLIST_IN_MAIN
    reslist->SetGame(m_game);
#endif
    UpdateStatusBar();
    UpdateFrames(AuxFrame::FORCE_REFRESH);
}

// *******************
// SAVE
// *******************

void
MainFrame::OnMenuGameSave(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog dialog(this, _("Save the game"), wxT(""), wxT(""), wxT("*"), wxSAVE|wxOVERWRITE_PROMPT);
    if (dialog.ShowModal() == wxID_OK)
    {
        ofstream fout(dialog.GetPath().mb_str());
        if (fout.rdstate() == ios::failbit)
        {
            wxString txt;
            txt << _("Cannot create ") << dialog.GetPath();
            wxMessageDialog msg(this, txt, _("Save the game"));
            msg.ShowModal();
            return ;
        }
        m_game->save(fout);
        fout.close();
    }
}

// *******************
// PRINT
// *******************

void
MainFrame::OnMenuGamePrint(wxCommandEvent& WXUNUSED(event))
{
    // TODO: gray out the menu instead...
    if (m_game == NULL)
    {
        wxMessageBox(_("No on going game"), _("Eliot: error"),
                     wxICON_INFORMATION | wxOK);
        return;
    }
    wxPrintDialogData printDialogData(config.getPrintData());
    wxPrinter printer(&printDialogData);
    GamePrintout printout(*m_game);
    if (!printer.Print(this, &printout, TRUE))
    {
        wxMessageBox(_("Printing not done"), _("Printing"),
                     wxOK | wxICON_ERROR);
    }
}

void
MainFrame::OnMenuGamePrintPreview(wxCommandEvent& WXUNUSED(event))
{
    // TODO: gray out the menu instead...
    if (m_game == NULL)
    {
        wxMessageBox(_("No on going game"), _("Eliot: error"),
                     wxICON_INFORMATION | wxOK);
        return;
    }
    wxPrintData printdata = config.getPrintData();

    wxString msg;
    wxPrintPreview *preview = new wxPrintPreview(new GamePrintout(*m_game),
                                                 new GamePrintout(*m_game), & printdata);
    if (!preview->Ok())
    {
        delete preview;
        msg << _("Print preview problem.\n")
            << _("The printer may not be correctly initialized");
        wxMessageBox(msg, _("Print preview"), wxOK);
        return;
    }
    wxPreviewFrame *frame = new wxPreviewFrame(preview, this, _("Printing"),
                                               wxPoint(-1, -1), wxSize(600, 550));
    frame->Centre(wxBOTH);
    frame->Initialize();
    frame->Show(TRUE);
}

void
MainFrame::OnMenuGamePrintPS(wxCommandEvent& WXUNUSED(event))
{
#ifdef ENABLE_SAVE_POSTSCRIPT
    // TODO: gray out the menu instead...
    if (m_game == NULL)
    {
        wxMessageBox(_("No on going game"), _("Eliot: error"),
                     wxICON_INFORMATION | wxOK);
        return;
    }
    wxFileDialog dialog(this, _("Print to a PostScript file"), wxT(""), wxT(""), wxT("*.ps"), wxSAVE|wxOVERWRITE_PROMPT);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxPrintData printdataPS;
        printdataPS.SetPrintMode(wxPRINT_MODE_FILE);
        printdataPS.SetFilename(dialog.GetPath());
        printdataPS.SetPaperId(wxPAPER_A4);
        printdataPS.SetQuality(wxPRINT_QUALITY_HIGH);
        printdataPS.SetOrientation(wxPORTRAIT);

        wxPostScriptDC printps(printdataPS);
        if (printps.Ok())
        {
            wxPrintDialogData printDialogData(printdataPS);
            wxPostScriptPrinter printer(&printDialogData);
            GamePrintout printout(*m_game);
            if (!printer.Print(this, &printout, FALSE))
            {
                wxMessageBox(_("Printing not done"),
                             _("PostScript printing"), wxOK | wxICON_ERROR);
            }
        }
        else
        {
            wxMessageBox(_("Cannot initialize PostScript printer"),
                         _("PostScript printing"), wxOK | wxICON_ERROR);
        }
    }
#endif
}


void
MainFrame::OnMenuGameQuit(wxCommandEvent& WXUNUSED(event))
{
    Close(TRUE);
}



// *******************
// Dictionary Loading
// *******************

void
MainFrame::OnMenuConfGameDic(wxCommandEvent& WXUNUSED(event))
{
    wxFileDialog dialog(this, _("Choose a dictionary"), wxT(""), wxT("*.dawg"), wxT("*.dawg"), wxOPEN);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxString dicpath = dialog.GetPath();
        try
        {
            Dictionary *dic = new Dictionary(dicpath.mb_str().data());
            delete m_dic;
            m_dic = dic;
            config.setDicPath(dialog.GetPath(), ::wxFileNameFromPath(dialog.GetPath()));
            wxCommandEvent event;
            OnMenuGameNew(event);
        }
        catch (std::exception &e)
        {
            wxMessageDialog dlg(NULL, wxU(e.what()), wxT(APPNAME));
            dlg.ShowModal();
        }
    }
}

// ****************
// MENU CONF SEARCH
// ****************

void
MainFrame::OnMenuConfGameSearch(wxCommandEvent& WXUNUSED(event))
{
    ConfSearchDlg dlg(this);
    dlg.ShowModal();
}

// ****************
// MENU CONF PRINT
// ****************

void
MainFrame::OnMenuConfPrint(wxCommandEvent& WXUNUSED(event))
{
    ConfDimDlg dlg(this, config.getPrintData(), config.getPageSetupData());
    if (dlg.ShowModal() == wxID_OK)
    {
        config.setPrintData(dlg.getPrintData());
        config.setPageSetupData(dlg.getPageSetupData());
    }
}

// ****************
// MENU CONF FONTS
// ****************

void
MainFrame::OnMenuConfAspectFont(wxCommandEvent& event)
{
    int id;
    wxString attr;

    id = event.GetId();
    if (! GetMenuBar()->FindItem(id))
        return;

    switch (id)
    {
        case Menu_Conf_Aspect_Font_Search:  attr = wxString(LISTFONT); break;
        case Menu_Conf_Aspect_Font_Board:   attr = wxString(BOARDFONT); break;
        case Menu_Conf_Aspect_Font_Default: attr = wxU("Default"); break;
        default: INCOMPLETE; break;
    }

    if (attr == wxU("Default"))
        config.setFontDefault();
    else
        config.setFont(attr, config.ChooseFont(this, config.getFont(attr)));

    UpdateFrames(AuxFrame::FORCE_REFRESH);
}

// ************************
// MENU CONF BOARD COLOURS
// ************************

void
MainFrame::OnMenuConfAspectBoardColour(wxCommandEvent& event)
{
    int id;
    wxString attr;

    id = event.GetId();
    if (! GetMenuBar()->FindItem(id))
        return;

    switch (id)
    {
        case Menu_Conf_Aspect_BoardColour_Lines:        attr = wxString(BCOLOURLINES); break;
        case Menu_Conf_Aspect_BoardColour_Wx2:          attr = wxString(BCOLOURWX2); break;
        case Menu_Conf_Aspect_BoardColour_Wx3:          attr = wxString(BCOLOURWX3); break;
        case Menu_Conf_Aspect_BoardColour_Lx2:          attr = wxString(BCOLOURLX2); break;
        case Menu_Conf_Aspect_BoardColour_Lx3:          attr = wxString(BCOLOURLX3); break;
        case Menu_Conf_Aspect_BoardColour_Background:   attr = wxString(BCOLOURBACKGROUND); break;
        case Menu_Conf_Aspect_BoardColour_Letters:      attr = wxString(BCOLOURLETTERS); break;
        case Menu_Conf_Aspect_BoardColour_TestLetters:  attr = wxString(BCOLOURTSTLETTERS); break;
        case Menu_Conf_Aspect_BoardColour_TileBack:     attr = wxString(BTILEBACKGROUND); break;
        case Menu_Conf_Aspect_BoardColour_TestTileBack: attr = wxString(BTSTTILEBACKGROUND); break;
        case Menu_Conf_Aspect_BoardColour_Default:      attr = wxU("Default"); break;
        default: INCOMPLETE; break;
    }

    if (attr == wxU("Default"))
        config.setColourDefault();
    else
        config.setColour(attr, config.ChooseColour(this, config.getColour(attr)));

    UpdateFrames(AuxFrame::FORCE_REFRESH);
}

//**************************************************************************************
//   MENU HELP
//**************************************************************************************

void
MainFrame::OnMenuHelpAbout(wxCommandEvent& WXUNUSED(event))
{
    wxString msg;
    msg.Printf(wxT("Eliot %s\n\n"), wxT(VERSION));
    msg << wxT("Copyright (C) 1999-2007 - Antoine Fraboulet & Olivier Teuliere\n\n");
    msg << _("This program is free software; you can redistribute it and/or modify " \
             "it under the terms of the GNU General Public License as published by " \
             "the Free Software Foundation; either version 2 of the License, or " \
             "(at your option) any later version.");
    wxMessageBox(msg, _("About Eliot"), wxICON_INFORMATION | wxOK);
}

//**************************************************************************************
// BUTTONS
//**************************************************************************************

void
MainFrame::OnSetRack(wxCommandEvent& event)
{
    int id;

    Game::set_rack_mode mode = Game::RACK_NEW;
    debug("OnSetRack ");
    switch ((id = event.GetId()))
    {
        case Button_SetRack:
            mode = Game::RACK_ALL;
            debug("PlayedRack::RACK_ALL\n");
            break;
        case Button_SetNew:
            mode = Game::RACK_NEW;
            debug("PlayedRack::RACK_NEW\n");
            break;
        case Button_SetManual:
            mode = Game::RACK_MANUAL;
            debug("PlayedRack::RACK_MANUAL\n");
            break;
        default:
            return;
    }
    SetRack(mode);
}

void
MainFrame::OnSearch(wxCommandEvent& WXUNUSED(event))
{
    debug("MainFrame::OnSearch\n");
    // check if rack has been set manually
    SetRack(Game::RACK_MANUAL,rack->GetValue());
    Search();
}

void
MainFrame::OnTextEnter(wxCommandEvent& WXUNUSED(event))
{
    debug("MainFrame::OnTextEnter -> %s\n",(const char*)rack->GetValue().mb_str());
    SetRack(Game::RACK_MANUAL,rack->GetValue());
    Search();
}

void
MainFrame::OnPlay(wxCommandEvent& event)
{
    int dir = 1;
    int id = event.GetId();
    switch (id)
    {
        case Button_Play:
            dir = 1;
            break;
        case Button_PlayBack:
            dir = -1;
            break;
        default:
            break;
    }
    Play(dir);
}

//*********************************
// SPECIAL FRAMES
//*********************************

void
MainFrame::InitFrames()
{
    debug("InitFrames start : \n");
    if (m_game == NULL)
    {
        debug("m_game == NULL\n");
        return;
    }

    for (int i = 0 ; i < MAX_FRAME_ID; i++)
    {
        debug("   delete frame %d\n",i);
        delete auxframes_ptr[i];
    }

    auxframes_ptr[ID_Frame_Verif]  = new VerifFrame (this, m_game->getDic());
    debug("0 : Verif\n");
    auxframes_ptr[ID_Frame_Search] = new SearchFrame(this, m_game->getDic());
    debug("1 : Search\n");
    auxframes_ptr[ID_Frame_Plus1]  = new Plus1Frame (this, m_game);
    debug("2 : Plus1\n");
    auxframes_ptr[ID_Frame_Racc]   = new RaccFrame  (this, m_game);
    debug("3 : Racc\n");
    auxframes_ptr[ID_Frame_Benj]   = new BenjFrame  (this, m_game);
    debug("4 : Benj\n");
    auxframes_ptr[ID_Frame_Bag]    = new BagFrame   (this, *m_game);
    debug("5 : Bag\n");
    auxframes_ptr[ID_Frame_Board]  = new BoardFrame (this, *m_game);
    debug("6 : Board\n");
    auxframes_ptr[ID_Frame_Game]   = new GameFrame  (this, *m_game);
    debug("7 : Game\n");
#ifndef ENABLE_RESLIST_IN_MAIN
    auxframes_ptr[ID_Frame_Result] = new ResultFrame(this, m_game);
    debug("8 : Result\n");
#endif

    for (int i = MIN_FRAME_ID; i < MAX_FRAME_ID; i++)
    {
        if (auxframes_ptr[i] != NULL)
        {
            auxframes_ptr[i]->Reload();
            debug("reload %d\n",i);
        }
    }

    // Check the corresponding menu item if the window is visible
    GetMenuBar()->Check(Menu_ShowVerif, auxframes_ptr[ID_Frame_Verif]->IsShown());
    GetMenuBar()->Check(Menu_ShowSearch, auxframes_ptr[ID_Frame_Search]->IsShown());
    GetMenuBar()->Check(Menu_ShowPlus1, auxframes_ptr[ID_Frame_Plus1]->IsShown());
    GetMenuBar()->Check(Menu_ShowRacc, auxframes_ptr[ID_Frame_Racc]->IsShown());
    GetMenuBar()->Check(Menu_ShowBenj, auxframes_ptr[ID_Frame_Benj]->IsShown());
    GetMenuBar()->Check(Menu_ShowBag, auxframes_ptr[ID_Frame_Bag]->IsShown());
    GetMenuBar()->Check(Menu_ShowBoard, auxframes_ptr[ID_Frame_Board]->IsShown());
    GetMenuBar()->Check(Menu_ShowGame, auxframes_ptr[ID_Frame_Game]->IsShown());
#ifndef ENABLE_RESLIST_IN_MAIN
    GetMenuBar()->Check(Menu_ShowResult, auxframes_ptr[ID_Frame_Result]->IsShown());
#endif
    debug("InitFrames end ok.\n");
}

void
MainFrame::OnMenuShowFrame(wxCommandEvent& event)
{
    int id;
    id = event.GetId();

    if (!GetMenuBar()->FindItem(id))
        return;
    id -= IDBASE;

    if ((id < 0) || (id >= MAX_FRAME_ID))
    {
        INCOMPLETE;
        return;
    }

    if (auxframes_ptr[id] == NULL)
    {
        debug("ShowFrame: auxframes_ptr[%d] == NULL\n", id);
        return;
    }
    auxframes_ptr[id]->SwitchDisplay();
    debug("ShowFrame: SwitchDisplay frame %d\n",id);
}

// *********************************
// UPDATES
// *********************************

void
MainFrame::UpdateFrames(AuxFrame::refresh_t force)
{
    for (int id = 0; id < MAX_FRAME_ID; id++)
    {
        if (auxframes_ptr[id])
        {
            auxframes_ptr[id]->Refresh(force);
        }
    }
#ifdef ENABLE_RESLIST_IN_MAIN
    if (reslist)
    {
        reslist->Refresh();
    }
#endif
}

void
MainFrame::UpdateStatusBar()
{
    wxString text;
    if (statusbar)
    {
        text = config.getDicName() + wxT(" ") + config.getTileName();
        statusbar->SetStatusText(text, 0);

        if (m_game)
        {
            text = wxT("");
            text << _("turn:") << wxT(" ") << (m_game->getHistory().getSize() + 1) << wxT(" ");
            text << _("points:") << wxT(" ") << (m_game->getCurrentPlayer().getPoints());
            statusbar->SetStatusText(text, 1);
        }
    }
}

// *********************************
// ACTIONS
// *********************************

// Can come from a
//    BUTTON ALL -> mode = Game::RACK_ALL, srack = empty
//    BUTTON NEW -> mode = Game::RACK_NEW, srack = empty
//    TEXT_ENTER -> mode = Game::RACK_MANUAL, srack = letters
//    Play       -> mode = Game::RACK_MANUAL, srack = letters

void
MainFrame::SetRack(Game::set_rack_mode mode, wxString srack)
{
    wxString msg;
    bool check = config.getRackChecking();

    if (m_game == NULL)
    {
        return;
    }
    static_cast<Training*>(m_game)->removeTestPlay();
    int res = static_cast<Training*>(m_game)->setRack(mode, check, srack.c_str());

    switch (res)
    {
        case 0x00: /* ok */
            debug("SetRack Ok :: ");
            break;
        case 0x01:
            msg = _("The bag doesn't contain enough letters\nfor a new rack.");
            wxMessageBox(msg, _("Rack validation"), wxICON_ERROR | wxOK);
            return;
        case 0x02:
            msg = _("The rack must contain at least 2 consonants and 2 vowels.");
            wxMessageBox(msg, _("Rack validation"), wxICON_ERROR | wxOK);
            return;
        case 0x03:
            msg = _("The rack contains invalid letters for the current dictionary");
            wxMessageBox(msg, _("Rack validation"), wxICON_ERROR | wxOK);
            break;
        default:
            statusbar->SetStatusText(_("The rack has been modified manually"), 0);
            break;
    }

    std::wstring r = m_game->getCurrentPlayer().getCurrentRack().toString();
    debug("MainFrame::SetRack : setvalue %ls\n",r.c_str());
    rack->SetValue(wxU(r.c_str()));
    UpdateFrames();
    UpdateStatusBar();
}

void
MainFrame::Search()
{
    if (m_game == NULL)
    {
        return;
    }
    static_cast<Training*>(m_game)->removeTestPlay();
#ifdef ENABLE_RESLIST_IN_MAIN
    reslist->Search();
#else
    if (auxframes_ptr[ ID_Frame_Result ])
    {
        ((ResultFrame*)(auxframes_ptr[ ID_Frame_Result ]))->Search();
    }
#endif
    UpdateFrames();
    UpdateStatusBar();
}

void
MainFrame::Play(int n)
{
    if (m_game == NULL)
    {
        return;
    }
    static_cast<Training*>(m_game)->removeTestPlay();
    if (n < 0)
    {
        debug("MainFrame::Play back %d\n",n);
        m_game->back(- n);
    }
    else
    {
        int n = 0;
        debug("MainFrame::Play +%d\n",n);
#ifdef ENABLE_RESLIST_IN_MAIN
        n = reslist->GetSelected();
#else
        n = ((ResultFrame*)auxframes_ptr[ ID_Frame_Result ])->GetSelected();
#endif
        if (n > -1)
        {
            static_cast<Training*>(m_game)->playResult(n);
        }
    }
    wxString r = wxU(m_game->getCurrentPlayer().getCurrentRack().toString().c_str());
    rack->SetValue(r);
    UpdateFrames();
    UpdateStatusBar();
}

void
MainFrame::TestPlay(int n)
{
    if (m_game == NULL)
    {
        return;
    }
    static_cast<Training*>(m_game)->removeTestPlay();
    static_cast<Training*>(m_game)->testPlay(n);
    UpdateFrames();
    UpdateStatusBar();
}

/****************************************************************/
/****************************************************************/

/// Local Variables: / mode: c++ / mode: hs-minor / c-basic-offset: 4 /
//indent-tabs-mode: nil / End:
