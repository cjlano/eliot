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

/* $Id: mainframe.cc,v 1.1 2004/04/08 09:43:06 afrab Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

#include "ewx.h"

#include "dic.h"
#include "game.h"

#include "configdb.h"
#include "confdimdlg.h"
#include "confsearch.h"
#include "printout.h"
#include "mainframe.h"

#include "wx/intl.h"
#include "wx/menu.h"
#include "wx/statusbr.h"
#include "wx/sizer.h"
#include "wx/filedlg.h"
#include "wx/msgdlg.h"

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
    : wxFrame((wxFrame *) NULL, -1, "Eliot", wxPoint(-1,-1),
	      size_, wxDEFAULT_FRAME_STYLE, wxString("Eliot"))
{
  wxSysColourChangedEvent event;

  Dictionary dic = NULL;  
  wxString dicpath = config.getDicPath();
  Dic_load(&dic, (const char*)dicpath);

  game = Game_create(dic);

  rack = new wxTextCtrl(this,Rack_ID,wxString(""),wxPoint(-1,-1),wxSize(-1,-1),wxTE_PROCESS_ENTER);
  rack->SetToolTip("Tirage");

  results = new wxListCtrl(this,ListCtrl_ID);
#if defined(ENABLE_LC_NO_HEADER)
  results->SetSingleStyle(wxLC_REPORT | wxLC_NO_HEADER | wxLC_SINGLE_SEL);
#else
  results->SetSingleStyle(wxLC_REPORT | wxLC_SINGLE_SEL);
#endif
  results->InsertColumn(0,"Sol");
  results->InsertColumn(1,"*");
  results->InsertColumn(2,"Pos");
  results->InsertColumn(3,"Pts");
  results->SetToolTip("Résultats de la recherche");

  InitFrames();
  InitMenu();

  statusbar = CreateStatusBar(2,0,Status_ID);
  int ww[2] = {-1, 160};
  statusbar->SetStatusWidths(2,ww);
  UpdateStatusBar();

  b_rackrandomset = new wxButton(this,Button_SetRack," Tirage ");
  b_rackrandomset->SetToolTip("Tirage aléatoire");
  b_rackrandomnew = new wxButton(this,Button_SetNew," Complément ");
  b_rackrandomnew->SetToolTip("Complément aléatoire du tirage");
  b_search = new wxButton(this,Button_Search," Rechercher ");
  b_search->SetToolTip("Recherche sur le tirage courant");
  b_back = new wxButton(this,Button_PlayBack," Arrière ");
  b_back->SetToolTip("Revenir un coup en arrière");
  b_play = new wxButton(this,Button_Play," Jouer ");
  b_play->SetToolTip("Jouer le mot sélectionné");

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
  Move(config.getFramePos(APPNAME));
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

  config.setFramePos(APPNAME,GetPosition());  
  config.setFrameSize(APPNAME,GetClientSize());

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
  menu_game->Append(Menu_Game_New,"Nouvelle","Démarrer une nouvelle partie");
  menu_game->Append(Menu_Game_Open,"Charge","Charger une partie");
  menu_game->Append(Menu_Game_Save,"Sauver","Sauver cette partie");
  menu_game->AppendSeparator();
  menu_game->Append(Menu_Game_Print,"Imprimer","Imprimer cette partie");
  menu_game->Append(Menu_Game_PrintPreview,"Préimpression","Préimpression de la partie");
#ifdef ENABLE_SAVE_POSTSCRIPT
  menu_game->AppendSeparator();
  menu_game->Append(Menu_Game_PrintPS,"Imprimer du PostScript","Imprimer dans un fichier PostScript");
#endif
  //
  wxMenu *menu_conf_game = new wxMenu;
  menu_conf_game->Append(Menu_Conf_Game_Dic,"Dictionnaire","Choix du dictionnaire");
  menu_conf_game->Append(Menu_Conf_Game_Search,"Recherche","Options de recherches");
  //
  wxMenu *menu_conf_board_colour = new wxMenu;
  menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Background,"Fond","Couleur du fond");
  menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Lines,"Lignes","Couleurs des lignes");
  menu_conf_board_colour->AppendSeparator();
  menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Letters,"Lettres jouées","lettres jouées sur la grille");
  menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_TestLetters,"Lettres provisoires","lettre du mot à jouer");
  menu_conf_board_colour->AppendSeparator();
  menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Wx2,"Mot compte double","Mot compte double");
  menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Wx3,"Mot compte triple","Mot compte triple");
  menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Lx2,"Lettre compte double","Lettre compte double");
  menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Lx3,"Lettre compte triple","Lettre compte triple");
  menu_conf_board_colour->AppendSeparator();
  menu_conf_board_colour->Append(Menu_Conf_Aspect_BoardColour_Default,"Couleurs d'origine","Retrouver les couleurs d'origine");
  //
  wxMenu *menu_conf_board_font = new wxMenu;
  menu_conf_board_font->Append(Menu_Conf_Aspect_Font_Search,"Lettres de recherche","Police de caractère pour les recherches");
  menu_conf_board_font->Append(Menu_Conf_Aspect_Font_Board,"Lettres de la grille","Police de caractère de la grille");
  //
  wxMenu *menu_conf = new wxMenu;
  menu_conf->Append(Menu_Conf_Game,"Jeux",menu_conf_game,"Configuration du jeux");
  menu_conf->Append(Menu_Conf_Aspect_Font,"Fonte des lettres",menu_conf_board_font,"Modification des fontes");
  menu_conf->Append(Menu_Conf_Aspect_BoardColour,"Couleurs de la grille",menu_conf_board_colour,"Modification des couleurs");
  menu_conf->Append(Menu_Conf_Print,"Impression","Dimensions de la partie");
  //
  wxMenu *menu_frame = new wxMenu;
  menu_frame->Append(Menu_ShowBoard,"Grille","Grille de jeux");
  menu_frame->Append(Menu_ShowVerif,"Vérification","Vérification d'un mot dans le dictionnaire");
  menu_frame->Append(Menu_ShowSearch,"Recherche","Recherche dans le dictionnaire");
  menu_frame->AppendSeparator();
  menu_frame->Append(Menu_ShowPlus1,"Tirage + 1","Lettres du tirage plus une");
  menu_frame->Append(Menu_ShowRacc,"Raccords","Raccords sur un mot de la recherche");
  menu_frame->Append(Menu_ShowBenj,"Benjamins","Benjamins sur un mot de la recherche");
  menu_frame->AppendSeparator();
  menu_frame->Append(Menu_ShowBag,"Sac","Lettres restantes dans le sac");
  //
  wxMenu *menu_quit = new wxMenu;
  menu_quit->Append(Menu_Quit_Apropos,"A propos","A propos d'Eliot");
  menu_quit->Append(Menu_Quit_Confirm,"Quitter","Quitter");
  //
  wxMenuBar *menu_bar = new wxMenuBar;
  menu_bar->Append(menu_game,"Partie");
  menu_bar->Append(menu_conf,"Configurer");
  menu_bar->Append(menu_frame,"Fenêtres");
  menu_bar->Append(menu_quit,"Quitter");

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
  wxString text="";
  
  text << config.getDicName();
  text << " ";
  text << config.getTileName();
  statusbar->SetStatusText(text,0);

  text = "";
  text << "coup:" << (Game_getnrounds(game) + 1)
       << " " 
       << "points:" << Game_getpoints(game);
  statusbar->SetStatusText(text,1);
}

//**************************************************************************************
//   MENU GAME
//**************************************************************************************

void
MainFrame::OnMenuGameNew(wxCommandEvent&)
{
  Game_init(game);
  rack->SetValue(wxString(""));
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
  wxFileDialog dialog(this,"Ouvrir une partie", "","","*",wxOPEN);
  if (Game_getdic(game) == NULL) {
    wxMessageBox("Il n'y a pas de dictionnaire sélectionné", "Eliot: erreur",
		 wxICON_INFORMATION | wxOK);
    return;
  } 
  if (dialog.ShowModal() == wxID_OK)
    {
      FILE* fin;
      if ((fin = fopen((const char*)dialog.GetPath(),"r")) == NULL) 
	{
	  txt << "Impossible de d'ouvrir" << dialog.GetPath();
	  wxMessageDialog msg(this, txt, "Ouverture d'une partie");
	  msg.ShowModal();
	  return ;
	}
      Game_init(game);
      switch (Game_load(game,fin)) {
      case 0: // everything is ok
	break;
      case 1: 
	{ 
	  wxMessageDialog msg(this,"Format de fichier inconnu","chargement de partie");
	  msg.ShowModal(); 
	}
	break; 
      default: 
	{ 
	  wxMessageDialog msg(this,"Erreur pendant la lecture de la partie","chargement de partie");
	  msg.ShowModal(); 
	}
	break; 
      }
      fclose(fin);
    }
  char r[RACK_SIZE_MAX];
  Game_getplayedrack(game,Game_getnrounds(game),r);
  rack->SetValue(wxString(r));
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
  wxFileDialog dialog(this, "Sauver une partie", "", "", "*",wxSAVE|wxOVERWRITE_PROMPT);
  if (dialog.ShowModal() == wxID_OK)
    {
      FILE* fout;
      if ((fout = fopen((const char*)dialog.GetPath(),"w")) == NULL) 
	{
	  txt << "Impossible de créer " << dialog.GetPath();
	  wxMessageDialog msg(this, txt, "Sauvegarde de la partie");
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
    wxMessageBox("Impression non effectuée.");
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
      msg << "Problème de prévisualisation.\n"
	  << "Il se peut que l'imprimante par défaut soit mal initialisée";
      wxMessageBox(msg,"Impression (prévisualisation)", wxOK);
      return;
    }
  wxPreviewFrame *frame = new wxPreviewFrame(preview, this, "Impression",
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
  wxFileDialog dialog(this, "Imprimer dans un fichier PostScript", "", "", "*.ps",wxSAVE|wxOVERWRITE_PROMPT);
  if (dialog.ShowModal() == wxID_OK)
    {
      wxPostScriptDC printps(dialog.GetPath(),FALSE,this);
      if (printps.Ok())
	{
	  wxPrintData printdataPS;
	  printdataPS.SetPrintMode(wxPRINT_MODE_FILE);
	  printdataPS.SetFilename(dialog.GetPath());
	  printdataPS.SetPaperId(wxPAPER_A4);
	  printdataPS.SetQuality(wxPRINT_QUALITY_HIGH);
	  printdataPS.SetOrientation(wxPORTRAIT);

	  wxPrintDialogData printDialogData(printdataPS);
	  wxPostScriptPrinter printer(&printDialogData);
	  GamePrintout printout(game);
	  if (!printer.Print(this,&printout,FALSE))
	    {
	      wxMessageBox("Impression non effectuée.");
	    }
	  else
	    {
	      wxString msg;
	      msg << "Dessin effectué dans " << dialog.GetPath() << "\n";
	      wxMessageBox(msg,"Sauvegarde PostScript", wxOK);
	    }
	}
      else
	{
	  wxString msg;
	  msg << "impossible d'initialiser le traitement PostScript.\n";
	  wxMessageBox(msg,"Sauvegarde PostScript", wxOK);
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
  wxFileDialog dialog(this,"Choisir un dictionnaire", "","*.dawg","*.dawg",wxOPEN);
  if (dialog.ShowModal() == wxID_OK)
    {
      wxString dicpath = dialog.GetPath();
      int res=Dic_load(&dic,(const char*)dicpath);
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
	    case 1: msg << "chargement: problème d'ouverture de " << dicpath << "\n";	break;
	    case 2: msg << "chargement: mauvais en-tête de dictionnaire\n"; break;
	    case 3: msg << "chargement: problème 3 d'allocation mémoire\n"; break;
	    case 4: msg << "chargement: problème 4 d'allocation mémoire\n"; break;
	    case 5: msg << "chargement: problème de lecture des arcs du dictionnaire\n"; break;
	    default: msg << "chargement: problème non-répertorié\n"; break;
	    }
	  wxMessageDialog dlg(NULL,msg,APPNAME); 
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
    case Menu_Conf_Aspect_Font_Default: attr = wxString("Default"); break;
    default: INCOMPLETE; break;
    }

  if (attr == wxString("Default"))
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
    case Menu_Conf_Aspect_BoardColour_Default: attr = wxString("Default"); break; 
    default: INCOMPLETE; break;
    }

  if (attr == wxString("Default"))
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

  msg << "Eliot\n© Antoine Fraboulet 1999-2004\n\n";
  msg << "This program is free software; you can redistribute it and/or modify\n";
  msg << "it under the terms of the GNU General Public License as published by\n";
  msg << "the Free Software Foundation; either version 2 of the License, or\n";
  msg << "(at your option) any later version.\n\n";
  msg << "Version " << VERSION << "\n";

  wxMessageBox(msg, "A propos d'Eliot", wxICON_INFORMATION | wxOK);
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
  res = Game_setrack_random(game,check,mode);

  switch (res) {
  case 0x00: /* ok */ 
    break;
  case 0x01:
    msg << "Le sac ne contient plus assez de lettres." << "\n";
    wxMessageBox(msg,"Correction du tirage", wxICON_INFORMATION | wxOK);
    break;
  case 0x02: 
    {
      msg << "Le tirage doit contenir au moins 2 consonnes et 2 voyelles" << "\n";
      wxMessageDialog dlg(this, msg, "Correction du tirage");
      dlg.ShowModal();
    }
    break;
  case 0x03:
    {
      msg << "Le tirage doit contenir au moins 2 consonnes et 2 voyelles" << "\n"
	  << "mais le sac ne contient plus assez de lettres" << "\n" << "\n";
      wxMessageDialog dlg(this, msg, "Correction du tirage");
      dlg.ShowModal();
    }
    break;
  default: 
    INCOMPLETE; 
    break;
  }
 
  Game_getplayedrack(game,Game_getnrounds(game),r); 
  rack->SetValue(wxString(r));                      
  Game_removetestplay(game);                        
  results->DeleteAllItems();                        
  UpdateFrames();                                   
}

