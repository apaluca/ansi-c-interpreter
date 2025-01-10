/* ========================================================================== */
/*                              Type Definitions                              */
/* ========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "ansi_c_interpreter.h"

/* ========================================================================== */
/*                              Global Variables                              */
/* ========================================================================== */

struct ast *root = NULL;
enum value_type current_type = NO_TYPE;
struct scope *current_scope = NULL;
struct function_context function_stack[MAX_FUNCTION_DEPTH];
int function_depth = 0;
static struct symbol *current_function_sym = NULL;
int interactive_mode = 1;
int error_state = 0;
jmp_buf error_jmp;

/* ========================================================================== */
/*                              Scope Management                              */
/* ========================================================================== */

struct scope *push_scope(void)
{
    struct scope *new_scope = malloc(sizeof(struct scope));
    if (!new_scope)
    {
        error("out of space");
        exit(0);
    }

    new_scope->symbols = NULL;
    new_scope->parent = current_scope;
    current_scope = new_scope;

    PRINT_SCOPE_CHAIN("After Push");

    return new_scope;
}

void pop_scope(void)
{
    if (!current_scope)
    {
        MEM_DEBUG("Warning: Attempting to pop null scope");
        return;
    }

    MEM_DEBUG("Popping scope %p (parent: %p)",
              (void *)current_scope,
              (void *)(current_scope->parent));

    struct scope *scope_to_pop = current_scope;
    struct scope *parent_scope = current_scope->parent;

    // First free all symbols in this scope
    struct symbol_table *st = scope_to_pop->symbols;
    while (st)
    {
        if (st->sym)
        {
            // Don't free function definitions or their symbols
            if (!st->sym->func)
            {
                MEM_DEBUG("Freeing symbol %s in scope %p",
                          st->sym->name ? st->sym->name : "(unnamed)",
                          (void *)scope_to_pop);
                if (st->sym->name)
                    free(st->sym->name);
                if (st->sym->syms)
                    symlistfree(st->sym->syms);
                free(st->sym);
            }
            else
            {
                MEM_DEBUG("Preserving function symbol %s", st->sym->name);
            }
        }
        struct symbol_table *next = st->next;
        free(st);
        st = next;
    }

    // Now free the scope itself
    current_scope = parent_scope;
    free(scope_to_pop);

    PRINT_SCOPE_CHAIN("After Pop");
}

struct symbol *scope_lookup(char *name)
{
    if (!current_scope || !name)
    {
        return NULL;
    }

    struct symbol_table *st = current_scope->symbols;
    while (st)
    {
        if (st->sym && st->sym->name && strcmp(st->sym->name, name) == 0)
        {
            return st->sym;
        }
        st = st->next;
    }

    return NULL;
}

struct symbol *lookup_all_scopes(char *name)
{
    MEM_DEBUG("Looking up symbol %s in all scopes", name);

    // For declarations, only look in current scope
    if (current_type != NO_TYPE)
    {
        struct symbol *current = scope_lookup(name);
        if (current)
        {
            MEM_DEBUG("Found %s in current scope during declaration", name);
            return current;
        }
        return NULL;
    }

    // For non-declarations (expressions), look in all scopes
    struct scope *s = current_scope;
    struct symbol *innermost = NULL;
    struct symbol *outermost = NULL;

    while (s)
    {
        MEM_DEBUG("Searching scope %p for %s", (void *)s, name);
        struct symbol_table *st = s->symbols;
        while (st)
        {
            if (st->sym && st->sym->name && strcmp(st->sym->name, name) == 0)
            {
                if (!innermost)
                {
                    innermost = st->sym;
                    MEM_DEBUG("Found innermost definition of %s", name);
                }
                outermost = st->sym;
                MEM_DEBUG("Found outermost definition of %s", name);
            }
            st = st->next;
        }
        s = s->parent;
    }

    // If this is an assignment target, use the innermost (shadowed) version
    if (current_scope && scope_lookup(name))
    {
        MEM_DEBUG("Returning innermost definition of %s for assignment", name);
        return innermost;
    }

    // For expressions, use the outermost version
    if (outermost)
    {
        MEM_DEBUG("Returning outermost definition of %s for expression", name);
        return outermost;
    }

    MEM_DEBUG("Symbol %s not found in any scope", name);
    return NULL;
}

/* ========================================================================== */
/*                              Type System                                   */
/* ========================================================================== */

void settype(struct symbol *sym, enum value_type type)
{
    if (sym->type != type)
    {
        sym->type = type;
        // Initialize value to 0 of the appropriate type
        switch (type)
        {
        case TYPE_INT:
            sym->value.i_val = 0;
            break;
        case TYPE_FLOAT:
            sym->value.f_val = 0.0f;
            break;
        case TYPE_DOUBLE:
            sym->value.d_val = 0.0;
            break;
        }
    }
}

enum value_type promote_types(enum value_type t1, enum value_type t2)
{
    // If either type is double, result is double
    if (t1 == TYPE_DOUBLE || t2 == TYPE_DOUBLE)
        return TYPE_DOUBLE;

    // If either type is float, result is float
    if (t1 == TYPE_FLOAT || t2 == TYPE_FLOAT)
        return TYPE_FLOAT;

    // Otherwise result is int
    return TYPE_INT;
}

void convert_binary_operands(struct value *v1, struct value *v2, enum value_type result_type)
{
    struct value temp1 = *v1;
    struct value temp2 = *v2;

    v1->type = result_type;
    v2->type = result_type;

    safe_convert_value(v1, &temp1);
    safe_convert_value(v2, &temp2);
}

void safe_convert_value(struct value *dest, struct value *src)
{
    if (!dest || !src)
        return;

    // If types are the same, just copy
    if (dest->type == src->type)
    {
        dest->value = src->value;
        return;
    }

    // Handle conversions
    switch (src->type)
    {
    case TYPE_INT:
        switch (dest->type)
        {
        case TYPE_FLOAT:
            dest->value.f_val = (float)src->value.i_val;
            break;
        case TYPE_DOUBLE:
            dest->value.d_val = (double)src->value.i_val;
            break;
        }
        break;

    case TYPE_FLOAT:
        switch (dest->type)
        {
        case TYPE_INT:
            dest->value.i_val = (int)src->value.f_val;
            break;
        case TYPE_DOUBLE:
            dest->value.d_val = (double)src->value.f_val;
            break;
        }
        break;

    case TYPE_DOUBLE:
        switch (dest->type)
        {
        case TYPE_INT:
            dest->value.i_val = (int)src->value.d_val;
            break;
        case TYPE_FLOAT:
            dest->value.f_val = (float)src->value.d_val;
            break;
        }
        break;
    }
}

