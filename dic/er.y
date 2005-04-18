%{
#include <stdio.h>
#include <malloc.h>  
#include <stdlib.h> 
#include "regexp.h"
#include "regexp-er.h"
#include "scanner.h"

void yyerror (yyscan_t scanner, NODE** root, char const *msg);

%}   
%union {
  char c;
  NODE *NODE_TYPE;
};

%defines
%pure-parser
%parse-param {yyscan_t yyscanner}
%parse-param {NODE **root}
%lex-param   {yyscan_t yyscanner}
/* %locations        */
/* %name-prefix="ER" */

/* Token declaration */
%token  <c>  CHAR
%token  L_SQBRACKET R_SQBRACKET 
%token  L_BRACKET R_BRACKET 
%token  HAT 
%token  NIMP
%token  CONS
%token  VOYL
%token  PLUS
%token  STAR
%token  SHARP
%type   <NODE_TYPE> var
%type   <NODE_TYPE> expr
%type   <NODE_TYPE> exprdis
%start  start        
%%

start: L_BRACKET expr R_BRACKET SHARP
     { 
       NODE *sharp;
       NODE *er;
       sharp = regexp_createNODE(NODE_VAR,'#',NULL,NULL);
       er = $2;
       *root = regexp_createNODE(NODE_AND,'\0',er,sharp);
       YYACCEPT; 
     }
     ;

expr : var                      
       {
	 $$=$1;
       }  
     | expr expr                
       {
	 $$=regexp_createNODE(NODE_AND,'\0',$1,$2);
       }
     | var STAR                 
       {
	 $$=regexp_createNODE(NODE_STAR,'\0',$1,NULL);
       }
     | L_BRACKET expr R_BRACKET 
       {
	 $$=$2;
       }
     | L_BRACKET expr R_BRACKET STAR    
       {
	 $$=regexp_createNODE(NODE_STAR,'\0',$2,NULL);
       }
     | L_SQBRACKET exprdis R_SQBRACKET
       {
	 $$=$2;
       }
     | L_SQBRACKET exprdis R_SQBRACKET STAR
       {
	 $$=regexp_createNODE(NODE_STAR,'\0',$2,NULL);
       }
     ;    


exprdis: var                      
       {
	 $$=$1;
       }  
     | exprdis exprdis
       {
	 $$=regexp_createNODE(NODE_OR,'\0',$1,$2);
       }
     ;



var : CHAR
       {
         $$=regexp_createNODE(NODE_VAR,$1,NULL,NULL);
       }  
     | NIMP
       {
         $$=regexp_createNODE_AllMatch();
       }
     | VOYL
       {
         $$=regexp_createNODE_VoylMatch();
       }
     | CONS
       {
         $$=regexp_createNODE_ConsMatch();
       }
     ;

%%


