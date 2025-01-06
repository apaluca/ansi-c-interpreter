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
    struct ast *func;     /* stmt for the function */
    struct symlist *syms; /* list of dummy args */
};

/* simple symtab of fixed size */
#define NHASH 9997
extern struct symbol symtab[];

struct symbol *lookup(char *);

/* list of symbols, for an argument list */
struct symlist
{
    struct symbol *sym;
    struct symlist *next;
};

struct symlist *newsymlist(struct symbol *sym, struct symlist *next);
void symlistfree(struct symlist *sl);

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
 */

enum bifs
{ /* built-in functions */
  B_sqrt = 1,
  B_exp,
  B_log,
  B_print
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

/* Type system functions */
struct ast *newdecl(struct symbol *s);
void settype(struct symbol *sym, enum value_type type);
enum value_type promote_types(enum value_type t1, enum value_type t2);
void convert_value(void *dest, enum value_type dest_type,
                   void *src, enum value_type src_type);

/* build an AST */
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newcmp(int cmptype, struct ast *l, struct ast *r);
struct ast *newfunc(int functype, struct ast *l);
struct ast *newcall(struct symbol *s, struct ast *l);
struct ast *newref(struct symbol *s);
struct ast *newasgn(struct symbol *s, struct ast *v);
struct ast *newnum(enum value_type type, union value_union value);
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *tr);

/* define a function */
void dodef(struct symbol *name, struct symlist *syms, struct ast *stmts);

/* evaluate an AST */
struct value eval(struct ast *);

/* delete and free an AST */
void treefree(struct ast *);

/* interface to the lexer */
extern int yylineno; /* from lexer */
void yyerror(const char *s);

/* error handling regarding the internals of the parser */
void error(char *s, ...);

extern int debug;
void dumpast(struct ast *a, int level);

void debug_print(const char *rule, const char *msg);

#endif