void convert_value(void *dest, enum value_type dest_type, void *src, enum value_type src_type)
{
    union value_union *s = (union value_union *)src;
    union value_union *d = (union value_union *)dest;

    if (dest_type == src_type)
    {
        memcpy(dest, src, (dest_type == TYPE_INT) ? sizeof(int) : (dest_type == TYPE_FLOAT) ? sizeof(float)
                                                                                            : sizeof(double));
        return;
    }

    switch (src_type)
    {
    case TYPE_INT:
        switch (dest_type)
        {
        case TYPE_FLOAT:
            d->f_val = (float)s->i_val;
            break;
        case TYPE_DOUBLE:
            d->d_val = (double)s->i_val;
            break;
        }
        break;
    case TYPE_FLOAT:
        switch (dest_type)
        {
        case TYPE_INT:
            d->i_val = (int)s->f_val;
            break;
        case TYPE_DOUBLE:
            d->d_val = (double)s->f_val;
            break;
        }
        break;
    case TYPE_DOUBLE:
        switch (dest_type)
        {
        case TYPE_INT:
            d->i_val = (int)s->d_val;
            break;
        case TYPE_FLOAT:
            d->f_val = (float)s->d_val;
            break;
        }
        break;
    }
}

/* ========================================================================== */
/*                              Symbol Management                             */
/* ========================================================================== */

void init_symbol(struct symbol *sym)
{
    if (!sym)
        return;

    switch (sym->type)
    {
    case TYPE_INT:
        sym->value.i_val = 0;
        break;
    case TYPE_FLOAT:
        sym->value.f_val = 0.0f;
        break;
    case TYPE_DOUBLE:
        sym->value.d_val = 0.0;
        break;
    }
}

struct symbol *lookup_function(char *name)
{
    struct scope *s = current_scope;

    while (s)
    {
        struct symbol_table *st = s->symbols;
        while (st)
        {
            if (strcmp(st->sym->name, name) == 0 && st->sym->func != NULL)
            {
                return st->sym;
            }
            st = st->next;
        }
        s = s->parent;
    }
    return NULL;
}

struct symbol *lookup(char *sym)
{

    struct symbol *found = lookup_all_scopes(sym);
    if (!found && current_type == NO_TYPE)
    {
        error("use of undeclared identifier '%s'", sym);
        // Will not return - error() exits
    }

    return found;
}

struct symbol_list *newsymlist(struct symbol *sym, struct symbol_list *next)
{
    struct symbol_list *sl = malloc(sizeof(struct symbol_list));
    if (!sl)
    {
        error("out of space");
        exit(0);
    }
    sl->sym = sym;
    sl->next = next;
    return sl;
}

void symlistfree(struct symbol_list *sl)
{
    struct symbol_list *nsl;

    while (sl)
    {
        nsl = sl->next;
        free(sl);
        sl = nsl;
    }
}

/* ========================================================================== */
/*                              AST Building                                  */
/* ========================================================================== */

struct ast *newast(int nodetype, struct ast *l, struct ast *r)
{
    struct ast *a = malloc(sizeof(struct ast));
    if (!a)
    {
        error("out of space");
        exit(0);
    }
    a->nodetype = nodetype;
    a->l = l;
    a->r = r;

    // Determine result type based on operands
    if (l && r)
    {
        enum value_type lt = l->result_type;
        enum value_type rt = r->result_type;
        a->result_type = promote_types(lt, rt);
    }
    else if (l)
    {
        a->result_type = l->result_type;
    }
    else
    {
        a->result_type = TYPE_INT; // default type
    }

    return a;
}

struct ast *newcast(enum value_type type, struct ast *operand)
{
    struct typecast *a = malloc(sizeof(struct typecast));
    if (!a)
    {
        error("out of space");
        exit(0);
    }
    a->nodetype = 'T';
    a->type = type;
    a->operand = operand;
    a->result_type = type;
    return (struct ast *)a;
}

struct ast *newstring(char *s)
{
    struct strast *str = malloc(sizeof(struct strast));
    if (!str)
    {
        error("out of space");
        exit(0);
    }
    str->nodetype = 'S';
    str->string = s;
    str->next = NULL;
    str->result_type = TYPE_INT;
    return (struct ast *)str;
}

struct ast *newdecl(struct symbol *s)
{
    /* Check for redefinition in current scope only */
    struct symbol *existing = scope_lookup(s->name);
    if (existing)
    {
        if (s->name)
            free(s->name);
        if (s->func)
            treefree(s->func);
        if (s->syms)
            symlistfree(s->syms);
        free(s);
        error("redefinition of '%s' in current scope", existing->name);
        return NULL;
    }

    /* Create new symbol table entry */
    struct symbol_table *st = malloc(sizeof(struct symbol_table));
    if (!st)
    {
        if (s->name)
            free(s->name);
        if (s->func)
            treefree(s->func);
        if (s->syms)
            symlistfree(s->syms);
        free(s);
        error("out of space");
        return NULL;
    }

    /* Initialize the symbol value based on type */
    switch (s->type)
    {
    case TYPE_INT:
        s->value.i_val = 0;
        break;
    case TYPE_FLOAT:
        s->value.f_val = 0.0f;
        break;
    case TYPE_DOUBLE:
        s->value.d_val = 0.0;
        break;
    }

    st->sym = s;
    st->next = current_scope->symbols;
    current_scope->symbols = st;

    struct ast *a = malloc(sizeof(struct ast));
    if (!a)
    {
        current_scope->symbols = st->next;
        if (s->name)
            free(s->name);
        if (s->func)
            treefree(s->func);
        if (s->syms)
            symlistfree(s->syms);
        free(s);
        free(st);
        error("out of space");
        return NULL;
    }

    a->nodetype = 'D';
    a->l = (struct ast *)s;
    a->r = NULL;
    a->result_type = s->type;

