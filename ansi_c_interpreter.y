%{
#  include <stdio.h>
#  include <stdlib.h>
#  include <unistd.h>
#  include <string.h>
#  include "ansi_c_interpreter.h"
int yylex(void);
int yyparse(void);
extern char yytext[];
extern int column;
%}

%union {
    struct ast *a;
    struct value v;
    struct symbol *s;
    struct symbol_list *sl;
    int fn;
}

/* declare tokens */
%token <v> CONSTANT
%token <s> IDENTIFIER
%token <s> STRING_LITERAL
%token LE_OP GE_OP EQ_OP NE_OP

%token INT FLOAT DOUBLE VOID

%token IF ELSE WHILE RETURN

%nonassoc IFX
%nonassoc ELSE
%nonassoc UMINUS

%type <a> primary_expression postfix_expression unary_expression
%type <a> multiplicative_expression additive_expression relational_expression
%type <a> equality_expression assignment_expression expression
%type <a> declaration init_declarator statement compound_statement
%type <a> expression_statement selection_statement iteration_statement
%type <a> jump_statement translation_unit external_declaration
%type <a> block_item block_item_list argument_expression_list
%type <a> cast_expression
%type <a> control_body

%type <a> function_definition compound_statement_function
%type <sl> parameter_list parameter_list_opt
%type <s> parameter_declaration

%start translation_unit
%%

primary_expression
    : IDENTIFIER        { 
        debug_print("primary_expression", "Processing IDENTIFIER");
        if ($1 == NULL) {
            printf("ERROR: NULL identifier\n");
        } else {
            printf("DEBUG: Creating ref for identifier: %s\n", $1->name);
        }
        $$ = newref($1); 
        debug_print("primary_expression", "Created reference node");
    }
    | CONSTANT         { 
        debug_print("primary_expression", "Processing CONSTANT");
        printf("DEBUG: Creating number node with value: ");
        switch($1.type) {
            case TYPE_INT:
                printf("%d\n", $1.value.i_val);
                break;
            case TYPE_FLOAT:
                printf("%f\n", $1.value.f_val);
                break;
            case TYPE_DOUBLE:
                printf("%g\n", $1.value.d_val);
                break;
        }
        $$ = newnum($1.type, $1.value);
    }
    | STRING_LITERAL   { 
        debug_print("primary_expression", "Processing STRING_LITERAL (not implemented)");
        $$ = NULL; 
    }
    | '(' expression ')'    { 
        debug_print("primary_expression", "Processing parenthesized expression");
        $$ = $2; 
    }
    ;

argument_expression_list
    : assignment_expression    { $$ = $1; }
    | argument_expression_list ',' assignment_expression
        { $$ = newast('L', $1, $3); }
    ;

postfix_expression
    : primary_expression    { $$ = $1; }
    | IDENTIFIER '(' ')'   
        { 
            printf("DEBUG: Creating function call node for %s\n", $1->name);
            struct symbol *func = lookup_function($1->name);
            if (!func) {
                error("undefined function %s", $1->name);
                func = $1;
            }
            $$ = newcall(func, NULL); 
        }
    | IDENTIFIER '(' argument_expression_list ')'
        { 
            printf("DEBUG: Creating function call node for %s with arguments\n", $1->name);
            struct symbol *func = lookup_function($1->name);
            if (!func) {
                error("undefined function %s", $1->name);
                func = $1;
            }
            $$ = newcall(func, $3); 
        }
    ;

unary_expression
    : postfix_expression    { 
        debug_print("unary_expression", "Processing postfix expression");
        $$ = $1; 
    }
    | '-' cast_expression %prec UMINUS 
        { 
            debug_print("unary_expression", "Processing unary minus");
            printf("DEBUG: Creating unary minus node\n");
            $$ = newast('M', $2, NULL); 
        }
    ;

cast_expression
    : unary_expression    { $$ = $1; }
    | '(' type_specifier ')' cast_expression
        { $$ = $4; /* Handle type casting later */ }
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
            if ($1->nodetype == 'N')  // Must be an identifier reference
                $$ = newasgn(((struct symref *)$1)->s, $3);
            else
                yyerror("Invalid assignment target");
        }
    ;

expression
    : assignment_expression    { $$ = $1; }
    | expression ',' assignment_expression
        { $$ = newast('L', $1, $3); }
    ;

declaration
    : type_specifier init_declarator ';'    
        { $$ = $2; }
    ;

init_declarator
    : IDENTIFIER    
        { 
            debug_print("init_declarator", "Processing IDENTIFIER declaration");
            if ($1 == NULL) {
                printf("ERROR: NULL identifier in declaration\n");
            } else {
                printf("DEBUG: Setting type for identifier: %s to %s\n", 
                    $1->name, 
                    current_type == TYPE_INT ? "INT" :
                    current_type == TYPE_FLOAT ? "FLOAT" : "DOUBLE");
            }
            settype($1, current_type);
            printf("DEBUG: Creating declaration node\n");
            $$ = newdecl($1); 
            debug_print("init_declarator", "Created declaration node");
        }
    | IDENTIFIER '=' assignment_expression
        {
            debug_print("init_declarator", "Processing IDENTIFIER with initialization");
            if ($1 == NULL) {
                printf("ERROR: NULL identifier in declaration with init\n");
            } else {
                printf("DEBUG: Setting type for identifier with init: %s to %s\n", 
                    $1->name,
                    current_type == TYPE_INT ? "INT" :
                    current_type == TYPE_FLOAT ? "FLOAT" : "DOUBLE");
            }
            settype($1, current_type);
            printf("DEBUG: Creating declaration and assignment nodes\n");
            struct ast *decl = newdecl($1);
            struct ast *asgn = newasgn($1, $3);
            $$ = newast('L', decl, asgn);
            debug_print("init_declarator", "Created declaration with initialization");
        }
    ;


