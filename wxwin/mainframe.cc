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

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

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

    Menu_Quit_Apropos                         = 4000,
    Menu_Quit_Confirm,

    Button_SetRack                            = 10000,
    Button_SetNew,
    Button_SetManual,
    Button_Search,
    Button_Play,
    Button_PlayBack,

    ListCtrl_ID                               = 11000,
    Rack_ID,
    Status_ID,
};

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
    //
    EVT_MENU(Menu_Game_New,          MainFrame::OnMenuGameNew)
    EVT_MENU(Menu_Game_Open,         MainFrame::OnMenuGameOpen)
    EVT_MENU(Menu_Game_Save,         MainFrame::OnMenuGameSave)
    EVT_MENU(Menu_Game_Print,        MainFrame::OnMenuGamePrint)
    EVT_MENU(Menu_Game_PrintPreview, MainFrame::OnMenuGamePrintPreview)
    EVT_MENU(Menu_Game_PrintPS,      MainFrame::OnMenuGamePrintPS)
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
    EVT_MENU(Menu_Quit_Apropos, MainFrame::OnMenuQuitApropos)
    EVT_MENU(Menu_Quit_Confirm, MainFrame::OnMenuQuitConfirm)
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
    
MainFrame::MainFrame(wxPoint pos_, wxSize size_)
    : wxFrame((wxFrame *) NULL, -1, wxT(APPNAME), wxPoint(-1, -1),
              size_, wxDEFAULT_FRAME_STYLE, wxT(APPNAME)),
    m_dic(NULL), m_game(NULL)
{
#if defined(ENABLE_RESLIST_IN_MAIN)
    reslist = NULL;
#endif
    statusbar = NULL;
    for(int i=0 ; i < MAX_FRAME_ID; i++)
      auxframes_ptr[i] = NULL;

    wxString dicpath = config.getDicPath();
    Dic_load(&m_dic, dicpath.mb_str());
    if (m_dic == NULL)
	{
	    wxCommandEvent event;
	    OnMenuConfGameDic(event);
	}
    m_game = GameFactory::Instance()->createTraining(m_dic);
    if (m_game)
	{
	    m_game->start();
	}

    wxBoxSizer *listsizer = new wxBoxSizer(wxVERTICAL);
    rack = new wxTextCtrl(this, Rack_ID, wxU(""), wxPoint(-1, -1), wxSize(-1, -1), wxTE_PROCESS_ENTER);
    listsizer->Add(rack    , 0 , wxEXPAND | wxALL, 1);
    rack->SetToolTip(_("Tirage"));
#ifdef ENABLE_RESLIST_IN_MAIN
    reslist = new GfxResult(this,(MainFrame*)this,m_game);
    listsizer->Add(reslist, 1, wxEXPAND | wxLEFT | wxRIGHT, 1);
#endif

    InitMenu();

    statusbar = CreateStatusBar(2, 0, Status_ID);
    int ww[2] = {-1, 160};
    statusbar->SetStatusWidths(2, ww);
    UpdateStatusBar();

    b_rackrandomset = new wxButton(this, Button_SetRack,  _(" Tirage "));
    b_rackrandomnew = new wxButton(this, Button_SetNew,   _(" Complement "));
    b_search        = new wxButton(this, Button_Search,   _(" Rechercher "));
    b_back          = new wxButton(this, Button_PlayBack, _(" Arriere "));
    b_play          = new wxButton(this, Button_Play,     _(" Jouer "));

    b_rackrandomset->SetToolTip(_("Tirage aleatoire"));
    b_rackrandomnew->SetToolTip(_("Complement aleatoire du tirage"));
    b_search->SetToolTip(       _("Recherche sur le tirage courant"));
    b_back->SetToolTip(         _("Revenir un coup en arriere"));
    b_play->SetToolTip(         _("Jouer le mot selectionne"));

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

    if (m_dic)
      {
	Dic_destroy(m_dic);
      }
}

// ******************************
//
// ******************************