    return a;
}

struct ast *newnum(enum value_type type, union value_union value)
{
    struct numval *a = malloc(sizeof(struct numval));
    if (!a)
    {
        error("out of space");
        exit(0);
    }
    a->nodetype = 'K';
    a->type = type;
    a->value = value;
    ((struct ast *)a)->result_type = type;
    return (struct ast *)a;
}

struct ast *newcmp(int cmptype, struct ast *l, struct ast *r)
{
    struct ast *a = malloc(sizeof(struct ast));
    if (!a)
    {
        error("out of space");
        exit(0);
    }
    a->nodetype = '0' + cmptype;
    a->l = l;
    a->r = r;
    a->result_type = TYPE_INT;
    return a;
}

struct ast *newblock(struct ast *statements, struct scope *scope)
{
    MEM_DEBUG("Creating new block with original scope %p", (void *)scope);
    struct block *b = malloc(sizeof(struct block));
    if (!b)
    {
        error("out of space");
        return NULL;
    }

    b->nodetype = 'B';
    b->statements = statements;
    b->block_scope = scope; // Store original scope
    b->needs_scope = 1;     // Default to creating new scope during execution

    if (statements)
    {
        ((struct ast *)b)->result_type = statements->result_type;
    }
    else
    {
        ((struct ast *)b)->result_type = TYPE_INT;
    }

    MEM_DEBUG("Block created with statements at %p", (void *)statements);
    return (struct ast *)b;
}

struct ast *newfunc(int functype, struct ast *l)
{
    struct fncall *a = malloc(sizeof(struct fncall));
    if (!a)
    {
        error("out of space");
        exit(0);
    }
    a->nodetype = 'F';
    a->l = l;
    a->functype = functype;
    ((struct ast *)a)->result_type = TYPE_DOUBLE;
    return (struct ast *)a;
}

struct ast *newcall(struct symbol *s, struct ast *l)
{
    struct ufncall *a = malloc(sizeof(struct ufncall));
    if (!a)
    {
        error("out of space");
        exit(0);
    }
    a->nodetype = 'C';
    a->l = l;
    a->s = s;
    ((struct ast *)a)->result_type = s->type;
    return (struct ast *)a;
}

struct ast *newref(struct symbol *s)
{
    struct symref *a = malloc(sizeof(struct symref));
    if (!a)
    {
        error("out of space");
        exit(0);
    }

    // Always look up the symbol in all accessible scopes
    struct symbol *found = lookup_all_scopes(s->name);

    if (found)
    {
        a->s = found;
    }
    else if (current_type != NO_TYPE)
    {
        a->s = s;
    }
    else
    {
        error("undefined variable '%s'", s->name);
        free(a);
        exit(0);
    }

    ((struct ast *)a)->nodetype = 'N';
    ((struct ast *)a)->result_type = a->s->type;
    return (struct ast *)a;
}

struct ast *newasgn(struct symbol *s, struct ast *v)
{
    struct symasgn *a = malloc(sizeof(struct symasgn));
    if (!a)
    {
        error("out of space");
        exit(0);
    }
    a->nodetype = '=';
    a->s = s;
    a->v = v;
    ((struct ast *)a)->result_type = s->type;
    return (struct ast *)a;
}

struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el)
{
    struct flow *a = malloc(sizeof(struct flow));
    if (!a)
    {
        error("out of space");
        exit(0);
    }
    a->nodetype = nodetype;
    a->cond = cond;
    a->tl = tl;
    a->el = el;

    if (tl)
    {
        ((struct ast *)a)->result_type = tl->result_type;
    }
    else if (el)
    {
        ((struct ast *)a)->result_type = el->result_type;
    }
    else
    {
        ((struct ast *)a)->result_type = TYPE_INT;
    }
    return (struct ast *)a;
}

struct ast *newstrast(char *str)
{
    struct strast *s = malloc(sizeof(struct strast));

    if (!str)
    {
        error("null string passed to newstrast");
        exit(0);
    }

    if (!s)
    {
        error("out of memory");
        exit(0);
    }

    s->nodetype = 'S';
    s->string = str; // Takes ownership of the string
    s->next = NULL;
    s->result_type = TYPE_INT;

    return (struct ast *)s;
}

/* ========================================================================== */
/*                              AST Management                                */
/* ========================================================================== */

void free_string_ast(struct strast *s)
{
    if (!s)
        return;
    if (s->string)
    {
        free(s->string);
        s->string = NULL;
    }
}

void treefree(struct ast *a)
{
    if (!a)
        return;

    // Add a guard against invalid node types
    if (a->nodetype < 0 || (a->nodetype > 'z' && a->nodetype < 256))
    {
        return;
    }

    switch (a->nodetype)
    {
    /* two subtrees */
    case '+':
    case '-':
    case '*':
    case '/':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case 'L':
        if (a->r)
        {
            treefree(a->r);
        }
        /* fall through */

    /* one subtree */
    case '|':
    case 'M':
    case 'C':
    case 'F':
        if (a->l)
        {
            treefree(a->l);
        }
        break;

    /* no subtree */
    case 'D':
    case 'K':
    case 'N':
        break;

    case '=':
        if (((struct symasgn *)a)->v)
        {
            treefree(((struct symasgn *)a)->v);
        }
        break;

    case 'I':
    case 'W':
        if (((struct flow *)a)->cond)
        {
            treefree(((struct flow *)a)->cond);
        }
        if (((struct flow *)a)->tl)
        {
            treefree(((struct flow *)a)->tl);
        }
        if (((struct flow *)a)->el)
        {
            treefree(((struct flow *)a)->el);
        }
        break;

    case 'S':
    {
        struct strast *s = (struct strast *)a;
        if (s->string)
        {
            free(s->string);
            s->string = NULL;
        }
        if (s->next)
        {
            treefree((struct ast *)s->next);
            s->next = NULL;
        }
        break;
    }

    case 'B':
    {
        struct block *b = (struct block *)a;
        // First free any statements in the block
        if (b->statements)
        {
            treefree(b->statements);
            b->statements = NULL;
        }

        // The scope itself is managed by pop_scope()
        // Don't free it here as it might still be in use
        b->block_scope = NULL; // Just clear the reference
        break;
    }

    default:
        break;
    }

    free(a);
}

