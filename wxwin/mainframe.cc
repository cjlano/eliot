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

/* $Id: mainframe.cc,v 1.4 2005/01/01 15:42:55 ipkiss Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

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
  Menu_Conf_Aspect_BoardColour_Default      = 2209,

#define IDBASE 3300

  Menu_ShowBoard                            = (IDBASE + ID_Frame_Board),
  Menu_ShowVerif                            = (IDBASE + ID_Frame_Verif),
  Menu_ShowSearch                           = (IDBASE + ID_Frame_Search),
  Menu_ShowPlus1                            = (IDBASE + ID_Frame_Plus1),
  Menu_ShowRacc                             = (IDBASE + ID_Frame_Racc),
  Menu_ShowBenj                             = (IDBASE + ID_Frame_Benj),
  Menu_ShowBag                              = (IDBASE + ID_Frame_Bag),

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
  EVT_MENU_RANGE(Menu_ShowVerif, Menu_ShowBoard, MainFrame::OnMenuShowFrame)
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
  EVT_TEXT_ENTER(Rack_ID, MainFrame::OnSearch)
  //
  EVT_LIST_ITEM_SELECTED  (ListCtrl_ID, MainFrame::OnListCtrlSelected)
  EVT_LIST_ITEM_ACTIVATED (ListCtrl_ID, MainFrame::OnListCtrlActivated)
  //
  EVT_CLOSE(MainFrame::OnCloseWindow)
  //
END_EVENT_TABLE()

  /** ******************************
   *
   *
   ******************************* */

MainFrame::MainFrame(wxPoint pos_, wxSize size_)
    : wxFrame((wxFrame *) NULL, -1, wxT("Eliot"), wxPoint(-1,-1),
              size_, wxDEFAULT_FRAME_STYLE, wxT("Eliot"))
{
  wxSysColourChangedEvent event;

  Dictionary dic = NULL;
  wxString dicpath = config.getDicPath();
  Dic_load(&dic, dicpath.mb_str());

  game = Game_create(dic);
  Game_training_start(game);

  rack = new wxTextCtrl(this,Rack_ID,wxT(""),wxPoint(-1,-1),wxSize(-1,-1),wxTE_PROCESS_ENTER);
  rack->SetToolTip(wxT("Tirage"));

  results = new wxListCtrl(this,ListCtrl_ID);
#if defined(ENABLE_LC_NO_HEADER)
  results->SetSingleStyle(wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL);
#else
  results->SetSingleStyle(wxLC_REPORT | wxLC_SINGLE_SEL);
#endif
  results->InsertColumn(0,wxT("Sol"));
  results->InsertColumn(1,wxT("*"));
  results->InsertColumn(2,wxT("Pos"));
  results->InsertColumn(3,wxT("Pts"));
  results->SetToolTip(wxT("Résultats de la recherche"));

  InitFrames();
  InitMenu();

  statusbar = CreateStatusBar(2,0,Status_ID);
  int ww[2] = {-1, 160};
  statusbar->SetStatusWidths(2,ww);
  UpdateStatusBar();

  b_rackrandomset = new wxButton(this,Button_SetRack,wxT(" Tirage "));
  b_rackrandomset->SetToolTip(wxT("Tirage aléatoire"));
  b_rackrandomnew = new wxButton(this,Button_SetNew,wxT(" Complément "));
  b_rackrandomnew->SetToolTip(wxT("Complément aléatoire du tirage"));
  b_search = new wxButton(this,Button_Search,wxT(" Rechercher "));
  b_search->SetToolTip(wxT("Recherche sur le tirage courant"));
  b_back = new wxButton(this,Button_PlayBack,wxT(" Arrière "));
  b_back->SetToolTip(wxT("Revenir un coup en arrière"));
  b_play = new wxButton(this,Button_Play,wxT(" Jouer "));
  b_play->SetToolTip(wxT("Jouer le mot sélectionné"));

  wxBoxSizer *buttonsizer = new wxBoxSizer(wxHORIZONTAL);
  buttonsizer->Add(b_rackrandomset, 1, wxEXPAND | wxTOP | wxBOTTOM | wxLEFT , 1);
  buttonsizer->Add(b_rackrandomnew, 1, wxEXPAND | wxTOP | wxBOTTOM          , 1);
  buttonsizer->Add(b_search,        1, wxEXPAND | wxTOP | wxBOTTOM          , 1);
  buttonsizer->Add(b_back,          1, wxEXPAND | wxTOP | wxBOTTOM          , 1);
  buttonsizer->Add(b_play,          1, wxEXPAND | wxTOP | wxBOTTOM | wxRIGHT, 1);

  wxBoxSizer *listsizer = new wxBoxSizer(wxVERTICAL);
  listsizer->Add(rack    ,0 ,wxEXPAND | wxALL, 1);
  listsizer->Add(results ,1 ,wxEXPAND | wxLEFT | wxRIGHT, 1);

  wxBoxSizer *mainsizer = new wxBoxSizer(wxVERTICAL);
  mainsizer->Add(listsizer  , 1, wxEXPAND | wxVERTICAL, 0);
  mainsizer->Add(buttonsizer, 0, wxEXPAND             , 0);

  SetAutoLayout(TRUE);
  SetSizer(mainsizer);
  mainsizer->Fit(this);
  mainsizer->SetSizeHints(this);

  SetClientSize(size_);
  Move(config.getFramePos(wxT(APPNAME)));
}

  /** ******************************
   *
   *
   ******************************* */