void
MainFrame::InitMenu()
{
    // menus
    wxMenu *menu_game = new wxMenu;
    menu_game->Append(Menu_Game_New, _("Nouvelle"), _("Demarrer une nouvelle partie"));
    menu_game->Append(Menu_Game_Open, _("Charger..."), _("Charger une partie"));
    menu_game->Append(Menu_Game_Save, _("Sauver..."), _("Sauver cette partie"));
    menu_game->AppendSeparator();
    menu_game->Append(Menu_Game_Print, _("Imprimer..."), _("Imprimer cette partie"));
    menu_game->Append(Menu_Game_PrintPreview, _("Preimpression"), _("Preimpression de la partie"));
#ifdef ENABLE_SAVE_POSTSCRIPT
    menu_game->AppendSeparator();
    menu_game->Append(Menu_Game_PrintPS, _("Imprimer du PostScript"), _("Imprimer dans un fichier PostScript"));
#endif
    //
    wxMenu *menu_conf_game = new wxMenu;
    menu_conf_game->Append(Menu_Conf_Game_Dic, _("Dictionnaire"), _("Choix du dictionnaire"));
    menu_conf_game->Append(Menu_Conf_Game_Search, _("Recherche"), _("Options de recherche"));
    //
    wxMenu *menu_tileback = new wxMenu;
    menu_tileback->Append(Menu_Conf_Aspect_BoardColour_Letters     , _("Lettres jouees"), _("Lettres jouees sur la grille"));
    menu_tileback->Append(Menu_Conf_Aspect_BoardColour_TestLetters , _("Lettres provisoires"), _("Lettres du mot a jouer"));
    menu_tileback->AppendSeparator();
    menu_tileback->Append(Menu_Conf_Aspect_BoardColour_TileBack    , _("Fonds lettres jouees"), _("Fonds des pions sur la grille"));
    menu_tileback->Append(Menu_Conf_Aspect_BoardColour_TestTileBack, _("Fonds lettres provisoires"), _("Fonds des pions sur la grille"));
    //
    wxMenu *menu_conf_board_colour = new wxMenu;
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Background, _("Fond"), _("Couleur du fond"));
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Lines, _("Lignes"), _("Couleur des lignes"));
    menu_conf_board_colour->Append(Menu_Conf_Tile, _("Pions et lettres"), menu_tileback, _("Pions et lettres"));
    menu_conf_board_colour->AppendSeparator();
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Letters, _("Lettres jouees"), _("Lettres jouees sur la grille"));
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_TestLetters, _("Lettres provisoires"), _("Lettres du mot a jouer"));
    menu_conf_board_colour->AppendSeparator();
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Wx2, _("Mot compte double"), _("Mot compte double"));
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Wx3, _("Mot compte triple"), _("Mot compte triple"));
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Lx2, _("Lettre compte double"), _("Lettre compte double"));
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Lx3, _("Lettre compte triple"), _("Lettre compte triple"));
    menu_conf_board_colour->AppendSeparator();
    menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Default, _("Couleurs d'origine"), _("Retrouver les couleurs d'origine"));
    //
    wxMenu *menu_conf_board_font = new wxMenu;
// XXX:    menu_conf_board_font->Append(Menu_Conf_Aspect_Font_Search, _("Lettres de recherche"), _("Police de caractères pour les recherches"));
    menu_conf_board_font->Append(Menu_Conf_Aspect_Font_Search, _("Lettres de recherche"), _("Police de caracteres pour les recherches"));
// XXX:    menu_conf_board_font->Append(Menu_Conf_Aspect_Font_Board, _("Lettres de la grille"), _("Police de caractères de la grille"));
    menu_conf_board_font->Append(Menu_Conf_Aspect_Font_Board, _("Lettres de la grille"), _("Police de caracteres de la grille"));
    //
    wxMenu *menu_conf = new wxMenu;
    menu_conf->Append(Menu_Conf_Game, _("Jeu"), menu_conf_game, _("Configuration du jeu"));
    menu_conf->Append(Menu_Conf_Aspect_Font, _("Fonte des lettres"), menu_conf_board_font, _("Modification des fontes"));
    menu_conf->Append(Menu_Conf_Aspect_BoardColour, _("Couleurs de la grille"), menu_conf_board_colour, _("Modification des couleurs"));
    menu_conf->Append(Menu_Conf_Print, _("Impression"), _("Dimensions de la partie"));
    //
    wxMenu *menu_frame = new wxMenu;
    menu_frame->Append(Menu_ShowBoard, _("Grille"), _("Grille de jeu"));
// XXX:    menu_frame->Append(Menu_ShowVerif, _("Vérification"), _("Vérification d'un mot dans le dictionnaire"));
    menu_frame->Append(Menu_ShowVerif, _("Verification"), _("Verification d'un mot dans le dictionnaire"));
    menu_frame->Append(Menu_ShowSearch, _("Recherche"), _("Recherche dans le dictionnaire"));
    menu_frame->AppendSeparator();
    menu_frame->Append(Menu_ShowPlus1, _("Tirage + 1"), _("Lettres du tirage plus une"));
    menu_frame->Append(Menu_ShowRacc, _("Raccords"), _("Raccords sur un mot de la recherche"));
    menu_frame->Append(Menu_ShowBenj, _("Benjamins"), _("Benjamins sur un mot de la recherche"));
    menu_frame->AppendSeparator();
    menu_frame->Append(Menu_ShowBag, _("Sac"), _("Lettres restantes dans le sac"));
    menu_frame->AppendSeparator();
    menu_frame->Append(Menu_ShowGame, _("Partie"), _("Partie"));
