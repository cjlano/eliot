%{

#include "regexp.h"
#include "regexp-er.h"

#define YY_NO_UNPUT

%}
%option noyywrap

alphabet [a-zA-Z]
%%

{alphabet} {yylval.variable = yytext[0]; return VARIABLE;}
"("        {return PAR_G;}
")"        {return PAR_D;}
"#"        {return DIESE;}
"*"        {return STAR;}
"+"        {return OR;}
"."        {return AND;}

"\n"       {return FIN;}
%%