MainFrame::~MainFrame()
{
  Dictionary dic;

  if ((dic = Game_getdic(game)) != NULL)
    {
      Dic_destroy(dic);
    }

  config.setFramePos(wxT(APPNAME),GetPosition());
  config.setFrameSize(wxT(APPNAME),GetClientSize());

  Game_destroy(game);
}

 /** ******************************
   *
   *
   ******************************* */

void
MainFrame::InitMenu()
{
  // menus
  wxMenu *menu_game = new wxMenu;
  menu_game->Append(Menu_Game_New,wxT("Nouvelle"),wxT("Démarrer une nouvelle partie"));
  menu_game->Append(Menu_Game_Open,wxT("Charger..."),wxT("Charger une partie"));
  menu_game->Append(Menu_Game_Save,wxT("Sauver..."),wxT("Sauver cette partie"));
  menu_game->AppendSeparator();
  menu_game->Append(Menu_Game_Print,wxT("Imprimer..."),wxT("Imprimer cette partie"));
  menu_game->Append(Menu_Game_PrintPreview,wxT("Préimpression"),wxT("Préimpression de la partie"));
#ifdef ENABLE_SAVE_POSTSCRIPT
  menu_game->AppendSeparator();
  menu_game->Append(Menu_Game_PrintPS,wxT("Imprimer du PostScript"),wxT("Imprimer dans un fichier PostScript"));
#endif
  //
  wxMenu *menu_conf_game = new wxMenu;
  menu_conf_game->Append(Menu_Conf_Game_Dic,wxT("Dictionnaire"),wxT("Choix du dictionnaire"));
  menu_conf_game->Append(Menu_Conf_Game_Search,wxT("Recherche"),wxT("Options de recherche"));
  //
  wxMenu *menu_conf_board_colour = new wxMenu;
  menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Background,wxT("Fond"),wxT("Couleur du fond"));
  menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Lines,wxT("Lignes"),wxT("Couleur des lignes"));
  menu_conf_board_colour->AppendSeparator();
  menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Letters,wxT("Lettres jouées"),wxT("Lettres jouées sur la grille"));
  menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_TestLetters,wxT("Lettres provisoires"),wxT("Lettres du mot à jouer"));
  menu_conf_board_colour->AppendSeparator();
  menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Wx2,wxT("Mot compte double"),wxT("Mot compte double"));
  menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Wx3,wxT("Mot compte triple"),wxT("Mot compte triple"));
  menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Lx2,wxT("Lettre compte double"),wxT("Lettre compte double"));
  menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Lx3,wxT("Lettre compte triple"),wxT("Lettre compte triple"));
  menu_conf_board_colour->AppendSeparator();
  menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Default,wxT("Couleurs d'origine"),wxT("Retrouver les couleurs d'origine"));
  //
  wxMenu *menu_conf_board_font = new wxMenu;
  menu_conf_board_font->Append(Menu_Conf_Aspect_Font_Search,wxT("Lettres de recherche"),wxT("Police de caractères pour les recherches"));
  menu_conf_board_font->Append(Menu_Conf_Aspect_Font_Board,wxT("Lettres de la grille"),wxT("Police de caractères de la grille"));
  //
  wxMenu *menu_conf = new wxMenu;
  menu_conf->Append(Menu_Conf_Game,wxT("Jeu"),menu_conf_game,wxT("Configuration du jeu"));
  menu_conf->Append(Menu_Conf_Aspect_Font,wxT("Fonte des lettres"),menu_conf_board_font,wxT("Modification des fontes"));
  menu_conf->Append(Menu_Conf_Aspect_BoardColour,wxT("Couleurs de la grille"),menu_conf_board_colour,wxT("Modification des couleurs"));
  menu_conf->Append(Menu_Conf_Print,wxT("Impression"),wxT("Dimensions de la partie"));
  //
  wxMenu *menu_frame = new wxMenu;
  menu_frame->Append(Menu_ShowBoard,wxT("Grille"),wxT("Grille de jeu"));
  menu_frame->Append(Menu_ShowVerif,wxT("Vérification"),wxT("Vérification d'un mot dans le dictionnaire"));
  menu_frame->Append(Menu_ShowSearch,wxT("Recherche"),wxT("Recherche dans le dictionnaire"));
  menu_frame->AppendSeparator();
  menu_frame->Append(Menu_ShowPlus1,wxT("Tirage + 1"),wxT("Lettres du tirage plus une"));
  menu_frame->Append(Menu_ShowRacc,wxT("Raccords"),wxT("Raccords sur un mot de la recherche"));
  menu_frame->Append(Menu_ShowBenj,wxT("Benjamins"),wxT("Benjamins sur un mot de la recherche"));
  menu_frame->AppendSeparator();
  menu_frame->Append(Menu_ShowBag,wxT("Sac"),wxT("Lettres restantes dans le sac"));
  //
  wxMenu *menu_quit = new wxMenu;
  menu_quit->Append(Menu_Quit_Apropos,wxT("A propos..."),wxT("A propos d'Eliot"));
  menu_quit->Append(Menu_Quit_Confirm,wxT("Quitter"),wxT("Quitter"));
  //
  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(menu_game,wxT("Partie"));
  menu_bar->Append(menu_conf,wxT("Configuration"));
  menu_bar->Append(menu_frame,wxT("Fenêtres"));
  menu_bar->Append(menu_quit,wxT("Quitter"));

  SetMenuBar(menu_bar);
}

  /** ******************************
   *
   *
   ******************************* */