#ifndef ENABLE_RESLIST_IN_MAIN
    menu_frame->Append(Menu_ShowResult, _("Resultats"), _("Resultats"));
#endif
    //
    wxMenu *menu_quit = new wxMenu;
    menu_quit->Append(Menu_Quit_Apropos, _("A propos..."), _("A propos d'Eliot"));
    menu_quit->Append(Menu_Quit_Confirm, _("Quitter"), _("Quitter"));
    //
    wxMenuBar *menu_bar = new wxMenuBar;
    menu_bar->Append(menu_game, _("Partie"));
    menu_bar->Append(menu_conf, _("Configuration"));
// XXX:    menu_bar->Append(menu_frame, _("Fenêtres"));
    menu_bar->Append(menu_frame, _("Fenetres"));
    menu_bar->Append(menu_quit, _("Quitter"));

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
MainFrame::OnMenuGameNew(wxCommandEvent&)
{
    if (m_dic == NULL)
      {
        wxMessageBox(_("Il n'y a pas de dictionnaire selectionne"), _("Eliot: erreur"),
                     wxICON_INFORMATION | wxOK);
        return;
      }

    //    TODO("selection du type de partie dans OnMenuGameNew\n");

    if (m_game != NULL)
      {
	GameFactory::Instance()->releaseGame(*m_game);
	m_game = NULL;
      }

    m_game = GameFactory::Instance()->createTraining(m_dic);
    m_game->start();
    rack->SetValue(wxU(""));
    InitFrames();
#ifdef ENABLE_RESLIST_IN_MAIN
    reslist->SetGame(m_game);
#endif
    UpdateStatusBar();
    UpdateFrames(AuxFrame::FORCE_REFRESH);
}

// *******************
// OPEN
// *******************

void
MainFrame::OnMenuGameOpen(wxCommandEvent&)
{
    wxString txt;
    wxFileDialog dialog(this, _("Ouvrir une partie"), wxT(""), wxT(""), wxT("*"), wxOPEN);
    if (m_dic == NULL)
    {
        wxMessageBox(_("Il n'y a pas de dictionnaire selectionne"), _("Eliot: erreur"),
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
            txt << _("Impossible d'ouvrir") << dialog.GetPath();
            wxMessageDialog msg(this, txt, _("Ouverture d'une partie"));
            msg.ShowModal();
            return ;
        }

    m_game = Game::load(fin, m_dic);
    fclose(fin);

    if (m_game == NULL)
        {
            wxMessageDialog msg(this,
				_("Erreur pendant la lecture de la partie"),
				_("chargement de partie"));
            msg.ShowModal();
            return;
        }

    if (m_game->getHistory().getSize() == 0)
	{
            wxMessageDialog msg(this,
				_("Erreur pendant la lecture de la partie"),
				_("La partie est vide"));
            msg.ShowModal();
            return;
	}

    std::wstring r;

    if (m_game->getHistory().getSize() >= 0)
    {
        r = m_game->getCurrentPlayer().getCurrentRack().toString();
    }

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
    wxFileDialog dialog(this, _("Sauver une partie"), wxT(""), wxT(""), wxT("*"), wxSAVE|wxOVERWRITE_PROMPT);
    if (dialog.ShowModal() == wxID_OK)
    {
        ofstream fout(dialog.GetPath().mb_str());
        if (fout.rdstate() == ios::failbit)
        {
            wxString txt;
// XXX:            txt << _("Impossible de créer ") << dialog.GetPath();
            txt << _("Impossible de creer ") << dialog.GetPath();
            wxMessageDialog msg(this, txt, _("Sauvegarde de la partie"));
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
        wxMessageBox(_("Pas de partie en cours"), _("Eliot: erreur"),
                     wxICON_INFORMATION | wxOK);
        return;
    }
    wxPrintDialogData printDialogData(config.getPrintData());
    wxPrinter printer(&printDialogData);
    GamePrintout printout(*m_game);
    if (!printer.Print(this, &printout, TRUE))
// XXX:        wxMessageBox(wxT("Impression non effectuée."));
        wxMessageBox(_("Impression non effectuee."));
}

void
MainFrame::OnMenuGamePrintPreview(wxCommandEvent& WXUNUSED(event))
{
    // TODO: gray out the menu instead...
    if (m_game == NULL)
    {
        wxMessageBox(_("Pas de partie en cours"), _("Eliot: erreur"),
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
// XXX:        msg << _("Problème de prévisualisation.\n")
        msg << _("Probleme de previsualisation.\n")
// XXX:            << _("Il se peut que l'imprimante par défaut soit mal initialisée");
            << _("Il se peut que l'imprimante par defaut soit mal initialisee");
// XXX:        wxMessageBox(msg, _("Impression (prévisualisation)"), wxOK);
        wxMessageBox(msg, _("Impression (previsualisation)"), wxOK);
        return;
    }
    wxPreviewFrame *frame = new wxPreviewFrame(preview, this, _("Impression"),
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
        wxMessageBox(_("Pas de partie en cours"), _("Eliot: erreur"),
                     wxICON_INFORMATION | wxOK);
        return;
    }
    wxString txt;
    wxFileDialog dialog(this, _("Imprimer dans un fichier PostScript"), wxT(""), wxT(""), wxT("*.ps"), wxSAVE|wxOVERWRITE_PROMPT);
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
// XXX:                wxMessageBox(_("Impression non effectuée."));
                wxMessageBox(_("Impression non effectuee."));
            }
            else
            {
                wxString msg;
// XXX:                msg << _("Dessin effectué dans ") << dialog.GetPath() << _("\n");
                msg << _("Dessin effectue dans ") << dialog.GetPath() << _("\n");
                wxMessageBox(msg, _("Sauvegarde PostScript"), wxOK);
            }
        }
        else
        {
            wxString msg;
            msg << _("impossible d'initialiser le traitement PostScript.\n");
            wxMessageBox(msg, _("Sauvegarde PostScript"), wxOK);
        }
    }
#endif
}


