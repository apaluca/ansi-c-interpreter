#ifndef _ANSI_C_INTERPRETER_H_
#define _ANSI_C_INTERPRETER_H_

int yyparse(void);
int yylex(void);

/* Type system */
enum value_type
{
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_DOUBLE
};

/* Format string for built-in functions scanf and printf */
struct format_string
{
    char *str;
};

/* Value union to store different types */
union value_union
{
    int i_val;
    float f_val;
    double d_val;
};

/* Structure to hold a value and its type */
struct value
{
    enum value_type type;
    union value_union value;
};

/* symbol table */
struct symbol
{
    char *name;
    enum value_type type;
    union value_union value;
    struct ast *func;         /* stmt for the function */
    struct symbol_list *syms; /* list of dummy args */
};

struct symbol *lookup(char *);

/* list of symbols, for an argument list */
struct symbol_list
{
    struct symbol *sym;
    struct symbol_list *next;
};

struct symbol_list *newsymlist(struct symbol *sym, struct symbol_list *next);
void symlistfree(struct symbol_list *sl);

/* Scope management */
struct symbol_table
{
    struct symbol *sym;        // Symbol entry
    struct symbol_table *next; // Next symbol in this scope
};

struct scope
{
    struct symbol_table *symbols; // Symbols in current scope
    struct scope *parent;         // Parent scope
};

extern struct scope *current_scope;

/* Function context structure */
struct function_context
{
    struct symbol *function; /* Current function being executed */
    struct value return_value;
};

/* Stack of function contexts to handle nested calls */
#define MAX_FUNCTION_DEPTH 256
extern struct function_context function_stack[];
extern int function_depth;

/* node types
 * + - * / |
 * 0-7 comparison ops, bit coded 04 equal, 02 less, 01 greater
 * M unary minus
 * L statement list
 * I IF statement
 * W WHILE statement
 * D declaration without initialization
 * N symbol ref
 * = assignment
 * S list of symbols
 * F built in function call
 * C user function call
 * R return statement
 */

enum bifs
{ /* built-in functions */
  B_printf,
  B_scanf
};

/* nodes in the Abstract Syntax Tree */
/* all have common initial nodetype */
struct ast
{
    int nodetype;
    enum value_type result_type; /* type of the expression result */
    struct ast *l;
    struct ast *r;
};

struct typecast
{
    int nodetype;         /* type T for typecast */
    enum value_type type; /* target type */
    struct ast *operand;  /* expression to cast */
    enum value_type result_type;
};

struct strval
{
    int nodetype; // type 'S' for string
    char *str;
};

struct fncall
{                 /* built-in function */
    int nodetype; /* type F */
    struct ast *l;
    enum bifs functype;
    enum value_type result_type;
};

struct ufncall
{                  /* user function */
    int nodetype;  /* type C */
    struct ast *l; /* list of arguments */
    struct symbol *s;
    enum value_type result_type;
};

struct flow
{
    int nodetype;     /* type I or W */
    struct ast *cond; /* condition */
    struct ast *tl;   /* then or do list */
    struct ast *el;   /* optional else list */
    enum value_type result_type;
};

struct numval
{
    int nodetype; /* type K */
    enum value_type type;
    union value_union value;
};

struct symref
{
    int nodetype; /* type N */
    struct symbol *s;
    enum value_type result_type;
};

struct symasgn
{
    int nodetype; /* type = */
    struct symbol *s;
    struct ast *v; /* value */
    enum value_type result_type;
};

/* Function management functions */
char *process_string_literal(char *str);
void push_function(struct symbol *func);
struct function_context *pop_function(void);
struct function_context *current_function(void);
struct value eval_function_body(struct ast *body, struct symbol *func);
struct symbol *lookup_function(char *name);

/* Scope management functions */
struct scope *push_scope(void);
void pop_scope(void);
struct symbol *scope_lookup(char *name);      // Look up symbol in current scope only
struct symbol *lookup_all_scopes(char *name); // Look up symbol in all accessible scopes

/* Type system functions */
struct ast *newdecl(struct symbol *s);
void settype(struct symbol *sym, enum value_type type);
enum value_type promote_types(enum value_type t1, enum value_type t2);
void convert_value(void *dest, enum value_type dest_type,
                   void *src, enum value_type src_type);

/* build an AST */
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newcast(enum value_type type, struct ast *operand);
struct ast *newstring(char *s);
struct ast *newcmp(int cmptype, struct ast *l, struct ast *r);
struct ast *newfunc(int functype, struct ast *l);
struct ast *newcall(struct symbol *s, struct ast *l);
struct ast *newref(struct symbol *s);
struct ast *newasgn(struct symbol *s, struct ast *v);
struct ast *newnum(enum value_type type, union value_union value);
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *tr);

/* define a function */
void dodef(struct symbol *name, struct symbol_list *syms, struct ast *stmts);

/* evaluate an AST */
struct value eval(struct ast *);

/* delete and free an AST */
void treefree(struct ast *);

/* interface to the lexer */
extern int yylineno; /* from lexer */
void yyerror(const char *s);

/* error handling regarding the internals of the parser */
void error(char *s, ...);
void cleanup_and_exit(int status);

extern struct ast *root;
#define NO_TYPE -1
extern enum value_type current_type;

extern int debug;
void dumpast(struct ast *a, int level);

void debug_print(const char *rule, const char *msg);

#endif