void
MainFrame::OnCloseWindow(wxCloseEvent&)
{
  this->Destroy();
}


void
MainFrame::UpdateStatusBar()
{
  wxString text;

  text << config.getDicName();
  text << wxT(" ");
  text << config.getTileName();
  statusbar->SetStatusText(text,0);

  text = wxT("");
  text << wxT("coup:") << (Game_getnrounds(game) + 1)
       << wxT(" ")
       << wxT("points:") << Game_getplayerpoints(game, 0);
  statusbar->SetStatusText(text,1);
}

//*****************************************************************************
//   MENU GAME
//*****************************************************************************

void
MainFrame::OnMenuGameNew(wxCommandEvent&)
{
  Game_init(game);
  Game_training_start(game);
  rack->SetValue(wxT(""));
  results->DeleteAllItems();
  UpdateStatusBar();
  UpdateFrames();
}

  /** ******************************
   *
   *
   ******************************* */

void
MainFrame::OnMenuGameOpen(wxCommandEvent&)
{
  wxString txt;
  wxFileDialog dialog(this,wxT("Ouvrir une partie"), wxT(""),wxT(""),wxT("*"),wxOPEN);
  if (Game_getdic(game) == NULL) {
    wxMessageBox(wxT("Il n'y a pas de dictionnaire sélectionné"), wxT("Eliot: erreur"),
                 wxICON_INFORMATION | wxOK);
    return;
  }
  if (dialog.ShowModal() == wxID_OK)
    {
      FILE* fin;
      if ((fin = fopen(dialog.GetPath().mb_str(), "r")) == NULL)
        {
          txt << wxT("Impossible d'ouvrir") << dialog.GetPath();
          wxMessageDialog msg(this, txt, wxT("Ouverture d'une partie"));
          msg.ShowModal();
          return ;
        }
      Game_init(game);
      Game_training_start(game);
      switch (Game_load(game,fin)) {
      case 0: // everything is ok
        break;
      case 1:
        {
      wxMessageDialog msg(this,wxT("Format de fichier inconnu"),wxT("Chargement de partie"));
          msg.ShowModal();
        }
        break;
      default:
        {
          wxMessageDialog msg(this,wxT("Erreur pendant la lecture de la partie"),wxT("chargement de partie"));
          msg.ShowModal();
        }
        break;
      }
      fclose(fin);
    }
  char r[RACK_SIZE_MAX];
  Game_getplayedrack(game,Game_getnrounds(game),r);
  rack->SetValue(wxU(r));
  results->DeleteAllItems();
  UpdateStatusBar();
  UpdateFrames();
}

  /** ******************************
   *
   *
   ******************************* */

