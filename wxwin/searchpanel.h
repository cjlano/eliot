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

// -*- C++ -*-

/**
 *  \file   searchpanel.h
 *  \brief  Panel used in Eliot search window
 *  \author Antoine Fraboulet
 *  \date   2005
 */

#ifndef _SEARCHPANEL_H
#define _SEARCHPANEL_H
#include "wx/notebook.h"

class SearchPanel : public wxNotebook
{
 public:
  SearchPanel(wxFrame*, const Dictionary&);
  ~SearchPanel();
};

#endif
