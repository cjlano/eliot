%{

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
%option noyywrap

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