void
MainFrame::OnMenuGameSave(wxCommandEvent& WXUNUSED(event))
{
  wxString txt;
  wxFileDialog dialog(this, wxT("Sauver une partie"), wxT(""), wxT(""), wxT("*"),wxSAVE|wxOVERWRITE_PROMPT);
  if (dialog.ShowModal() == wxID_OK)
    {
      FILE* fout;
      if ((fout = fopen(dialog.GetPath().mb_str(),"w")) == NULL)
        {
          txt << wxT("Impossible de créer ") << dialog.GetPath();
          wxMessageDialog msg(this, txt, wxT("Sauvegarde de la partie"));
          msg.ShowModal();
          return ;
        }
      Game_save(game,fout);
      fclose(fout);
    }
}

  /** ******************************
   *
   *
   ******************************* */

void
MainFrame::OnMenuGamePrint(wxCommandEvent& WXUNUSED(event))
{
  wxPrintDialogData printDialogData(config.getPrintData());
  wxPrinter printer(&printDialogData);
  GamePrintout printout(game);
  if (!printer.Print(this,&printout,TRUE))
    wxMessageBox(wxT("Impression non effectuée."));
}

  /** ******************************
   *
   *
   ******************************* */

void
MainFrame::OnMenuGamePrintPreview(wxCommandEvent& WXUNUSED(event))
{
  wxPrintData printdata = config.getPrintData();

  wxString msg;
  wxPrintPreview *preview = new wxPrintPreview(new GamePrintout(game),
                                               new GamePrintout(game), & printdata);
  if (!preview->Ok())
    {
      delete preview;
      msg << wxT("Problème de prévisualisation.\n")
          << wxT("Il se peut que l'imprimante par défaut soit mal initialisée");
      wxMessageBox(msg,wxT("Impression (prévisualisation)"), wxOK);
      return;
    }
  wxPreviewFrame *frame = new wxPreviewFrame(preview, this, wxT("Impression"),
                                             wxPoint(-1, -1), wxSize(600, 550));
  frame->Centre(wxBOTH);
  frame->Initialize();
  frame->Show(TRUE);
}

  /** ******************************
   *
   *
   ******************************* */

void
MainFrame::OnMenuGamePrintPS(wxCommandEvent& WXUNUSED(event))
{
#ifdef ENABLE_SAVE_POSTSCRIPT
  wxString txt;
  wxFileDialog dialog(this, wxT("Imprimer dans un fichier PostScript"), wxT(""), wxT(""), wxT("*.ps"),wxSAVE|wxOVERWRITE_PROMPT);
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
          GamePrintout printout(game);
          if (!printer.Print(this,&printout,FALSE))
            {
              wxMessageBox(wxT("Impression non effectuée."));
            }
          else
            {
              wxString msg;
              msg << wxT("Dessin effectué dans ") << dialog.GetPath() << wxT("\n");
              wxMessageBox(msg,wxT("Sauvegarde PostScript"), wxOK);
            }
        }
      else
        {
          wxString msg;
          msg << wxT("impossible d'initialiser le traitement PostScript.\n");
          wxMessageBox(msg,wxT("Sauvegarde PostScript"), wxOK);
        }
    }
