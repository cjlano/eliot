%{
/* Eliot                                                                     */
/* Copyright (C) 2005  Antoine Fraboulet                                     */
/*                                                                           */
/* This file is part of Eliot.                                               */
/*                                                                           */
/* Eliot is free software; you can redistribute it and/or modify             */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 2 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* Elit is distributed in the hope that it will be useful,                   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software               */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */

#include "dic.h"
#include "regexp.h"
#include "libdic_a-er.h"

#define MASK_TO_REMOVE    0x1F

%}
%option prefix="regexp"
%option outfile="lex.yy.c"
%option header-file="scanner.h"
%option reentrant bison-bridge
%option bison-locations
%option noyywrap nounput

/* TODO : remove lexer translation */
alphabet [a-zA-Z]
%%

{alphabet} {yylval_param->c=(yytext[0]&MASK_TO_REMOVE); return LEX_CHAR;}
"["        {return LEX_L_SQBRACKET;}
"]"        {return LEX_R_SQBRACKET;}
"("        {return LEX_L_BRACKET;}
")"        {return LEX_R_BRACKET;}
"^"        {return LEX_HAT;}

"."        {return LEX_ALL;}
":v:"      {return LEX_VOWL;}
":c:"      {return LEX_CONS;}
":1:"      {return LEX_USER1;}
":2:"      {return LEX_USER2;}

"?"        {return LEX_QMARK;}
"+"        {return LEX_PLUS;}
"*"        {return LEX_STAR;}

"#"        {return LEX_SHARP;}
%%

