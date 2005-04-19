%{
#include <stdio.h>
#include <malloc.h>  
#include <stdlib.h> 

#include "dic.h"
#include "dic_internals.h" 

#include "regexp.h"
#include "libdic_a-er.h"
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

%token  <c>  LEX_CHAR
%token  LEX_L_SQBRACKET LEX_R_SQBRACKET 
%token  LEX_L_BRACKET LEX_R_BRACKET 
%token  LEX_HAT 

%token  LEX_ALL
%token  LEX_VOWL
%token  LEX_CONS
%token  LEX_USER1
%token  LEX_USER2

%token  LEX_PLUS
%token  LEX_STAR
%token  LEX_SHARP

%type   <NODE_TYPE> var
%type   <NODE_TYPE> expr
%type   <NODE_TYPE> exprdis
%start  start        
%%

start: LEX_L_BRACKET expr LEX_R_BRACKET LEX_SHARP
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
     | var LEX_STAR                 
       {
	 $$=regexp_createNODE(NODE_STAR,'\0',$1,NULL);
       }
     | LEX_L_BRACKET expr LEX_R_BRACKET 
       {
	 $$=$2;
       }
     | LEX_L_BRACKET expr LEX_R_BRACKET LEX_STAR    
       {
	 $$=regexp_createNODE(NODE_STAR,'\0',$2,NULL);
       }
     | LEX_L_SQBRACKET exprdis LEX_R_SQBRACKET
       {
	 $$=$2;
       }
     | LEX_L_SQBRACKET exprdis LEX_R_SQBRACKET LEX_STAR
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



var : LEX_CHAR
       {
         $$=regexp_createNODE(NODE_VAR,$1 & DIC_CHAR_MASK,NULL,NULL);
       }  
     | LEX_ALL
       {
         $$=regexp_createNODE(NODE_VAR,RE_ALL_MATCH,NULL,NULL);
       }
     | LEX_VOWL
       {
         $$=regexp_createNODE(NODE_VAR,RE_VOWL_MATCH,NULL,NULL);
       }
     | LEX_CONS
       {
         $$=regexp_createNODE(NODE_VAR,RE_CONS_MATCH,NULL,NULL);
       }
     | LEX_USER1
       {
         $$=regexp_createNODE(NODE_VAR,RE_USR1_MATCH,NULL,NULL);
       }
     | LEX_USER2
       {
         $$=regexp_createNODE(NODE_VAR,RE_USR2_MATCH,NULL,NULL);
       }
     ;

%%