/* ========================================================================== */
/*                              Function Management                           */
/* ========================================================================== */

/* Helper to reverse a symbol list */
static struct symbol_list *reverse_symbol_list(struct symbol_list *sl)
{
    struct symbol_list *prev = NULL;
    struct symbol_list *current = sl;
    struct symbol_list *next = NULL;

    while (current != NULL)
    {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }

    return prev;
}

void dodef(struct symbol *name, struct symbol_list *syms, struct ast *func)
{
    MEM_DEBUG("Defining function %s", name->name);

    if (name->syms)
        symlistfree(name->syms);
    if (name->func)
        treefree(name->func);

    name->syms = syms; // Already reversed in grammar
    name->func = func;

    MEM_DEBUG("Function %s defined with body at %p", name->name, (void *)func);
}

void push_function(struct symbol *func, struct scope *caller_scope)
{
    MEM_DEBUG("Pushing function %s with caller scope %p", func->name, (void *)caller_scope);

    if (function_depth >= MAX_FUNCTION_DEPTH)
    {
        error("maximum function call depth exceeded");
        return;
    }

    function_stack[function_depth].function = func;
    function_stack[function_depth].caller_scope = caller_scope;
    function_depth++;

    MEM_DEBUG("Function depth is now %d", function_depth);
}

struct function_context *pop_function(void)
{

    if (function_depth <= 0)
    {
        return NULL;
    }

    function_depth--;

    return &function_stack[function_depth];
}

struct function_context *current_function(void)
{
    if (function_depth <= 0)
        return NULL;
    return &function_stack[function_depth - 1];
}

/* ========================================================================== */
/*                              Expression Evaluation                         */
/* ========================================================================== */

static struct value *evaluate_arguments(struct ast *args, int *count)
{
    struct value *arg_values = NULL;
    *count = 0;

    /* First count arguments */
    struct ast *temp = args;
    while (temp)
    {
        (*count)++;
        if (temp->nodetype == 'L')
        {
            temp = temp->r;
        }
        else
        {
            break;
        }
    }

    if (*count > 0)
    {
        arg_values = malloc(*count * sizeof(struct value));
        if (!arg_values)
        {
            error("out of memory");
            return NULL;
        }

        /* Now evaluate in left-to-right order */
        temp = args;
        int i = 0;
        while (temp && i < *count)
        {
            if (temp->nodetype == 'L')
            {
                arg_values[i] = eval(temp->l);
                temp = temp->r;
            }
            else
            {
                arg_values[i] = eval(temp);
                temp = NULL;
            }
            i++;
        }
    }

    return arg_values;
}

struct value eval_function_body(struct ast *body, struct symbol *func)
{
    MEM_DEBUG("Entering eval_function_body for function %s", func->name);
    struct value result = {0};

    if (!body)
    {
        MEM_DEBUG("Empty function body for %s", func->name);
        result.type = func->type;
        result.value.i_val = 0;
        return result;
    }

    MEM_DEBUG("Function body node type: %c", body->nodetype);

    // If the body is a block, make sure it creates a new scope
    if (body->nodetype == 'B')
    {
        ((struct block *)body)->needs_scope = 1;
    }

    /* Evaluate the body */
    result = eval(body);
    MEM_DEBUG("Function %s evaluated with result value: ", func->name);
    switch (result.type)
    {
    case TYPE_INT:
        MEM_DEBUG("INT: %d", result.value.i_val);
        break;
    case TYPE_FLOAT:
        MEM_DEBUG("FLOAT: %f", result.value.f_val);
        break;
    case TYPE_DOUBLE:
        MEM_DEBUG("DOUBLE: %f", result.value.d_val);
        break;
    }

    /* Ensure return value matches function type */
    if (result.type != func->type)
    {
        MEM_DEBUG("Converting return value from type %d to function type %d",
                  result.type, func->type);
        struct value temp = result;
        result.type = func->type;
        convert_value(&result.value, result.type, &temp.value, temp.type);
    }

    MEM_DEBUG("Finished eval_function_body for %s with final value: ", func->name);
    switch (result.type)
    {
    case TYPE_INT:
        MEM_DEBUG("INT: %d", result.value.i_val);
        break;
    case TYPE_FLOAT:
        MEM_DEBUG("FLOAT: %f", result.value.f_val);
        break;
    case TYPE_DOUBLE:
        MEM_DEBUG("DOUBLE: %f", result.value.d_val);
        break;
    }
    return result;
}

/* ========================================================================== */
/*                              Built-in Functions                            */
/* ========================================================================== */

static struct value eval_printf(struct ast *format_node, struct ast *args)
{
    struct value result = {.type = TYPE_INT, .value.i_val = 0};

    if (!format_node || format_node->nodetype != 'S')
    {
        error("printf requires a string literal format");
        return result;
    }

    struct strast *fmt = (struct strast *)format_node;
    const char *format = fmt->string;
    int chars_written = 0;
    struct ast *current_arg = args;

    for (const char *p = format; *p; p++)
    {
        if (*p == '%')
        {
            p++;
            if (!current_arg)
            {
                error("Too few arguments for format string");
                break;
            }

            struct value arg_val = eval(current_arg);
            switch (*p)
            {
            case 'd':
                if (arg_val.type != TYPE_INT)
                {
                    error("format specifier %%d requires int argument");
                }
                else
                {
                    chars_written += printf("%d", arg_val.value.i_val);
                }
                break;

            case 'f':
                if (arg_val.type == TYPE_FLOAT)
                {
                    chars_written += printf("%f", arg_val.value.f_val);
                }
                else if (arg_val.type == TYPE_DOUBLE)
                {
                    chars_written += printf("%f", arg_val.value.d_val);
                }
                else
                {
                    error("format specifier %%f requires float or double argument");
                }
                break;

            default:
                error("unsupported format specifier %%%c", *p);
                break;
            }

            if (current_arg->nodetype == 'L')
            {
                current_arg = current_arg->r;
            }
            else
            {
                current_arg = NULL;
            }
        }
        else
        {
            putchar(*p);
            chars_written++;
        }
    }

    result.value.i_val = chars_written;
    return result;
}