// *******************
// Dictionnary Loading
// *******************

void
MainFrame::OnMenuConfGameDic(wxCommandEvent& WXUNUSED(event))
{
    wxString txt, msg, dicpath;
    wxFileDialog dialog(this, _("Choisir un dictionnaire"), wxT(""), wxT("*.dawg"), wxT("*.dawg"), wxOPEN);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxString dicpath = dialog.GetPath();
        Dictionary dic;
        int res = Dic_load(&dic, dicpath.mb_str());
        if (res == 0)
        {
            if (m_dic)
	      {
                Dic_destroy(m_dic);
	      }

            m_dic = dic;
            config.setDicPath(dialog.GetPath(), ::wxFileNameFromPath(dialog.GetPath()));
        }
        else
        {
            switch (res)
            {
                case 0: /* cas normal */ break;
// XXX:                case 1: msg << _("chargement: problème d'ouverture de ") << dicpath << _("\n"); break;
                case 1: msg << _("chargement: probleme d'ouverture de ") << dicpath << _("\n"); break;
// XXX:                case 2: msg << _("chargement: mauvais en-tête de dictionnaire\n"); break;
                case 2: msg << _("chargement: mauvais en-tete de dictionnaire\n"); break;
// XXX:                case 3: msg << _("chargement: problème 3 d'allocation mémoire\n"); break;
                case 3: msg << _("chargement: probleme 3 d'allocation memoire\n"); break;
// XXX:                case 4: msg << _("chargement: problème 4 d'allocation mémoire\n"); break;
                case 4: msg << _("chargement: probleme 4 d'allocation memoire\n"); break;
// XXX:                case 5: msg << _("chargement: problème de lecture des arcs du dictionnaire\n"); break;
                case 5: msg << _("chargement: probleme de lecture des arcs du dictionnaire\n"); break;
// XXX:                default: msg << _("chargement: problème non-répertorié\n"); break;
                default: msg << _("chargement: probleme non-repertorie\n"); break;
            }
            wxMessageDialog dlg(NULL, msg, wxT(APPNAME));
            dlg.ShowModal();
        }
    }
    UpdateStatusBar();
    UpdateFrames();
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
//   MENU QUIT
//**************************************************************************************