#endif
}

//
//   MENU CONFIG
//

  /** ******************************
   *
   *
   ******************************* */

void
MainFrame::OnMenuConfGameDic(wxCommandEvent& WXUNUSED(event))
{
  wxString txt,msg,dicpath;
  Dictionary dic,dicold;
  wxFileDialog dialog(this,wxT("Choisir un dictionnaire"), wxT(""),wxT("*.dawg"),wxT("*.dawg"),wxOPEN);
  if (dialog.ShowModal() == wxID_OK)
    {
      wxString dicpath = dialog.GetPath();
      int res=Dic_load(&dic, dicpath.mb_str());
      if (res == 0)
        {
          /* cas normal */
          if ((dicold = Game_getdic(game)) != NULL)
            Dic_destroy(dicold);
          Game_setdic(game,dic);
          config.setDicPath(dialog.GetPath(),::wxFileNameFromPath(dialog.GetPath()));
        }
      else
        {
          switch (res)
            {
            case 0: /* cas normal */ break;
            case 1: msg << wxT("chargement: problème d'ouverture de ") << dicpath << wxT("\n");        break;
            case 2: msg << wxT("chargement: mauvais en-tête de dictionnaire\n"); break;
            case 3: msg << wxT("chargement: problème 3 d'allocation mémoire\n"); break;
            case 4: msg << wxT("chargement: problème 4 d'allocation mémoire\n"); break;
            case 5: msg << wxT("chargement: problème de lecture des arcs du dictionnaire\n"); break;
            default: msg << wxT("chargement: problème non-répertorié\n"); break;
            }
          wxMessageDialog dlg(NULL, msg, wxT(APPNAME));
          dlg.ShowModal();
        }
    }
  UpdateStatusBar();
  UpdateFrames();
}

  /** ******************************
   *
   *
   ******************************* */

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
    case Menu_Conf_Aspect_Font_Search: attr = wxString(LISTFONT); break;
    case Menu_Conf_Aspect_Font_Board: attr = wxString(BOARDFONT); break;
    case Menu_Conf_Aspect_Font_Default: attr = wxT("Default"); break;
    default: INCOMPLETE; break;
    }

  if (attr == wxString(wxT("Default")))
    config.setFontDefault();
  else
    config.setFont(attr,config.ChooseFont(this,config.getFont(attr)));

  UpdateFrames(FORCE_REFRESH);
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
    case Menu_Conf_Aspect_BoardColour_Lines: attr = wxString(BCOLOURLINES); break;
    case Menu_Conf_Aspect_BoardColour_Wx2: attr = wxString(BCOLOURWX2); break;
    case Menu_Conf_Aspect_BoardColour_Wx3: attr = wxString(BCOLOURWX3); break;
    case Menu_Conf_Aspect_BoardColour_Lx2: attr = wxString(BCOLOURLX2); break;
    case Menu_Conf_Aspect_BoardColour_Lx3: attr = wxString(BCOLOURLX3); break;
    case Menu_Conf_Aspect_BoardColour_Background: attr = wxString(BCOLOURBACKGROUND); break;
    case Menu_Conf_Aspect_BoardColour_Letters: attr = wxString(BCOLOURLETTERS); break;
    case Menu_Conf_Aspect_BoardColour_TestLetters: attr = wxString(BCOLOURTSTLETTERS); break;
    case Menu_Conf_Aspect_BoardColour_Default: attr = wxT("Default"); break;
    default: INCOMPLETE; break;
    }

  if (attr == wxString(wxT("Default")))
    config.setColourDefault();
  else
    config.setColour(attr,config.ChooseColour(this,config.getColour(attr)));

  UpdateFrames(FORCE_REFRESH);
}

//**************************************************************************************
//   MENU QUIT
//**************************************************************************************

void
MainFrame::OnMenuQuitApropos(wxCommandEvent& WXUNUSED(event))
{
  wxString msg;

  msg << wxT("Eliot\n© Antoine Fraboulet 1999-2004\n\n");
  msg << wxT("This program is free software; you can redistribute it and/or modify\n");
  msg << wxT("it under the terms of the GNU General Public License as published by\n");
  msg << wxT("the Free Software Foundation; either version 2 of the License, or\n");
  msg << wxT("(at your option) any later version.\n\n");
  msg << wxT("Version ") << wxT(VERSION) << wxT("\n");

  wxMessageBox(msg, wxT("A propos d'Eliot"), wxICON_INFORMATION | wxOK);
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
  int res,check;
  set_rack_mode mode = RACK_NEW;
  char r[RACK_SIZE_MAX];
  char oldr[RACK_SIZE_MAX];
  check = config.getRackChecking();
  wxString msg;

  switch ((id = event.GetId())) {
  case Button_SetRack: mode = RACK_ALL; break;
  case Button_SetNew:  mode = RACK_NEW; break;
  case Button_SetManual: break;
  default: return;
  }

  Game_getplayedrack(game,Game_getnrounds(game),oldr);
  res = Game_training_setrackrandom(game,check,mode);

  switch (res) {
  case 0x00: /* ok */
    break;
  case 0x01:
    msg << wxT("Le sac ne contient plus assez de lettres.") << wxT("\n");
    wxMessageBox(msg,wxT("Correction du tirage"), wxICON_INFORMATION | wxOK);
    break;
  case 0x02:
    {
      msg << wxT("Le tirage doit contenir au moins 2 consonnes et 2 voyelles") << wxT("\n");
      wxMessageDialog dlg(this, msg, wxT("Correction du tirage"));
      dlg.ShowModal();
    }
    break;
  case 0x03:
    {
      msg << wxT("Le tirage doit contenir au moins 2 consonnes et 2 voyelles") << wxT("\n")
          << wxT("mais le sac ne contient plus assez de lettres") << wxT("\n") << wxT("\n");
      wxMessageDialog dlg(this, msg, wxT("Correction du tirage"));
      dlg.ShowModal();
    }
    break;
  default:
    INCOMPLETE;
    break;
  }

  Game_getplayedrack(game,Game_getnrounds(game),r);
  rack->SetValue(wxU(r));
  Game_removetestplay(game);
  results->DeleteAllItems();
  UpdateFrames();
}

void
MainFrame::Search()
{
  int i;

  Game_training_search(game);

  // to speed up inserting we hide the control temporarily
  // but this is not good on slow machines as it shows an empty
  // square instead of the list
  //results->Hide();
  results->DeleteAllItems();
  results->SetFont(config.getFont(LISTFONT));
  for(i=0; i < Game_getnresults(game); i++)
    {
      char word[WORD_SIZE_MAX];
      char bonus;
      char coord1[3], coord2[3], coord[6];
      char pts[6];

      Game_getsearchedword       (game,i,word);
      Game_getsearchedfirstcoord (game,i,coord1);
      Game_getsearchedsecondcoord(game,i,coord2);
      sprintf(coord,"%2s%2s",coord1,coord2);
      bonus = Game_getsearchedbonus(game,i) ? '*' : ' ';
      sprintf(pts,"%3d",Game_getsearchedpoints(game,i));

      long tmp = results->InsertItem(i,wxU(word));
      results->SetItemData(tmp,i);
      tmp = results->SetItem(i,1,(wxChar)bonus);
      tmp = results->SetItem(i,2,wxU(coord));
      tmp = results->SetItem(i,3,wxU(pts));
    }

  for(i=0; i < 4; i++)
    results->SetColumnWidth( i, wxLIST_AUTOSIZE );

  results->Show();
  if (Game_getnresults(game))
    {
      results->SetItemState(0,wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED | wxLIST_MASK_STATE);
      Game_testplay(game,0);
    }
}

