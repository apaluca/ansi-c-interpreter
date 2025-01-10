%{
/* ========================================================================== */
/*                              Includes and Declarations                       */
/* ========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "ansi_c_interpreter.h"

extern char yytext[];
extern int column;
%}

/* ========================================================================== */
/*                              Union Declaration                               */
/* ========================================================================== */

%union {
    struct ast *a;
    struct value v;
    struct symbol *s;
    struct symbol_list *sl;
    int fn;
}

/* ========================================================================== */
/*                              Token Declarations                              */
/* ========================================================================== */

/* Terminals with values */
%token <v> CONSTANT
%token <s> IDENTIFIER
%token <a> STRING_LITERAL
%token <fn> BUILTIN

/* Operators */
%token LE_OP GE_OP EQ_OP NE_OP

/* Keywords */
%token INT FLOAT DOUBLE
%token IF ELSE WHILE RETURN
%token RUN

/* Operator precedence */
%nonassoc IFX
%nonassoc ELSE
%nonassoc UMINUS

/* ========================================================================== */
/*                              Type Declarations                               */
/* ========================================================================== */

/* Expressions */
%type <a> primary_expression postfix_expression unary_expression
%type <a> multiplicative_expression additive_expression relational_expression
%type <a> equality_expression assignment_expression expression
%type <a> cast_expression

/* Statements */
%type <a> declaration init_declarator statement compound_statement
%type <a> expression_statement selection_statement iteration_statement
%type <a> jump_statement control_body

/* Program structure */
%type <a> translation_unit external_declaration
%type <a> block_item block_item_list argument_expression_list

/* Functions */
%type <a> function_definition compound_statement_function
%type <sl> parameter_list parameter_list_opt
%type <s> parameter_declaration

/* Starting symbol */
%start translation_unit

%%
/* ========================================================================== */
/*                              Grammar Rules                                   */
/* ========================================================================== */

/* --------------- Expression Rules --------------- */

primary_expression
    : IDENTIFIER            { $$ = newref($1); }
    | CONSTANT             { $$ = newnum($1.type, $1.value); }
    | STRING_LITERAL       { $$ = NULL; }
    | '(' expression ')'   { $$ = $2; }
    ;

argument_expression_list
    : assignment_expression    
        { $$ = $1; }
    | argument_expression_list ',' assignment_expression
        { $$ = newast('L', $1, $3); }
    ;

postfix_expression
    : primary_expression 
        { $$ = $1; }
    | IDENTIFIER '(' ')'
        {
            struct symbol *func = lookup_function($1->name);
            if (!func) {
                error("undefined function %s", $1->name);
                func = $1;
            }
            $$ = newcall(func, NULL);
        }
    | IDENTIFIER '(' argument_expression_list ')'
        {
            struct symbol *func = lookup_function($1->name);
            if (!func) {
                error("undefined function %s", $1->name);
                func = $1;
            }
            $$ = newcall(func, $3);
        }
    | BUILTIN '(' STRING_LITERAL ',' argument_expression_list ')'
        {
            if ($1 == B_scanf) {
                if ($5->nodetype != 'N') {
                    error("scanf requires a variable reference");
                    $$ = NULL;
                } else {
                    struct ast *args = newast('L', $3, $5);
                    $$ = newfunc($1, args);
                }
            } else {
                struct ast *args = newast('L', $3, $5);
                $$ = newfunc($1, args);
            }
        }
    | BUILTIN '(' STRING_LITERAL ')'
        {
            if ($1 == B_scanf) {
                error("scanf requires a variable argument");
                $$ = NULL;
            } else {
                $$ = newfunc($1, $3);
            }
        }
    ;

unary_expression
    : postfix_expression           { $$ = $1; }
    | '-' cast_expression %prec UMINUS 
        { $$ = newast('M', $2, NULL); }
    ;

cast_expression
    : unary_expression            { $$ = $1; }
    | '(' type_specifier ')' cast_expression
        { $$ = newcast(current_type, $4); }
    ;

multiplicative_expression
    : cast_expression    { $$ = $1; }
    | multiplicative_expression '*' cast_expression
        { $$ = newast('*', $1, $3); }
    | multiplicative_expression '/' cast_expression
        { $$ = newast('/', $1, $3); }
    | multiplicative_expression '%' cast_expression
        { $$ = newast('%', $1, $3); }
    ;

additive_expression
    : multiplicative_expression    { $$ = $1; }
    | additive_expression '+' multiplicative_expression
        { $$ = newast('+', $1, $3); }
    | additive_expression '-' multiplicative_expression
        { $$ = newast('-', $1, $3); }
    ;

relational_expression
    : additive_expression    { $$ = $1; }
    | relational_expression '<' additive_expression
        { $$ = newcmp(2, $1, $3); }
    | relational_expression '>' additive_expression
        { $$ = newcmp(1, $1, $3); }
    | relational_expression LE_OP additive_expression
        { $$ = newcmp(6, $1, $3); }
    | relational_expression GE_OP additive_expression
        { $$ = newcmp(5, $1, $3); }
    ;

equality_expression
    : relational_expression    { $$ = $1; }
    | equality_expression EQ_OP relational_expression
        { $$ = newcmp(4, $1, $3); }
    | equality_expression NE_OP relational_expression
        { $$ = newcmp(3, $1, $3); }
    ;

assignment_expression
    : equality_expression    { $$ = $1; }
    | primary_expression '=' assignment_expression
        { 
            if ($1->nodetype == 'N') {  // Must be an identifier reference
                struct symref *ref = (struct symref *)$1;
                
                // Look up the symbol in all accessible scopes
                struct symbol *sym = lookup_all_scopes(ref->s->name);
                if (sym) {
                    $$ = newasgn(sym, $3);
                } else {
                    error("assignment to undeclared variable '%s'", ref->s->name);
                }
            } else {
                yyerror("Invalid assignment target");
            }
        }
    ;

expression
    : assignment_expression    { $$ = $1; }
    | expression ',' assignment_expression
        { $$ = newast('L', $1, $3); }
    ;

/* --------------- Declaration Rules --------------- */

declaration
    : type_specifier init_declarator ';'    
        { $$ = $2; }
    ;

init_declarator
    : IDENTIFIER    
        { 
            $1->type = current_type;
            $$ = newdecl($1); 
        }
    ;

type_specifier
    : INT     { current_type = TYPE_INT; }
    | FLOAT   { current_type = TYPE_FLOAT; }
    | DOUBLE  { current_type = TYPE_DOUBLE; }
    ;

/* --------------- Statement Rules --------------- */

statement
    : compound_statement     { $$ = $1; }
    | expression_statement   { $$ = $1; }
    | selection_statement    { $$ = $1; }
    | iteration_statement    { $$ = $1; }
    | jump_statement         { $$ = $1; }
    ;

compound_statement
    : '{' {
        MEM_DEBUG("Starting new compound statement");
        struct scope *new_scope = push_scope();
        MEM_DEBUG("Created new scope %p for compound statement", (void*)new_scope);
    } block_item_list '}'
    {
        MEM_DEBUG("Creating block for compound statement");
        struct ast *block = newblock($3, current_scope);
        $$ = block;
    }
    | '{' {
        MEM_DEBUG("Starting empty compound statement");
        struct scope *new_scope = push_scope();
        MEM_DEBUG("Created new scope %p for empty compound statement", (void*)new_scope);
    } '}'
    {
        MEM_DEBUG("Creating empty block");
        $$ = newblock(NULL, current_scope);
    }
    ;

block_item_list
    : block_item    
        { $$ = $1; }
    | block_item_list block_item
        { 
            if ($1 && $2) {
                $$ = newast('L', $1, $2);
            } else if ($1) {
                $$ = $1;
            } else {
                $$ = $2;
            }
        }
    ;

block_item
    : declaration     { $$ = $1; }
    | statement       { $$ = $1; }
    ;

expression_statement
    : ';'                   
        { 
            current_type = NO_TYPE; 
            $$ = NULL; 
        }
    | expression ';'        
        { 
            current_type = NO_TYPE; 
            $$ = $1; 
        }
    ;

control_body
    : statement  { $$ = $1; }
    ;

selection_statement
    : IF '(' expression ')' control_body %prec IFX
        { $$ = newflow('I', $3, $5, NULL); }
    | IF '(' expression ')' control_body ELSE control_body
        { $$ = newflow('I', $3, $5, $7); }
    ;

iteration_statement
    : WHILE '(' expression ')' control_body
        { $$ = newflow('W', $3, $5, NULL); }
    ;

jump_statement
    : RETURN ';'
        { $$ = newast('R', NULL, NULL); }
    | RETURN expression ';'
        { $$ = newast('R', $2, NULL); }
    ;

/* --------------- Program Structure Rules --------------- */

translation_unit
    : external_declaration
        { 
            if ($1) {
                DEBUG_PRINT_AST($1, 0);
                if (!error_state) {
                    struct value result = eval($1);
                    treefree($1);
                }
            }
            if (interactive_mode) {
                printf("> ");
                fflush(stdout);
            }
        }
    | translation_unit external_declaration
        {
            if ($2) {
                DEBUG_PRINT_AST($2, 0);
                if (!error_state) {
                    struct value result = eval($2);
                    treefree($2);
                }
            }
            if (interactive_mode) {
                printf("> ");
                fflush(stdout);
            }
        }
    | RUN STRING_LITERAL ';'
        {
            if (!error_state) {
                struct strast *s = (struct strast *)$2;
                if (run_script(s->string) != 0) {
                    error("Failed to run script: %s", s->string);
                }
                treefree($2);
            }
            if (interactive_mode) {
                printf("> ");
                fflush(stdout);
            }
        }
    | translation_unit RUN STRING_LITERAL ';'
        {
            if (!error_state) {
                struct strast *s = (struct strast *)$3;
                if (run_script(s->string) != 0) {
                    error("Failed to run script: %s", s->string);
                }
                treefree($3);
            }
            if (interactive_mode) {
                printf("> ");
                fflush(stdout);
            }
        }
    ;

external_declaration
    : function_definition   { $$ = $1; }
    | declaration           { $$ = $1; }
    | statement             { $$ = $1; }
    ;

/* --------------- Function Rules --------------- */

function_definition
    : type_specifier IDENTIFIER 
        {
            struct scope *save_scope = current_scope;
            while (current_scope->parent) current_scope = current_scope->parent;
            settype($2, current_type);

            struct symbol_table *st = malloc(sizeof(struct symbol_table));
            if (!st) {
                error("out of space");
                exit(0);
            }
            st->sym = $2;
            st->next = current_scope->symbols;
            current_scope->symbols = st;

            current_scope = save_scope;
            push_scope();
        }
      '(' parameter_list_opt ')' compound_statement_function
        {
            struct scope *save_scope = current_scope;
            while (current_scope->parent) current_scope = current_scope->parent;
            dodef($2, $5, $7);
            current_scope = save_scope;
            pop_scope();
            $$ = NULL;
        }
    ;

parameter_list_opt
    : /* empty */     { $$ = NULL; }
    | parameter_list  { $$ = $1; }
    ;

parameter_list
    : parameter_declaration
        { $$ = newsymlist($1, NULL); }
    | parameter_list ',' parameter_declaration
        { $$ = newsymlist($3, $1); }
    ;

parameter_declaration
    : type_specifier IDENTIFIER
        {
            settype($2, current_type);
            
            /* Add parameter to current scope */
            struct symbol_table *st = malloc(sizeof(struct symbol_table));
            if (!st) {
                error("out of space");
                exit(0);
            }
            st->sym = $2;
            st->next = current_scope->symbols;
            current_scope->symbols = st;
            
            $$ = $2;
        }
    ;

compound_statement_function
    : '{' block_item_list '}'
        { $$ = $2; }
    | '{' '}'
        { $$ = NULL; }
    ;

%%

/* ========================================================================== */
/*                              Error Handling                                  */
/* ========================================================================== */

void yyerror(char const *s) 
{
    fflush(stdout);
    printf("\n%*s\n%*s\n", column, "^", column, s);
    cleanup_and_exit(1);
}
