%{
#  include <stdio.h>
#  include <stdlib.h>
#  include "ansi_c_interpreter.h"
int yylex(void);
int yyparse(void);
%}

%union {
    struct ast *a;
    struct value v;
    struct symbol *s;
    struct symlist *sl;
    int fn;
}

/* declare tokens */
%token <v> NUMBER
%token <s> NAME
%token <fn> FUNC
%token EOL

%token IF THEN ELSE WHILE DO LET
%token TYPE_INT_T TYPE_FLOAT_T TYPE_DOUBLE_T

%nonassoc <fn> CMP
%right '='
%left '+' '-'
%left '*' '/'
%nonassoc '|' UMINUS

%type <a> exp stmt list explist
%type <sl> symlist

%start calclist

%%

stmt: IF exp THEN list           { $$ = newflow('I', $2, $4, NULL); }
   | IF exp THEN list ELSE list  { $$ = newflow('I', $2, $4, $6); }
   | WHILE exp DO list           { $$ = newflow('W', $2, $4, NULL); }
   | TYPE_INT_T NAME             { 
       settype($2, TYPE_INT); 
       $$ = newdecl($2);
   }
   | TYPE_FLOAT_T NAME           { 
       settype($2, TYPE_FLOAT); 
       $$ = newdecl($2);
   }
   | TYPE_DOUBLE_T NAME          { 
       settype($2, TYPE_DOUBLE); 
       $$ = newdecl($2);
   }
   | TYPE_INT_T NAME '=' exp     { 
       settype($2, TYPE_INT); 
       $$ = newasgn($2, $4); 
   }
   | TYPE_FLOAT_T NAME '=' exp   { 
       settype($2, TYPE_FLOAT); 
       $$ = newasgn($2, $4); 
   }
   | TYPE_DOUBLE_T NAME '=' exp  { 
       settype($2, TYPE_DOUBLE); 
       $$ = newasgn($2, $4); 
   }
   | exp                         { $$ = $1; }
;

list: /* nothing */ { $$ = NULL; }
   | stmt ';' list { 
       if ($3 == NULL)
           $$ = $1;
       else
           $$ = newast('L', $1, $3);
   }
;

exp: exp CMP exp          { $$ = newcmp($2, $1, $3); }
   | exp '+' exp          { $$ = newast('+', $1, $3); }
   | exp '-' exp          { $$ = newast('-', $1, $3); }
   | exp '*' exp          { $$ = newast('*', $1, $3); }
   | exp '/' exp          { $$ = newast('/', $1, $3); }
   | '|' exp              { $$ = newast('|', $2, NULL); }
   | '(' exp ')'          { $$ = $2; }
   | '-' exp %prec UMINUS { $$ = newast('M', $2, NULL); }
   | NUMBER               { $$ = newnum($1.type, $1.value); }
   | NAME                 { $$ = newref($1); }
   | NAME '=' exp         { $$ = newasgn($1, $3); }
   | FUNC '(' explist ')' { $$ = newfunc($1, $3); }
   | NAME '(' explist ')' { $$ = newcall($1, $3); }
;

explist: exp
   | exp ',' explist  { $$ = newast('L', $1, $3); }
;

symlist: NAME       { $$ = newsymlist($1, NULL); }
   | NAME ',' symlist { $$ = newsymlist($1, $3); }
;

calclist: /* nothing */
   | calclist stmt EOL {
       if(debug) dumpast($2, 0);
       struct value v = eval($2);
       switch(v.type) {
           case TYPE_INT:
               printf("= %d\n> ", v.value.i_val);
               break;
           case TYPE_FLOAT:
               printf("= %f\n> ", v.value.f_val);
               break;
           case TYPE_DOUBLE:
               printf("= %g\n> ", v.value.d_val);
               break;
       }
       treefree($2);
   }
   | calclist LET NAME '(' symlist ')' '=' list EOL {
       dodef($3, $5, $8);
       printf("Defined %s\n> ", $3->name);
   }
   | calclist error EOL { yyerrok; printf("> "); }
;
%%