static struct value eval_scanf(struct ast *format_node, struct ast *args)
{
    struct value result = {.type = TYPE_INT, .value.i_val = 0};

    if (!format_node || format_node->nodetype != 'S')
    {
        error("scanf requires a string literal format");
        return result;
    }

    if (!args || args->nodetype != 'N')
    {
        error("scanf requires a variable reference");
        return result;
    }

    struct strast *fmt = (struct strast *)format_node;
    struct symref *ref = (struct symref *)args;
    int items_read = 0;

    switch (fmt->string[1])
    { // Check format character after %
    case 'd':
        if (ref->s->type != TYPE_INT)
        {
            error("format %%d requires int variable");
        }
        else
        {
            // Read the value
            if (scanf("%d", &ref->s->value.i_val) != 1)
            {
                error("scanf failed to read integer");
            }
            items_read = 1;
        }
        break;

    case 'f':
        if (ref->s->type == TYPE_FLOAT)
        {
            if (scanf("%f", &ref->s->value.f_val) != 1)
            {
                error("scanf failed to read float");
            }
            items_read = 1;
        }
        else if (ref->s->type == TYPE_DOUBLE)
        {
            if (scanf("%lf", &ref->s->value.d_val) != 1)
            {
                error("scanf failed to read double");
            }
            items_read = 1;
        }
        else
        {
            error("format %%f requires float or double variable");
        }
        break;

    default:
        error("unsupported format specifier");
        break;
    }

    // Read and discard any remaining characters up to and including newline
    char c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
        // Discard the character
    }

    // Reset any error indicators on stdin
    clearerr(stdin);

    result.value.i_val = items_read;
    return result;
}

/* ========================================================================== */
/*                              Core Evaluation                               */
/* ========================================================================== */

struct value eval(struct ast *a)
{
    struct scope *entry_scope = current_scope;
    struct value v1, v2, result = {0};

    if (!a)
    {
        error("internal error, null eval");
        result.type = TYPE_INT;
        result.value.i_val = 0;
        return result;
    }

    switch (a->nodetype)
    {
    /* typecast */
    case 'T':
    {
        struct typecast *t = (struct typecast *)a;
        struct value operand = eval(t->operand);
        result.type = t->type;
        convert_value(&result.value, result.type, &operand.value, operand.type);
        return result;
    }

    /* return statement */
    case 'R':
    {
        MEM_DEBUG("Processing return statement");
        struct function_context *ctx = current_function();

        if (!current_function_sym)
        {
            error("return statement outside of function");
            result.type = TYPE_INT;
            result.value.i_val = 0;
            return result;
        }

        if (a->l)
        {
            result = eval(a->l);
            MEM_DEBUG("Return statement evaluated expression with result: ");
            MEM_DEBUG("INT: %d", result.value.i_val);

            if (ctx)
            {
                ctx->has_returned = 1;
                ctx->return_value = result;
            }
        }
        else
        {
            result.type = current_function_sym->type;
            result.value.i_val = 0;
            MEM_DEBUG("Empty return statement, returning 0");

            if (ctx)
            {
                ctx->has_returned = 1;
                ctx->return_value = result;
            }
        }

        return result;
    }

    /* declaration */
    case 'D':
    {
        struct symbol *s = (struct symbol *)a->l;
        init_symbol(s); // Initialize the symbol

        result.type = s->type;
        result.value = s->value; // Copy initialized value
        break;
    }

    /* constant */
    case 'K':
    {
        struct numval *n = (struct numval *)a;
        result.type = n->type;
        result.value = n->value;
        return result;
    }

    /* name reference */
    case 'N':
    {
        struct symref *s = (struct symref *)a;

        struct symbol *sym = lookup_all_scopes(s->s->name);
        if (!sym)
        {
            error("undefined variable '%s'", s->s->name);
            break;
        }

        result.type = sym->type;
        result.value = sym->value;
        break;
    }

    /* assignment */
    case '=':
    {
        struct symasgn *sa = (struct symasgn *)a;

        // Evaluate right-hand side first
        struct value rhs = eval(sa->v);

        // For assignments, we want the innermost matching symbol
        struct symbol *sym = lookup_all_scopes(sa->s->name);
        if (!sym)
        {
            error("assignment to undeclared variable '%s'", sa->s->name);
            break;
        }

        // Create a temporary value with the target type
        struct value target_val;
        target_val.type = sym->type;

        // Convert the value using our safe conversion function
        safe_convert_value(&target_val, &rhs);

        // Store the converted value
        sym->value = target_val.value;

        // Assignment expression returns the original (unconverted) value
        result = rhs;
        break;
    }

    /* binary operations */
    case '+':
    case '-':
    case '*':
    case '/':
    {
        v1 = eval(a->l);
        v2 = eval(a->r);

        // Debug the values before operation
        switch (v1.type)
        {
        case TYPE_INT:
            break;
        case TYPE_FLOAT:
        case TYPE_DOUBLE:
            break;
        }
        switch (v2.type)
        {
        case TYPE_INT:
            break;
        case TYPE_FLOAT:
        case TYPE_DOUBLE:
            break;
        }

        // Determine result type and convert operands
        result.type = promote_types(v1.type, v2.type);
        convert_binary_operands(&v1, &v2, result.type);

        switch (result.type)
        {
        case TYPE_INT:
            switch (a->nodetype)
            {
            case '+':
                result.value.i_val = v1.value.i_val + v2.value.i_val;
                break;
            case '-':
                result.value.i_val = v1.value.i_val - v2.value.i_val;
                break;
            case '*':
                result.value.i_val = v1.value.i_val * v2.value.i_val;
                break;
            case '/':
                if (v2.value.i_val == 0)
                {
                    error("division by zero");
                    result.value.i_val = 0;
                }
                else
                {
                    result.value.i_val = v1.value.i_val / v2.value.i_val;
                }
                break;
            }
            break;

        case TYPE_FLOAT:
            switch (a->nodetype)
            {
            case '+':
                result.value.f_val = v1.value.f_val + v2.value.f_val;
                break;
            case '-':
                result.value.f_val = v1.value.f_val - v2.value.f_val;
                break;
            case '*':
                result.value.f_val = v1.value.f_val * v2.value.f_val;
                break;
            case '/':
                if (v2.value.f_val == 0.0f)
                {
                    error("division by zero");
                    result.value.f_val = 0.0f;
                }
                else
                {
                    result.value.f_val = v1.value.f_val / v2.value.f_val;
                }
                break;
            }
            break;

        case TYPE_DOUBLE:
            switch (a->nodetype)
            {
            case '+':
                result.value.d_val = v1.value.d_val + v2.value.d_val;
                break;
            case '-':
                result.value.d_val = v1.value.d_val - v2.value.d_val;
                break;
            case '*':
                result.value.d_val = v1.value.d_val * v2.value.d_val;
                break;
            case '/':
                if (v2.value.d_val == 0.0)
                {
                    error("division by zero");
                    result.value.d_val = 0.0;
                }
                else
                {
                    result.value.d_val = v1.value.d_val / v2.value.d_val;
                }
                break;
            }
            break;
        }
        return result;
    }

    /* unary operations */
    case '|':
    {
        v1 = eval(a->l);
        result.type = v1.type;
        switch (v1.type)
        {
        case TYPE_INT:
            result.value.i_val = abs(v1.value.i_val);
            break;
        case TYPE_FLOAT:
            result.value.f_val = fabsf(v1.value.f_val);
            break;
        case TYPE_DOUBLE:
            result.value.d_val = fabs(v1.value.d_val);
            break;
        }
        return result;
    }

    case 'M':
    { // unary minus
        v1 = eval(a->l);
        result.type = v1.type;
        switch (v1.type)
        {
        case TYPE_INT:
            result.value.i_val = -v1.value.i_val;
            break;
        case TYPE_FLOAT:
            result.value.f_val = -v1.value.f_val;
            break;
        case TYPE_DOUBLE:
            result.value.d_val = -v1.value.d_val;
            break;
        }
        return result;
    }

    /* function calls */
    case 'F':
    {
        struct fncall *f = (struct fncall *)a;
        if (!f->l)
        {
            error("Missing arguments for built-in function");
            result.type = TYPE_INT;
            result.value.i_val = 0;
            return result;
        }

        // First argument must be a string node
        if (f->l->nodetype != 'S' &&
            (f->l->nodetype != 'L' || f->l->l->nodetype != 'S'))
        {
            error("First argument to built-in function must be a string literal");
            result.type = TYPE_INT;
            result.value.i_val = 0;
            return result;
        }

        struct ast *format_node;
        struct ast *args = NULL;

        if (f->l->nodetype == 'L')
        {
            format_node = f->l->l;
            args = f->l->r;
        }
        else
        {
            format_node = f->l;
        }

        switch (f->functype)
        {
        case B_printf:
            return eval_printf(format_node, args);
        case B_scanf:
            return eval_scanf(format_node, args);
        }
        return result;
    }

    case 'S':
    {
        struct strast *s = (struct strast *)a;
        result.type = TYPE_INT;
        result.value.i_val = 0;
        return result;
    }

        /* comparisons */
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    {
        v1 = eval(a->l);
        v2 = eval(a->r);

        enum value_type comp_type = promote_types(v1.type, v2.type);
        union value_union temp1, temp2;
        convert_value(&temp1, comp_type, &v1.value, v1.type);
        convert_value(&temp2, comp_type, &v2.value, v2.type);

        result.type = TYPE_INT; // comparisons always return int

        switch (comp_type)
        {
        case TYPE_INT:
        {
            int cmp = temp1.i_val - temp2.i_val;
            switch (a->nodetype)
            {
            case '1':
                result.value.i_val = cmp > 0;
                break; // >
            case '2':
                result.value.i_val = cmp < 0;
                break; // <
            case '3':
                result.value.i_val = cmp != 0;
                break; // <>
            case '4':
                result.value.i_val = cmp == 0;
                break; // ==
            case '5':
                result.value.i_val = cmp >= 0;
                break; // >=
            case '6':
                result.value.i_val = cmp <= 0;
                break; // <=
            }
            break;
        }
        case TYPE_FLOAT:
        {
            float cmp = temp1.f_val - temp2.f_val;
            switch (a->nodetype)
            {
            case '1':
                result.value.i_val = cmp > 0;
                break;
            case '2':
                result.value.i_val = cmp < 0;
                break;
            case '3':
                result.value.i_val = cmp != 0;
                break;
            case '4':
                result.value.i_val = cmp == 0;
                break;
            case '5':
                result.value.i_val = cmp >= 0;
                break;
            case '6':
                result.value.i_val = cmp <= 0;
                break;
            }
            break;
        }
        case TYPE_DOUBLE:
        {
            double cmp = temp1.d_val - temp2.d_val;
            switch (a->nodetype)
            {
            case '1':
                result.value.i_val = cmp > 0;
                break;
            case '2':
                result.value.i_val = cmp < 0;
                break;
            case '3':
                result.value.i_val = cmp != 0;
                break;
            case '4':
                result.value.i_val = cmp == 0;
                break;
            case '5':
                result.value.i_val = cmp >= 0;
                break;
            case '6':
                result.value.i_val = cmp <= 0;
                break;
            }
            break;
        }
        }
        return result;
    }

    /* control flow */
    case 'I':
    {
        v1 = eval(((struct flow *)a)->cond);

        if (v1.value.i_val != 0)
        {
            if (((struct flow *)a)->tl)
            {
                result = eval(((struct flow *)a)->tl);
            }
            else
            {
                result.type = TYPE_INT;
                result.value.i_val = 0;
            }
        }
        else
        {
            if (((struct flow *)a)->el)
            {
                result = eval(((struct flow *)a)->el);
            }
            else
            {
                result.type = TYPE_INT;
                result.value.i_val = 0;
            }
        }
        return result;
    }

    case 'W':
    {
        result.type = TYPE_INT;
        result.value.i_val = 0;

        if (((struct flow *)a)->tl)
        {
            while (1)
            {
                v1 = eval(((struct flow *)a)->cond);

                if (!v1.value.i_val)
                    break;

                result = eval(((struct flow *)a)->tl);

                // Verify scope is still valid
                if (!current_scope)
                {
                    error("Lost scope in while loop");
                    return result;
                }
            }
        }
        return result;
    }

    /* block evaluation */
    case 'B':
    {
        struct block *b = (struct block *)a;
        struct scope *new_scope = NULL;
        struct scope *saved_scope = current_scope;
        struct function_context *ctx = current_function();
        struct value block_result = {0};

        MEM_DEBUG("Evaluating block (original scope: %p)", (void *)b->block_scope);

        if (b->needs_scope)
        {
            MEM_DEBUG("Creating new scope for block execution");
            new_scope = push_scope();
            if (!new_scope)
            {
                error("Failed to create new scope for block");
                return result;
            }
            current_scope = new_scope;
        }

        // Evaluate statements
        if (b->statements)
        {
            MEM_DEBUG("Evaluating block statements, node type: %c", b->statements->nodetype);
            block_result = eval(b->statements);

            // If we've seen a return, propagate it
            if (ctx && ctx->has_returned)
            {
                result = ctx->return_value;
                MEM_DEBUG("Block propagating return value: %d", result.value.i_val);
            }
            else
            {
                result = block_result;
            }
        }
        else
        {
            MEM_DEBUG("Empty block");
            result.type = TYPE_INT;
            result.value.i_val = 0;
        }

        // Restore scope
        if (new_scope)
        {
            MEM_DEBUG("Cleaning up block execution scope");
            pop_scope();
        }
        current_scope = saved_scope;

        return result;
    }

    /* sequential execution */
    case 'L':
    {
        struct scope *save_scope = current_scope;
        struct function_context *ctx = current_function();

        // Evaluate left side
        if (a->l)
        {
            result = eval(a->l);
            // If we hit a return, stop evaluation
            if (ctx && ctx->has_returned)
            {
                MEM_DEBUG("List propagating return from left side: %d", result.value.i_val);
                return result;
            }
        }

        // Evaluate right side
        if (a->r && (!ctx || !ctx->has_returned))
        {
            result = eval(a->r);
            // Handle returns from right side
            if (ctx && ctx->has_returned)
            {
                MEM_DEBUG("List propagating return from right side: %d", result.value.i_val);
            }
        }

        current_scope = save_scope;
        return result;
    }

    /* user function calls */
    case 'C':
    {
        struct ufncall *f = (struct ufncall *)a;
        struct scope *save_scope = current_scope;
        struct symbol *save_function = current_function_sym;
        struct value call_result = {0};

        MEM_DEBUG("Function call to %s", f->s->name);

        if (!f->s->func)
        {
            error("call to undefined function %s", f->s->name);
            result.type = TYPE_INT;
            result.value.i_val = 0;
            return result;
        }

        /* Create new scope for function */
        push_scope();
        struct scope *function_scope = current_scope;
        current_function_sym = f->s;

        MEM_DEBUG("Created new scope %p for function %s",
                  (void *)function_scope, f->s->name);

        /* Evaluate arguments in caller's scope */
        current_scope = save_scope;
        int arg_count = 0;
        struct value *args = evaluate_arguments(f->l, &arg_count);

        /* Switch back to function scope for parameter binding */
        current_scope = function_scope;

        /* Count parameters */
        struct symbol_list *sl = f->s->syms;
        int param_count = 0;
        struct symbol_list *temp = sl;
        while (temp)
        {
            param_count++;
            temp = temp->next;
        }

        MEM_DEBUG("Binding %d arguments to parameters", arg_count);

        if (arg_count != param_count)
        {
            error("argument count mismatch: expected %d, got %d",
                  param_count, arg_count);
        }

        /* Bind parameters to function scope */
        sl = f->s->syms; // Reset to start of parameter list
        for (int i = 0; i < arg_count && sl; i++)
        {
            MEM_DEBUG("Binding parameter %d (%s) with value: %d",
                      i, sl->sym->name, args[i].value.i_val);

            struct symbol *param = malloc(sizeof(struct symbol));
            if (!param)
            {
                error("out of space");
                exit(0);
            }

            param->name = strdup(sl->sym->name);
            param->type = sl->sym->type;
            param->func = NULL;
            param->syms = NULL;

            /* Initialize with argument value */
            convert_value(&param->value, param->type, &args[i].value, args[i].type);

            /* Add parameter to function scope */
            struct symbol_table *st = malloc(sizeof(struct symbol_table));
            if (!st)
            {
                free(param->name);
                free(param);
                error("out of space");
                exit(0);
            }

            st->sym = param;
            st->next = current_scope->symbols;
            current_scope->symbols = st;

            sl = sl->next;
        }

        if (args)
        {
            free(args);
        }

        /* Create new function context and push it */
        struct function_context *new_ctx = &function_stack[function_depth];
        new_ctx->function = f->s;
        new_ctx->caller_scope = save_scope;
        new_ctx->has_returned = 0;
        memset(&new_ctx->return_value, 0, sizeof(struct value));

        push_function(f->s, save_scope);

        /* Execute function body */
        call_result = eval(f->s->func);

        /* Get final result */
        struct function_context *ctx = current_function();
        if (ctx && ctx->has_returned)
        {
            result = ctx->return_value;
            MEM_DEBUG("Using function's explicit return value: %d", result.value.i_val);
        }
        else
        {
            result = call_result;
            MEM_DEBUG("Using function's default return value: %d", result.value.i_val);
        }

        MEM_DEBUG("Function %s evaluated with result: %d",
                  f->s->name, result.value.i_val);

        /* Restore state */
        pop_function();
        pop_scope();
        current_scope = save_scope;
        current_function_sym = save_function;

        return result;
    }

    default:
        result.type = TYPE_INT;
        result.value.i_val = 0;
        break;
    }

    current_scope = entry_scope;
    return result;
}

/* ========================================================================== */
/*                              Error Handling                                */
/* ========================================================================== */

void error(char *s, ...)
{
    va_list ap;
    va_start(ap, s);
    fprintf(stderr, "%d: error: ", yylineno);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    error_state = 1;
    longjmp(error_jmp, 1); // Jump back to error handler in main
}

void cleanup_and_exit(int status)
{

    // Only clean up if we're not already in an error state
    if (!error_state)
    {
        // Clean up root AST if it exists
        if (root)
        {
            treefree(root);
            root = NULL;
        }

        // Clean up all scopes
        while (current_scope)
        {
            struct scope *old = current_scope;
            current_scope = current_scope->parent;

            // Clean up symbols in this scope
            struct symbol_table *st = old->symbols;
            while (st)
            {
                struct symbol_table *next = st->next;
                if (st->sym)
                {
                    if (st->sym->name)
                        free(st->sym->name);
                    if (st->sym->func)
                        treefree(st->sym->func);
                    if (st->sym->syms)
                        symlistfree(st->sym->syms);
                    free(st->sym);
                }
                free(st);
                st = next;
            }

            free(old);
        }
    }

    exit(status);
}

/* ========================================================================== */
/*                              Debugging Utilities                           */
/* ========================================================================== */

void dumpast(struct ast *a, int level)
{
    printf("%*s", 2 * level, "");
    level++;

    if (!a)
    {
        printf("NULL\n");
        return;
    }

    switch (a->nodetype)
    {
    case 'T':
    {
        printf("cast to ");
        switch (((struct typecast *)a)->type)
        {
        case TYPE_INT:
            printf("int\n");
            break;
        case TYPE_FLOAT:
            printf("float\n");
            break;
        case TYPE_DOUBLE:
            printf("double\n");
            break;
        }
        dumpast(((struct typecast *)a)->operand, level);
        return;
    }

    case 'D':
    {
        struct symbol *s = (struct symbol *)a->l;
        printf("Declaration of %s as ", s->name);
        switch (s->type)
        {
        case TYPE_INT:
            printf("INT\n");
            break;
        case TYPE_FLOAT:
            printf("FLOAT\n");
            break;
        case TYPE_DOUBLE:
            printf("DOUBLE\n");
            break;
        }
        break;
    }

    case 'K':
    {
        struct numval *n = (struct numval *)a;
        switch (n->type)
        {
        case TYPE_INT:
            printf("number(int) %d\n", n->value.i_val);
            break;
        case TYPE_FLOAT:
            printf("number(float) %f\n", n->value.f_val);
            break;
        case TYPE_DOUBLE:
            printf("number(double) %g\n", n->value.d_val);
            break;
        }
        break;
    }

    case 'N':
        printf("ref %s\n", ((struct symref *)a)->s->name);
        break;

    case '=':
        printf("= %s\n", ((struct symasgn *)a)->s->name);
        dumpast(((struct symasgn *)a)->v, level);
        return;

    case '+':
    case '-':
    case '*':
    case '/':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case 'L':
        printf("binop %c\n", a->nodetype);
        dumpast(a->l, level);
        dumpast(a->r, level);
        return;

    case 'B':
    {
        struct block *b = (struct block *)a;
        printf("block (scope: %p)\n", (void *)b->block_scope);
        if (b->statements)
        {
            dumpast(b->statements, level);
        }
        else
        {
            printf("%*sempty block\n", 2 * level, "");
        }
        return;
    }

    case '|':
    case 'M':
        printf("unop %c\n", a->nodetype);
        dumpast(a->l, level);
        return;

    case 'I':
    case 'W':
        printf("flow %c\n", a->nodetype);
        dumpast(((struct flow *)a)->cond, level);
        if (((struct flow *)a)->tl)
            dumpast(((struct flow *)a)->tl, level);
        if (((struct flow *)a)->el)
            dumpast(((struct flow *)a)->el, level);
        return;

    case 'F':
        printf("builtin %d\n", ((struct fncall *)a)->functype);
        dumpast(a->l, level);
        return;

    case 'S':
    {
        struct strast *s = (struct strast *)a;
        printf("string \"%s\"\n", s->string);
        if (s->next)
            dumpast(s->next, level);
        return;
    }

    case 'C':
        printf("call %s\n", ((struct ufncall *)a)->s->name);
        dumpast(a->l, level);
        return;

    default:
        printf("bad %c\n", a->nodetype);
        return;
    }
}

/* ========================================================================== */
/*                              String Processing                             */
/* ========================================================================== */

char *process_string_literal(char *str)
{
    char *processed = malloc(strlen(str) + 1);
    if (!processed)
    {
        error("out of memory");
        return NULL;
    }

    char *out = processed;
    str++; // Skip opening quote

    while (*str && str[0] != '"')
    {
        if (str[0] == '\\')
        {
            switch (str[1])
            {
            case 'n':
                *out++ = '\n';
                str += 2;
                break;
            case 't':
                *out++ = '\t';
                str += 2;
                break;
            case 'r':
                *out++ = '\r';
                str += 2;
                break;
            case '\\':
                *out++ = '\\';
                str += 2;
                break;
            case '"':
                *out++ = '"';
                str += 2;
                break;
            default:
                *out++ = *str++;
                break;
            }
        }
        else
        {
            *out++ = *str++;
        }
    }
    *out = '\0';

    return processed;
}

/* ========================================================================== */
/*                              Program Initialization                        */
/* ========================================================================== */

void init_scope_system(void)
{
    current_scope = NULL;
    current_type = NO_TYPE;
    struct scope *global_scope = push_scope();
}

int run_script(const char *filename)
{
    printf("Running script: %s\n", filename);

    // Store previous input file
    FILE *prev_yyin = yyin;

    // Try to open the new file
    yyin = fopen(filename, "r");
    if (!yyin)
    {
        printf("Error: Could not open file %s\n", filename);
        return 1;
    }

    // Save interactive mode state
    int prev_interactive = interactive_mode;
    interactive_mode = 0;

    // Reset lexer state for new file
    yyrestart(yyin);

    // Parse the file
    int result = yyparse();

    // Only treat as error if it's not EOF
    if (result != 0 && !feof(yyin))
    {
        printf("Error parsing file %s\n", filename);
        fclose(yyin);
        yyin = prev_yyin;
        interactive_mode = prev_interactive;
        return 1;
    }

    // Clean up and restore previous state
    fclose(yyin);
    yyin = prev_yyin;
    yyrestart(yyin);
    interactive_mode = prev_interactive;

    return 0;
}

int main(void)
{
    init_scope_system();
    yyin = stdin;

    // Set up error handling
    if (setjmp(error_jmp) != 0)
    {
        // We got here from an error - always exit
        cleanup_and_exit(1);
    }

    if (interactive_mode)
    {
        printf("> ");
    }
    return yyparse();
}
