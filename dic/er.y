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

/* Pure yylex.  */
%pure-parser
%parse-param {yyscan_t yyscanner}
%parse-param {NODE **root}
%lex-param   {yyscan_t yyscanner}

/* Token declaration */
%token  SHARP
%token  L_BRACKET R_BRACKET 
%token  L_SQBRACKET R_SQBRACKET 
%token  HAT 
%token  <c>  CHAR
%left   AND
%left   OR
%left   STAR
%type   <NODE_TYPE> var
%type   <NODE_TYPE> expr
%type   <NODE_TYPE> exprdis
%start  start        
%%

start: L_BRACKET expr R_BRACKET AND SHARP
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
     | expr AND expr            
       {
	 $$=regexp_createNODE(NODE_AND,'\0',$1,$3);
       }
     | expr OR expr             
       {
	 $$=regexp_createNODE(NODE_OR,'\0',$1,$3);
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
     ;

%%
/*--------------------------------------------------------         
 *
 *--------------------------------------------------------*/

void yyerror (yyscan_t yyscanner, NODE** root, char const *msg)
{
  printf("\n erreur ! (%s)\n",msg);
}



