%{

#include "regexp.h"
#include "regexp-er.h"

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

"*"        {return STAR;}
"+"        {return OR;}
"."        {return AND;}

"#"        {return SHARP;}
%%