void
MainFrame::OnMenuQuitApropos(wxCommandEvent& WXUNUSED(event))
{
    wxString msg;
    // XXX:    msg << wxT("Eliot\n© Antoine Fraboulet 1999-2004\n\n");
    msg << wxT("Eliot\nCopyright Antoine Fraboulet 1999-2006\n\n");
    msg << wxT("This program is free software; you can redistribute it and/or modify\n");
    msg << wxT("it under the terms of the GNU General Public License as published by\n");
    msg << wxT("the Free Software Foundation; either version 2 of the License, or\n");
    msg << wxT("(at your option) any later version.\n\n");
    msg << wxT("Version ") << wxT(VERSION) << wxT("\n");
    wxMessageBox(msg, _("A propos d'Eliot"), wxICON_INFORMATION | wxOK);
}

void
MainFrame::OnMenuQuitConfirm(wxCommandEvent& WXUNUSED(event))
{
    Close(TRUE);
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
    
    for(int i=0 ; i < MAX_FRAME_ID; i++)
	{
	    if (auxframes_ptr[i] != NULL)
		{
		    debug("   delete frame %d\n",i);
		    delete auxframes_ptr[i];
		}
	}
    
    auxframes_ptr[ ID_Frame_Verif  ] = new VerifFrame (this, m_game->getDic());
    debug("0 : Verif\n");
    auxframes_ptr[ ID_Frame_Search ] = new SearchFrame(this, m_game->getDic());
    debug("1 : Search\n");
    auxframes_ptr[ ID_Frame_Plus1  ] = new Plus1Frame (this, m_game);
    debug("2 : Plus1\n");
    auxframes_ptr[ ID_Frame_Racc   ] = new RaccFrame  (this, m_game);
    debug("3 : Racc\n");
    auxframes_ptr[ ID_Frame_Benj   ] = new BenjFrame  (this, m_game);
    debug("4 : Benj\n");
    auxframes_ptr[ ID_Frame_Bag    ] = new BagFrame   (this, *m_game);
    debug("5 : Bag\n");
    auxframes_ptr[ ID_Frame_Board  ] = new BoardFrame (this, *m_game);
    debug("6 : Board\n");
    auxframes_ptr[ ID_Frame_Game   ] = new GameFrame  (this, *m_game);
    debug("7 : Game\n");
#ifndef ENABLE_RESLIST_IN_MAIN
    auxframes_ptr[ ID_Frame_Result ] = new ResultFrame(this, m_game);
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
	    text << _("coup:") << (m_game->getHistory().getSize() + 1) << wxT(" ");
	    text << _("points:") << (m_game->getCurrentPlayer().getPoints());
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
    int res = 0;
    wxString msg;
    bool check = config.getRackChecking();

    static_cast<Training*>(m_game)->removeTestPlay();
    std::wstring str = srack.c_str();
    res = static_cast<Training*>(m_game)->setRack(mode, check, str);

    switch (res)
	{
        case 0x00: /* ok */
	    debug("SetRack Ok :: ");
	    break;
        case 0x01:
            msg = _("Le sac ne contient pas assez de lettres\npour assurer le tirage.");
            wxMessageBox(msg, _("Correction du tirage"), wxICON_INFORMATION | wxOK);
            return;
        case 0x02:
            msg = _("Le tirage doit contenir au moins 2 consonnes et 2 voyelles.\n");
            wxMessageBox(msg, _("Correction du tirage"), wxICON_INFORMATION | wxOK);
            return;
        case 0x03:
	    msg  = _("Le tirage doit contenir au moins 2 consonnes et 2 voyelles\n");
	    msg += _("mais le sac ne contient plus assez de lettres.\n\n");
	    wxMessageBox(msg, _("Correction du tirage"), wxICON_INFORMATION | wxOK);
            break;
	default:
	    statusbar->SetStatusText(_("Le tirage a ete modifie manuellement"), 0);
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
    ((Training*)m_game)->removeTestPlay();
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
    ((Training*)m_game)->removeTestPlay();
    if (n < 0)
	{
	    debug("MainFrame::Play back %d\n",n);
	    m_game->back(- n);
	}
    else
	{
	    int n=0;
	    debug("MainFrame::Play +%d\n",n);
#ifdef ENABLE_RESLIST_IN_MAIN
	    n = reslist->GetSelected();
#else
	    n = ((ResultFrame*)auxframes_ptr[ ID_Frame_Result ])->GetSelected();
#endif
	    if (n > -1)
		{
		    ((Training*)m_game)->playResult(n);
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
    ((Training*)m_game)->removeTestPlay();
    ((Training*)m_game)->testPlay(n);
    UpdateFrames();
    UpdateStatusBar();
}

/****************************************************************/
/****************************************************************/

/// Local Variables:
/// mode: c++
/// mode: hs-minor
/// c-basic-offset: 4
/// indent-tabs-mode: nil
/// End:
