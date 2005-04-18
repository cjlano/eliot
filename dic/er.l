%{

#include "regexp.h"
#include "regexp-er.h"

/* %option bison-locations */
/* %option prefix="ER"     */

%}
%option 8bit reentrant bison-bridge
%option header-file="scanner.h"
%option nounput noyywrap

alphabet [a-zA-Z]
%%

{alphabet} {yylval_param->c = yytext[0]; return CHAR;}
"["        {return L_SQBRACKET;}
"]"        {return R_SQBRACKET;}
"("        {return L_BRACKET;}
")"        {return R_BRACKET;}
"^"        {return HAT;}

"."        {return NIMP;}
":c:"      {return CONS;}
":v:"      {return VOYL;}

"+"        {return PLUS;}
"*"        {return STAR;}

"#"        {return SHARP;}
%%

