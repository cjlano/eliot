%{

#include "regexp.h"
#include "libdic_a-er.h"

/* %option bison-locations */
/* %option prefix="ER"     */

%}
%option 8bit reentrant bison-bridge
%option header-file="scanner.h"
%option nounput noyywrap

alphabet [a-zA-Z]
%%

{alphabet} {yylval_param->c = yytext[0]; return LEX_CHAR;}
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

"+"        {return LEX_PLUS;}
"*"        {return LEX_STAR;}

"#"        {return LEX_SHARP;}
%%