void
MainFrame::OnSearch(wxCommandEvent& WXUNUSED(event))
{
  wxString msg;
  char r[RACK_SIZE_MAX];
  if (Game_getdic(game) == NULL)
    {
      wxMessageBox(wxT("Vous devez choisir un dictionnaire"),wxT("Eliot: erreur"),wxICON_INFORMATION | wxOK);
      return;
    }

  Game_removetestplay(game);

  switch (Game_training_setrackmanual(game, config.getRackChecking(), rack->GetValue().mb_str()))
    {
    case 0x00: break;
    case 0x01:
      msg << wxT("Le sac ne contient pas assez de lettres") << wxT("\n")
          << wxT("pour assurer le tirage.");
      wxMessageBox(msg,wxT("Correction du tirage"), wxICON_INFORMATION | wxOK);
      return;
    case 0x02:
      msg << wxT("Le tirage doit contenir au moins 2 consonnes et 2 voyelles") << wxT("\n");
      wxMessageBox(msg,wxT("Correction du tirage"), wxICON_INFORMATION | wxOK);
      return;
    default: statusbar->SetStatusText(wxT("Le tirage a été modifié manuellement"),0); break;
    }

  Game_getplayedrack(game,Game_getnrounds(game),r);
  rack->SetValue(wxU(r));

  Search();

  UpdateStatusBar();
  UpdateFrames();
}

void
MainFrame::Play(int n)
{
  char r[RACK_SIZE_MAX];
  Game_removetestplay(game);

   if (n == -1)
    Game_back(game,1);
  else
    Game_training_playresult(game,n);

  Game_getplayedrack(game,Game_getnrounds(game),r);
  rack->SetValue(wxU(r));
  results->DeleteAllItems();
  UpdateStatusBar();
  UpdateFrames();
}

void
MainFrame::OnPlay(wxCommandEvent& event)
{
  int id;
  long item = -1;

  id= event.GetId();
  switch (id) {
  case Button_Play:
    item = results->GetNextItem(item,wxLIST_NEXT_ALL,wxLIST_STATE_SELECTED);
    if (item > -1) Play(item);
    break;
  case Button_PlayBack: Play(-1); break;
  default: return; break;
  }
}

void
MainFrame::OnListCtrlSelected(wxListEvent& event)
{
  Game_removetestplay(game);
  Game_testplay(game,event.m_itemIndex);
  UpdateFrames();
}

void
MainFrame::OnListCtrlActivated(wxListEvent& event)
{
  if (event.m_itemIndex > -1)
    Play(event.m_itemIndex);
}

//*********************************
// SPECIAL FRAMES
//*********************************

void
MainFrame::InitFrames()
{
  debug("InitFrames 1\n");

  auxframes_ptr[ID_Frame_Verif]  = new VerifFrame (this,Game_getdic(game));
  auxframes_ptr[ID_Frame_Search] = new SearchFrame(this,Game_getdic(game));
  auxframes_ptr[ID_Frame_Plus1]  = new Plus1Frame (this,game);
  auxframes_ptr[ID_Frame_Racc]   = new RaccFrame  (this,game,results);
  auxframes_ptr[ID_Frame_Benj]   = new BenjFrame  (this,game,results);
  auxframes_ptr[ID_Frame_Bag]    = new BagFrame   (this,game);
  auxframes_ptr[ID_Frame_Board]  = new BoardFrame (this,game);

  debug("InitFrames 2\n");
  for(int i=MIN_FRAME_ID; i < MAX_FRAME_ID; i++)
    {
      if (auxframes_ptr[i] == NULL)
        debug("auxframe NULL %d",i);
      debug("  Reload %d\n",i);
      auxframes_ptr[i]->Reload();
    }
  debug("InitFrames final\n");
}

void
MainFrame::OnMenuShowFrame(wxCommandEvent& event)
{
  int id;
  id = event.GetId();

  debug("switch display auxframe %d\n",id);

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
      debug("ShowFrame: auxframes_ptr[%d] == NULL\n",id);
      return;
    }
  auxframes_ptr[id]->SwitchDisplay();
}

void
MainFrame::UpdateFrames(refresh_t force)
{
  int id;
  for(id=0; id < MAX_FRAME_ID; id++)
    {
      if (auxframes_ptr[id])
        auxframes_ptr[id]->Refresh(force);
    }
}
