%{
#include <stdio.h>
#include <malloc.h>  
#include <stdlib.h> 
#include "regexp.h"

int yylex();
void yyerror(char*);

NODE *createNODE(int type, char v, NODE *fg, NODE *fd);

%}   
%union {
  char variable;
  NODE *NODE_TYPE;
};

%token  DIESE PAR_G PAR_D FIN
%token  <variable>  VARIABLE
%left   OR
%left   AND
%left   STAR
%type   <NODE_TYPE> var
%type   <NODE_TYPE> expr
%start  start        
%%

/* start : ex ex */
/*        { root=createNODE(NODE_TOP,'\0',$1,$2); YYACCEPT; } */
/*      ; */

start: expr FIN 
     { root = createNODE(NODE_AND,'\0',$1,createNODE(NODE_VAR,'#',NULL,NULL)); YYACCEPT; }
     ;

expr : var                   {$$=$1;}  
     | expr expr             {$$=createNODE(NODE_AND,'\0',$1,$2);}
     | expr AND expr         {$$=createNODE(NODE_AND,'\0',$1,$3);}
     | PAR_G expr PAR_D      {$$=$2;}
     | expr OR expr          {$$=createNODE(NODE_OR,'\0',$1,$3);}
     | var STAR              {$$=createNODE(NODE_STAR,'\0',$1,NULL);}
     | PAR_G expr PAR_D STAR {$$=createNODE(NODE_STAR,'\0',$2,NULL);}
     ;    

var : VARIABLE
    {$$=createNODE(NODE_VAR,$1,NULL,NULL);}  
    ;

%%
/*--------------------------------------------------------         
 *                FONCTIONS LEX/YACC
 *--------------------------------------------------------*/

void yyerror(char *s)
{
  printf("\n erreur ! (%s)\n",s);
  exit(1);
}

/*--------------------------------------------------------         
 *
 *--------------------------------------------------------*/
NODE *createNODE(int type,char v,NODE *fg,NODE *fd)
{
  NODE *x;
  x=(NODE *)malloc(sizeof(NODE));

  x->type      = type;
  x->var       = v;
  x->fd        = fd;
  x->fg        = fg;

  x->numero    = 0;
  x->position  = 0;
  x->annulable = 0;
  x->PP        = 0;
  x->DP        = 0;
  return (x);
}