type_specifier
    : VOID    { current_type = TYPE_INT; /* Handle void type later */ }
    | INT     { current_type = TYPE_INT; }
    | FLOAT   { current_type = TYPE_FLOAT; }
    | DOUBLE  { current_type = TYPE_DOUBLE; }
    ;

statement
    : compound_statement     { $$ = $1; }
    | expression_statement   { $$ = $1; }
    | selection_statement    { $$ = $1; }
    | iteration_statement    { $$ = $1; }
    | jump_statement         { $$ = $1; }
    ;

compound_statement
    : '{' {push_scope();} '}'
        { 
            pop_scope();
            $$ = NULL; 
        }
    | '{' {push_scope();} block_item_list '}'
        { 
            pop_scope();
            $$ = $3; 
        }
    ;

block_item_list
    : block_item    { $$ = $1; }
    | block_item_list block_item
        { $$ = newast('L', $1, $2); }
    ;

block_item
    : declaration     { $$ = $1; }
    | statement       { $$ = $1; }
    ;

expression_statement
    : ';'             { current_type = NO_TYPE; $$ = NULL; }
    | expression ';'  { current_type = NO_TYPE; $$ = $1; }
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
        {
            $$ = newast('R', NULL, NULL);  /* Return with no value */
        }
    | RETURN expression ';'
        {
            $$ = newast('R', $2, NULL);    /* Return with value */
        }
    ;

translation_unit
    : external_declaration
        { 
            debug_print("translation_unit", "Processing single external declaration");
            root = $1;
            if (root) {
                printf("DEBUG: Evaluating AST\n");
                struct value result = eval(root);
                printf("DEBUG: Evaluation complete\n");
                switch(result.type) {
                    case TYPE_INT:
                        printf("= %d\n", result.value.i_val);
                        break;
                    case TYPE_FLOAT:
                        printf("= %f\n", result.value.f_val);
                        break;
                    case TYPE_DOUBLE:
                        printf("= %g\n", result.value.d_val);
                        break;
                }
                printf("DEBUG: Freeing AST\n");
                treefree(root);
                printf("DEBUG: AST freed\n");
            } else {
                printf("DEBUG: Null root AST\n");
            }
            printf("> ");
            fflush(stdout);
            if (isatty(0)) {
                fflush(stdin);
            }
        }
    | translation_unit external_declaration
        {
            debug_print("translation_unit", "Processing additional external declaration");
            root = $2;
            if (root) {
                printf("DEBUG: Evaluating AST\n");
                struct value result = eval(root);
                printf("DEBUG: Evaluation complete\n");
                switch(result.type) {
                    case TYPE_INT:
                        printf("= %d\n", result.value.i_val);
                        break;
                    case TYPE_FLOAT:
                        printf("= %f\n", result.value.f_val);
                        break;
                    case TYPE_DOUBLE:
                        printf("= %g\n", result.value.d_val);
                        break;
                }
                printf("DEBUG: Freeing AST\n");
                treefree(root);
                printf("DEBUG: AST freed\n");
            } else {
                printf("DEBUG: Null root AST\n");
            }
            printf("> ");
            fflush(stdout);
            if (isatty(0)) {
                fflush(stdin);
            }
        }
    ;

external_declaration
    : function_definition   { $$ = $1; }
    | declaration           { $$ = $1; }
    | statement             { $$ = $1; }
    ;

function_definition
    : type_specifier IDENTIFIER 
        {
            /* Ensure function is added to global scope */
            struct scope *save_scope = current_scope;
            while (current_scope->parent) current_scope = current_scope->parent;
            settype($2, current_type);
            current_scope = save_scope;  // Restore current scope
            push_scope(); /* Create scope for parameters and body */
        }
      '(' parameter_list_opt ')' compound_statement_function
        {
            debug_print("function_definition", "Processing function definition");
            struct scope *save_scope = current_scope;
            while (current_scope->parent) current_scope = current_scope->parent;
            dodef($2, $5, $7);
            current_scope = save_scope;
            pop_scope();  /* Pop the function's scope */
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
            debug_print("parameter_declaration", "Processing parameter");
            settype($2, current_type);
            $$ = $2;
        }
    ;

compound_statement_function
    : '{' block_item_list '}'
        {
            $$ = $2;
        }
    | '{' '}'
        {
            $$ = NULL;
        }
    ;

%%

void yyerror(char const *s)
{
    fflush(stdout);
    printf("\n%*s\n%*s\n", column, "^", column, s);
}