void
MainFrame::Search()
{
  int i;

  Game_search(game);

  // to speed up inserting we hide the control temporarily
  // but this is not good on slow machines as it shows an empty
  // square instead of the list
  //results->Hide();
  results->DeleteAllItems();
  results->SetFont(config.getFont(LISTFONT));
  for(i=0; i < Game_getnresults(game); i++)
    {
      wxChar word[WORD_SIZE_MAX];
      wxChar bonus;
      wxChar coord1[3], coord2[3], coord[6];
      wxChar pts[6];

      Game_getsearchedword       (game,i,word);
      Game_getsearchedfirstcoord (game,i,coord1);
      Game_getsearchedsecondcoord(game,i,coord2);
      sprintf(coord,"%2s%2s",coord1,coord2);
      bonus = Game_getsearchedbonus(game,i) ? '*' : ' ';
      sprintf(pts,"%3d",Game_getsearchedpoints(game,i));

      long tmp = results->InsertItem(i,word);
      results->SetItemData(tmp,i);
      tmp = results->SetItem(i,1,bonus);
      tmp = results->SetItem(i,2,coord);
      tmp = results->SetItem(i,3,pts);
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
      wxMessageBox("Vous devez choisir un dictionnaire","Eliot: erreur",wxICON_INFORMATION | wxOK);
      return;
    }

  Game_removetestplay(game);

  switch (Game_setrack_manual(game,config.getRackChecking(),(const char*)rack->GetValue()))
    {
    case 0x00: break;
    case 0x01:
      msg << "Le sac ne contient pas assez de lettres" << "\n"
	  << "pour assurer le tirage.";
      wxMessageBox(msg,"Correction du tirage", wxICON_INFORMATION | wxOK);
      return;
    case 0x02:
      msg << "Le tirage doit contenir au moins 2 consonnes et 2 voyelles" << "\n"; 
      wxMessageBox(msg,"Correction du tirage", wxICON_INFORMATION | wxOK); 
      return;
    default: statusbar->SetStatusText("Le tirage a été modifié manuellement",0); break;
    }

  Game_getplayedrack(game,Game_getnrounds(game),r);
  rack->SetValue(wxString(r));
  
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
    Game_play(game,n);

  Game_getplayedrack(game,Game_getnrounds(game),r); 
  rack->SetValue(wxString(r));                      
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
