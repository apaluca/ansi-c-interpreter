#ifndef _ANSI_C_INTERPRETER_H_
#define _ANSI_C_INTERPRETER_H_

#include <setjmp.h>

/* ========================================================================== */
/*                              Parser Interface                              */
/* ========================================================================== */

int yyparse(void);
int yylex(void);
void yyrestart(FILE *input_file);
extern int yylineno;
extern FILE *yyin;
void yyerror(const char *s);

/* ========================================================================== */
/*                              Type System                                   */
/* ========================================================================== */

enum value_type
{
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_DOUBLE
};

#define NO_TYPE -1
extern enum value_type current_type;

union value_union
{
    int i_val;
    float f_val;
    double d_val;
};

struct value
{
    enum value_type type;
    union value_union value;
};

/* ========================================================================== */
/*                              Symbol System                                 */
/* ========================================================================== */

/* Symbol table */
struct symbol
{
    char *name;
    enum value_type type;
    union value_union value;
    struct ast *func;         /* stmt for the function */
    struct symbol_list *syms; /* list of dummy args */
};

/* List of symbols, for an argument list */
struct symbol_list
{
    struct symbol *sym;
    struct symbol_list *next;
};

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

/* ========================================================================== */
/*                              AST Node Types                                */
/* ========================================================================== */

/* node types
 * + - * / |
 * 0-7 comparison ops, bit coded 04 equal, 02 less, 01 greater
 * M unary minus
 * B block evaluation
 * L statement list
 * I IF statement
 * W WHILE statement
 * D declaration without initialization
 * N symbol ref
 * = assignment
 * F built in function call
 * C user function call
 * R return statement
 * S string for built-in functions
 */

/* Base AST node */
struct ast
{
    int nodetype;
    enum value_type result_type; /* type of the expression result */
    struct ast *l;
    struct ast *r;
};

/* Specialized node types */
struct typecast
{
    int nodetype;         /* type T for typecast */
    enum value_type type; /* target type */
    struct ast *operand;  /* expression to cast */
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

struct strast
{
    int nodetype; /* type 'S' */
    char *string;
    struct ast *next; /* for argument lists */
    enum value_type result_type;
};

/* ========================================================================== */
/*                              Function System                               */
/* ========================================================================== */

/* Built-in functions */
enum bifs
{
    B_printf,
    B_scanf
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

struct block
{
    int nodetype;              /* type 'B' for block */
    struct ast *statements;    /* Statements in the block */
    struct scope *block_scope; /* Original scope during definition */
    int needs_scope;           /* Flag indicating if this block needs a new scope */
    enum value_type result_type;
};

/* Function context handling */
struct function_context
{
    struct symbol *function;    /* Current function being executed */
    struct scope *caller_scope; /* Scope where function was called from */
    struct value return_value;  /* Return value from function */
    int has_returned;           /* Flag to indicate if function has returned */
};

/* Structure to rack return state with */
struct return_state
{
    int has_returned;
    struct value return_value;
};

#define MAX_FUNCTION_DEPTH 256
extern struct function_context function_stack[];
extern int function_depth;

/* ========================================================================== */
/*                              Global Variables                              */
/* ========================================================================== */

extern struct scope *current_scope;
extern struct ast *root;
extern int interactive_mode;
extern int error_state;
extern jmp_buf error_jmp;

/* ========================================================================== */
/*                              Function Prototypes                           */
/* ========================================================================== */

/* Symbol management */
void init_symbol(struct symbol *sym);
struct symbol *lookup(char *);
struct symbol *lookup_function(char *name);
struct symbol_list *newsymlist(struct symbol *sym, struct symbol_list *next);
void symlistfree(struct symbol_list *sl);

/* Scope management */
struct scope *push_scope(void);
void pop_scope(void);
struct symbol *scope_lookup(char *name);      // Look up symbol in current scope only
struct symbol *lookup_all_scopes(char *name); // Look up symbol in all accessible scopes

/* Function management */
char *process_string_literal(char *str);
void push_function(struct symbol *func, struct scope *caller_scope);
struct function_context *pop_function(void);
struct function_context *current_function(void);
struct value eval_function_body(struct ast *body, struct symbol *func);
void dodef(struct symbol *name, struct symbol_list *syms, struct ast *stmts);

/* Type system */
struct ast *newdecl(struct symbol *s);
void settype(struct symbol *sym, enum value_type type);
enum value_type promote_types(enum value_type t1, enum value_type t2);
void convert_binary_operands(struct value *v1, struct value *v2, enum value_type result_type);
void safe_convert_value(struct value *dest, struct value *src);
void convert_value(void *dest, enum value_type dest_type,
                   void *src, enum value_type src_type);

/* AST construction */
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newcast(enum value_type type, struct ast *operand);
struct ast *newstring(char *s);
struct ast *newcmp(int cmptype, struct ast *l, struct ast *r);
struct ast *newfunc(int functype, struct ast *l);
struct ast *newblock(struct ast *statements, struct scope *scope);
struct ast *newcall(struct symbol *s, struct ast *l);
struct ast *newref(struct symbol *s);
struct ast *newasgn(struct symbol *s, struct ast *v);
struct ast *newnum(enum value_type type, union value_union value);
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *tr);
struct ast *newstrast(char *str);

/* Evaluation and cleanup */
struct value eval(struct ast *);
void treefree(struct ast *);

/* Error handling */
void error(char *s, ...);
void cleanup_and_exit(int status);

/* Debugging */
void dumpast(struct ast *a, int level);

/* Running a script */
int run_script(const char *filename);

/* ========================================================================== */
/*                                    DEBUG                                   */
/* ========================================================================== */

void print_scope_chain(const char *prefix);

//#define SCOPE_CHAIN_PRINT 1

#ifdef SCOPE_CHAIN_PRINT
#define PRINT_SCOPE_CHAIN(prefix)     \
    do                                \
    {                                 \
        print_scope_chain(prefix);    \
    } while (0)
#else
#define PRINT_SCOPE_CHAIN(prefix)
#endif

//#define DEBUG 1

#ifdef DEBUG
#define DEBUG_PRINT_AST(ast, level)   \
    do                                \
    {                                 \
        printf("\nAST Structure:\n"); \
        dumpast(ast, level);          \
        printf("\n");                 \
    } while (0)
#else
#define DEBUG_PRINT_AST(ast, level)
#endif

//#define DEBUG_MEM 1

#ifdef DEBUG_MEM
#define MEM_DEBUG(fmt, ...) \
    fprintf(stderr, "MEM_DEBUG [%s:%d]: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define MEM_DEBUG(fmt, ...)
#endif

#endif