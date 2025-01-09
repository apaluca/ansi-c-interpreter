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

static void debug_print_symbol_table(const char *msg)
{
    printf("\nDEBUG: %s\n", msg);
    printf("DEBUG: Current scope at %p\n", (void *)current_scope);
    if (!current_scope)
    {
        printf("DEBUG: No current scope!\n");
        return;
    }

    struct symbol_table *st = current_scope->symbols;
    printf("DEBUG: Symbol table entries:\n");
    while (st)
    {
        if (st->sym)
        {
            printf("DEBUG:   Symbol: %s (type: %d) at %p\n",
                   st->sym->name ? st->sym->name : "NULL",
                   st->sym->type,
                   (void *)st->sym);
        }
        else
        {
            printf("DEBUG:   NULL symbol entry\n");
        }
        st = st->next;
    }
    printf("\n");
}

struct scope *push_scope(void)
{
    printf("DEBUG: Current scope before push: %p\n", (void *)current_scope);

    struct scope *new_scope = malloc(sizeof(struct scope));
    if (!new_scope)
    {
        error("out of space");
        exit(0);
    }

    new_scope->symbols = NULL;
    new_scope->parent = current_scope;
    current_scope = new_scope;

    printf("DEBUG: New scope created at %p\n", (void *)new_scope);
    printf("DEBUG: Parent scope: %p\n", (void *)new_scope->parent);

    return new_scope;
}

void pop_scope(void)
{
    printf("\nDEBUG: pop_scope() - Beginning scope pop\n");
    if (!current_scope)
    {
        printf("DEBUG: pop_scope() - No scope to pop!\n");
        return;
    }

    struct scope *scope_to_pop = current_scope;
    struct scope *parent_scope = current_scope->parent;

    printf("DEBUG: pop_scope() - Popping scope %p (parent: %p)\n",
           (void *)scope_to_pop, (void *)parent_scope);

    // Print all symbols in the scope before removing them
    printf("DEBUG: pop_scope() - Symbols in scope being popped:\n");
    struct symbol_table *st = scope_to_pop->symbols;
    while (st)
    {
        if (st->sym && st->sym->name)
        {
            printf("DEBUG: pop_scope() - Found symbol '%s'\n", st->sym->name);

            // Check if symbol exists in parent scope before freeing
            if (parent_scope)
            {
                struct scope *save_scope = current_scope;
                current_scope = parent_scope;
                struct symbol *parent_sym = lookup_all_scopes(st->sym->name);
                current_scope = save_scope;

                if (!parent_sym)
                {
                    printf("DEBUG: pop_scope() - Freeing symbol '%s'\n", st->sym->name);
                    free(st->sym->name);
                    free(st->sym);
                }
                else
                {
                    printf("DEBUG: pop_scope() - Symbol '%s' exists in parent scope, not freeing\n",
                           st->sym->name);
                }
            }
            else
            {
                printf("DEBUG: pop_scope() - No parent scope, freeing symbol '%s'\n",
                       st->sym->name);
                free(st->sym->name);
                free(st->sym);
            }
        }

        struct symbol_table *next = st->next;
        free(st);
        st = next;
    }

    free(scope_to_pop);
    current_scope = parent_scope;

    printf("DEBUG: pop_scope() - Scope pop complete, current scope is now %p\n",
           (void *)current_scope);
}

struct symbol *scope_lookup(char *name)
{
    printf("\nDEBUG: scope_lookup('%s') in scope %p\n",
           name ? name : "NULL",
           (void *)current_scope);

    if (!current_scope)
    {
        printf("DEBUG: scope_lookup - no current scope\n");
        return NULL;
    }

    if (!name)
    {
        printf("DEBUG: scope_lookup - null name provided\n");
        return NULL;
    }

    struct symbol_table *st = current_scope->symbols;
    while (st)
    {
        printf("DEBUG: checking symbol table entry %p\n", (void *)st);
        if (!st->sym)
        {
            printf("DEBUG: null symbol in table\n");
            st = st->next;
            continue;
        }

        if (!st->sym->name)
        {
            printf("DEBUG: symbol has null name\n");
            st = st->next;
            continue;
        }

        printf("DEBUG: comparing '%s' with '%s'\n", name, st->sym->name);
        if (strcmp(st->sym->name, name) == 0)
        {
            printf("DEBUG: found match!\n");
            return st->sym;
        }
        st = st->next;
    }

    printf("DEBUG: no match found\n");
    return NULL;
}

struct symbol *lookup_all_scopes(char *name)
{
    printf("\nDEBUG: lookup_all_scopes('%s') started\n", name);
    printf("DEBUG: lookup_all_scopes() - Current scope: %p\n", (void *)current_scope);
    printf("DEBUG: lookup_all_scopes() - Function depth: %d\n", function_depth);

    // First check current scope chain
    struct scope *s = current_scope;
    int level = 0;

    while (s)
    {
        printf("DEBUG: lookup_all_scopes() - Checking scope level %d at %p\n",
               level, (void *)s);

        struct symbol_table *st = s->symbols;
        while (st)
        {
            if (st->sym && st->sym->name)
            {
                printf("DEBUG: lookup_all_scopes() - Checking symbol '%s'\n",
                       st->sym->name);
                if (strcmp(st->sym->name, name) == 0)
                {
                    printf("DEBUG: lookup_all_scopes() - Found '%s' in scope %p\n",
                           name, (void *)s);
                    return st->sym;
                }
            }
            st = st->next;
        }
        s = s->parent;
        level++;
    }

    // If not found and we're in a function, check caller's scope chain
    if (function_depth > 0)
    {
        struct function_context *ctx = &function_stack[function_depth - 1];
        printf("DEBUG: lookup_all_scopes() - Checking caller's scope chain from %p\n",
               (void *)ctx->caller_scope);

        s = ctx->caller_scope;
        level = 0;

        while (s)
        {
            printf("DEBUG: lookup_all_scopes() - Checking caller scope level %d at %p\n",
                   level, (void *)s);

            struct symbol_table *st = s->symbols;
            while (st)
            {
                if (st->sym && st->sym->name)
                {
                    printf("DEBUG: lookup_all_scopes() - Checking symbol '%s'\n",
                           st->sym->name);
                    if (strcmp(st->sym->name, name) == 0)
                    {
                        printf("DEBUG: lookup_all_scopes() - Found '%s' in caller scope %p\n",
                               name, (void *)s);
                        return st->sym;
                    }
                }
                st = st->next;
            }
            s = s->parent;
            level++;
        }
    }

    printf("DEBUG: lookup_all_scopes() - Symbol '%s' not found in any scope\n", name);
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
    printf("\nDEBUG: lookup('%s') started\n", sym);

    struct symbol *found = lookup_all_scopes(sym);
    if (!found && current_type == NO_TYPE)
    {
        error("use of undeclared identifier '%s'", sym);
        // Will not return - error() exits
    }

    return found;
}

/* ========================================================================== */
/*                              AST Building                                  */
/* ========================================================================== */

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
    struct strval *sv = malloc(sizeof(struct strval));
    if (!sv)
    {
        error("out of space");
        exit(0);
    }
    sv->nodetype = 'S';
    sv->str = s;
    return (struct ast *)sv;
}

struct ast *newdecl(struct symbol *s)
{
    printf("\nDEBUG: Processing declaration for %s\n", s->name);

    /* Check for redefinition */
    struct symbol *existing = scope_lookup(s->name);
    if (existing)
    {
        printf("DEBUG: Found existing declaration of '%s'\n", s->name);
        // Free the duplicate symbol we were trying to declare
        if (s->name)
            free(s->name);
        if (s->func)
            treefree(s->func);
        if (s->syms)
            symlistfree(s->syms);
        free(s);
        error("redefinition of '%s'", existing->name);
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

    st->sym = s;
    st->next = current_scope->symbols;
    current_scope->symbols = st;

    printf("DEBUG: Added symbol '%s' to scope %p\n", s->name, (void *)current_scope);
    printf("DEBUG: Symbol type is %d\n", s->type);

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

    printf("DEBUG: Successfully created declaration node\n");
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
    struct block *b = malloc(sizeof(struct block));
    if (!b)
    {
        error("out of space");
        return NULL;
    }
    b->nodetype = 'B';
    b->statements = statements;
    b->block_scope = scope;

    // Set result type based on the last statement in the block
    if (statements)
    {
        b->result_type = statements->result_type;
    }
    else
    {
        b->result_type = TYPE_INT; // Default type for empty blocks
    }

    printf("DEBUG: newblock() - Created block node with scope %p\n", (void *)scope);
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
    printf("\nDEBUG: newref('%s')\n", s->name);
    struct symref *a = malloc(sizeof(struct symref));
    if (!a)
    {
        error("out of space");
        exit(0);
    }

    // Always look up the symbol in all accessible scopes
    printf("DEBUG: Looking up symbol '%s' in all scopes\n", s->name);
    struct symbol *found = lookup_all_scopes(s->name);
    printf("DEBUG: lookup_all_scopes result: %p\n", (void *)found);

    if (found)
    {
        printf("DEBUG: Found symbol '%s' with type %d\n", found->name, found->type);
        a->s = found;
    }
    else if (current_type != NO_TYPE)
    {
        printf("DEBUG: Using new symbol '%s' with type %d\n", s->name, s->type);
        a->s = s;
    }
    else
    {
        error("undefined variable '%s'", s->name);
        free(a);
        return NULL;
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

/* ========================================================================== */
/*                              AST Management                                */
/* ========================================================================== */

void treefree(struct ast *a)
{
    if (!a)
        return;

    // Add a guard against invalid node types
    if (a->nodetype < 0 || (a->nodetype > 'z' && a->nodetype < 256))
    {
        printf("DEBUG: Skipping invalid node type %d\n", a->nodetype);
        return;
    }

    printf("DEBUG: Freeing node type %c\n", a->nodetype);

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
            printf("DEBUG: Freeing right subtree of %c\n", a->nodetype);
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
            printf("DEBUG: Freeing left subtree of %c\n", a->nodetype);
            treefree(a->l);
        }
        break;

    /* no subtree */
    case 'D':
    case 'K':
    case 'N':
        printf("DEBUG: No subtrees to free for %c\n", a->nodetype);
        break;

    case '=':
        if (((struct symasgn *)a)->v)
        {
            printf("DEBUG: Freeing assignment value\n");
            treefree(((struct symasgn *)a)->v);
        }
        break;

    case 'I':
    case 'W':
        printf("DEBUG: Freeing flow node parts\n");
        if (((struct flow *)a)->cond)
            treefree(((struct flow *)a)->cond);
        if (((struct flow *)a)->tl)
            treefree(((struct flow *)a)->tl);
        if (((struct flow *)a)->el)
            treefree(((struct flow *)a)->el);
        break;

    case 'S':
        printf("DEBUG: Freeing string\n");
        if (((struct strval *)a)->str)
            free(((struct strval *)a)->str);
        break;

    case 'T':
        printf("DEBUG: Freeing typecast\n");
        if (((struct typecast *)a)->operand)
            treefree(((struct typecast *)a)->operand);
        break;

    case 'B':
    {
        struct block *b = (struct block *)a;
        printf("DEBUG: treefree() - Freeing block node with scope %p\n",
               (void *)b->block_scope);
        if (b->statements)
        {
            treefree(b->statements);
        }
        // Note: Don't free b->block_scope here as it's managed by pop_scope()
        break;
    }

    default:
        printf("DEBUG: Unknown node type %c, skipping\n", a->nodetype);
        return; // Don't free unknown node types
    }

    printf("DEBUG: Freeing node itself\n");
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
    if (name->syms)
        symlistfree(name->syms);
    if (name->func)
        treefree(name->func);
    name->syms = reverse_symbol_list(syms);
    name->func = func;
}

void push_function(struct symbol *func, struct scope *caller_scope)
{
    printf("\nDEBUG: push_function() - Starting with function '%s'\n",
           func ? func->name : "NULL");
    printf("DEBUG: push_function() - Caller scope: %p\n", (void *)caller_scope);
    printf("DEBUG: push_function() - Current depth: %d\n", function_depth);

    if (function_depth >= MAX_FUNCTION_DEPTH)
    {
        error("maximum function call depth exceeded");
        return;
    }
    function_stack[function_depth].function = func;
    function_stack[function_depth].caller_scope = caller_scope;
    function_depth++;

    printf("DEBUG: push_function() - New depth: %d\n", function_depth);
}

struct function_context *pop_function(void)
{
    printf("\nDEBUG: pop_function() - Starting\n");
    printf("DEBUG: pop_function() - Current depth: %d\n", function_depth);

    if (function_depth <= 0)
    {
        printf("DEBUG: pop_function() - No function context to pop\n");
        return NULL;
    }

    function_depth--;
    printf("DEBUG: pop_function() - Popped function, new depth: %d\n", function_depth);

    if (function_depth > 0)
    {
        printf("DEBUG: pop_function() - Returning to caller scope: %p\n",
               (void *)function_stack[function_depth - 1].caller_scope);
    }

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
    printf("\nDEBUG: eval_function_body() - Starting for function '%s'\n",
           func ? func->name : "NULL");
    printf("DEBUG: eval_function_body() - Current scope: %p\n",
           (void *)current_scope);

    struct value result;
    struct scope *caller_scope = current_scope;

    // Push function with caller's scope
    push_function(func, caller_scope);

    if (body)
    {
        printf("DEBUG: eval_function_body() - Evaluating function body\n");
        result = eval(body);
        printf("DEBUG: eval_function_body() - Body evaluation complete\n");
    }
    else
    {
        printf("DEBUG: eval_function_body() - Empty body, returning default value\n");
        /* Default return value is 0 */
        result.type = func->type;
        result.value.i_val = 0;
    }

    printf("DEBUG: eval_function_body() - Popping function context\n");
    pop_function();

    printf("DEBUG: eval_function_body() - Function execution complete\n");
    return result;
}

/* ========================================================================== */
/*                              Built-in Functions                            */
/* ========================================================================== */

static struct value eval_printf(const char *format, struct ast *args)
{
    struct value result = {.type = TYPE_INT, .value.i_val = 0};
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

static struct value eval_scanf(const char *format, struct ast *args)
{
    struct value result = {.type = TYPE_INT, .value.i_val = 0};

    // Input validation
    if (!args || args->nodetype != 'N')
    {
        error("scanf requires a variable reference");
        return result;
    }

    struct symref *ref = (struct symref *)args;
    int items_read = 0;

    switch (format[1])
    {
    case 'd':
        if (ref->s->type != TYPE_INT)
        {
            error("format %%d requires int variable");
        }
        else
        {
            items_read = scanf("%d", &ref->s->value.i_val);
        }
        break;

    case 'f':
        if (ref->s->type == TYPE_FLOAT)
        {
            items_read = scanf("%f", &ref->s->value.f_val);
        }
        else if (ref->s->type == TYPE_DOUBLE)
        {
            items_read = scanf("%lf", &ref->s->value.d_val);
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

    // Clear input buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;

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

    printf("\nDEBUG: eval() - Entering with scope %p\n", (void *)current_scope);

    if (!a)
    {
        printf("DEBUG: eval() - null AST node\n");
        error("internal error, null eval");
        result.type = TYPE_INT;
        result.value.i_val = 0;
        return result;
    }

    printf("DEBUG: eval() - Processing node type '%c'\n", a->nodetype);

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
            /* Convert to function's return type if needed */
            if (result.type != current_function_sym->type)
            {
                struct value temp = result;
                result.type = current_function_sym->type;
                convert_value(&result.value, result.type, &temp.value, temp.type);
            }
        }
        else
        {
            result.type = current_function_sym->type;
            result.value.i_val = 0;
        }
        return result;
    }

    /* declaration */
    case 'D':
    {
        printf("DEBUG: eval() - Processing declaration in scope %p\n", (void *)current_scope);
        struct symbol *s = (struct symbol *)a->l;
        printf("DEBUG: eval() - Declaring symbol '%s' of type %d\n", s->name, s->type);

        result.type = s->type;
        switch (s->type)
        {
        case TYPE_INT:
            result.value.i_val = 0;
            break;
        case TYPE_FLOAT:
            result.value.f_val = 0.0f;
            break;
        case TYPE_DOUBLE:
            result.value.d_val = 0.0;
            break;
        }
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
        printf("DEBUG: eval() - Variable reference to '%s'\n", s->s->name);

        struct symbol *sym = lookup_all_scopes(s->s->name);
        if (!sym)
        {
            error("undefined variable '%s'", s->s->name);
            break;
        }

        result.type = sym->type;
        result.value = sym->value;
        printf("DEBUG: eval() - Found value for '%s' in scope %p\n",
               sym->name, (void *)current_scope);
        break;
    }

    /* assignment */
    case '=':
    {
        struct symasgn *sa = (struct symasgn *)a;
        printf("\nDEBUG: eval() - Assignment to '%s'\n", sa->s->name);
        printf("DEBUG: eval() - Current scope during assignment: %p\n", (void *)current_scope);

        // Evaluate right-hand side first
        struct value rhs = eval(sa->v);

        // Look up the symbol in all accessible scopes
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
        printf("DEBUG: eval() - Successfully assigned to '%s' in scope %p\n",
               sym->name, (void *)current_scope);

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

        char *format = NULL;
        struct ast *args = NULL;

        if (f->l->nodetype == 'S')
        {
            format = ((struct strval *)f->l)->str;
        }
        else if (f->l->nodetype == 'L' && f->l->l->nodetype == 'S')
        {
            format = ((struct strval *)f->l->l)->str;
            args = f->l->r;
        }
        else
        {
            error("Invalid arguments to built-in function");
            result.type = TYPE_INT;
            result.value.i_val = 0;
            return result;
        }

        switch (f->functype)
        {
        case B_printf:
            return eval_printf(format, args);
        case B_scanf:
            return eval_scanf(format, args);
        }
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
            while ((v1 = eval(((struct flow *)a)->cond)).value.i_val)
            {
                result = eval(((struct flow *)a)->tl);
            }
        }
        return result;
    }

    /* block evaluation */
    case 'B':
    {
        struct block *b = (struct block *)a;
        printf("\nDEBUG: eval() - Block evaluation starting\n");
        printf("DEBUG: eval() - Current scope before block: %p\n", (void *)current_scope);
        printf("DEBUG: eval() - Block scope: %p\n", (void *)b->block_scope);

        // Evaluate the block's statements in the block's scope
        if (b->statements)
        {
            result = eval(b->statements);
        }
        else
        {
            // Empty block returns 0
            result.type = TYPE_INT;
            result.value.i_val = 0;
        }

        // Pop the block's scope and record the parent scope
        printf("DEBUG: eval() - Block evaluation complete, popping scope %p\n",
               (void *)b->block_scope);

        struct scope *parent_scope = b->block_scope->parent;
        pop_scope();

        printf("DEBUG: eval() - Restored to parent scope %p\n", (void *)parent_scope);
        current_scope = parent_scope;

        return result;
    }

    /* sequential execution */
    case 'L':
    {
        printf("DEBUG: eval() - Sequential execution block\n");
        printf("DEBUG: eval() - Current scope before left eval: %p\n", (void *)current_scope);

        struct scope *save_scope = current_scope;

        if (a->l)
        {
            result = eval(a->l);
            current_scope = save_scope; // Restore after left evaluation
        }

        printf("DEBUG: eval() - Current scope between L/R eval: %p\n", (void *)current_scope);

        if (a->r)
        {
            result = eval(a->r);
            current_scope = save_scope; // Restore after right evaluation
        }

        printf("DEBUG: eval() - Current scope after right eval: %p\n", (void *)current_scope);
        break;
    }

    /* user function calls */
    case 'C':
    {
        struct ufncall *f = (struct ufncall *)a;
        struct scope *save_scope = current_scope;
        struct symbol *save_function = current_function_sym;

        printf("\nDEBUG: eval() - Function call to '%s'\n", f->s->name);
        printf("DEBUG: eval() - Current scope before call: %p\n", (void *)current_scope);
        printf("DEBUG: eval() - Current function depth: %d\n", function_depth);

        if (!f->s->func)
        {
            error("call to undefined function %s", f->s->name);
            result.type = TYPE_INT;
            result.value.i_val = 0;
            return result;
        }

        /* Create new scope for function */
        printf("DEBUG: eval() - Creating new scope for function\n");
        push_scope();
        struct scope *function_scope = current_scope;
        current_function_sym = f->s;

        /* Save the caller's scope */
        printf("DEBUG: eval() - Saving caller's scope %p\n", (void *)save_scope);
        push_function(f->s, save_scope);

        /* Evaluate arguments in caller's scope */
        printf("DEBUG: eval() - Switching to caller's scope for argument evaluation\n");
        current_scope = save_scope;
        int arg_count = 0;
        struct value *args = evaluate_arguments(f->l, &arg_count);
        printf("DEBUG: eval() - Evaluated %d arguments in caller's scope\n", arg_count);

        /* Switch back to function scope for parameter binding */
        printf("DEBUG: eval() - Switching back to function scope %p\n", (void *)function_scope);
        current_scope = function_scope;

        /* Bind parameters to function scope */
        struct symbol_list *sl = f->s->syms; // Original parameter list from function def
        for (int i = 0; i < arg_count && sl; i++)
        {
            printf("DEBUG: eval() - Binding parameter '%s'\n", sl->sym->name);

            struct symbol *param = malloc(sizeof(struct symbol));
            if (!param)
            {
                error("out of space");
                exit(0);
            }

            /* Use original parameter name from function definition */
            param->name = strdup(sl->sym->name); // Important: use original name
            param->type = sl->sym->type;

            /* Convert argument value to parameter type */
            convert_value(&param->value, param->type, &args[i].value, args[i].type);

            printf("DEBUG: eval() - Created parameter symbol '%s' with type %d\n",
                   param->name, param->type);

            /* Add parameter to function scope */
            struct symbol_table *st = malloc(sizeof(struct symbol_table));
            if (!st)
            {
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

        /* Evaluate function body */
        printf("DEBUG: eval() - Starting function body evaluation\n");
        result = eval(f->s->func);
        printf("DEBUG: eval() - Completed function body evaluation\n");

        /* Restore state */
        printf("DEBUG: eval() - Cleaning up function call state\n");
        printf("DEBUG: eval() - Popping function scope %p\n", (void *)current_scope);
        pop_scope();
        pop_function();

        printf("DEBUG: eval() - Restoring caller scope %p\n", (void *)save_scope);
        current_scope = save_scope;
        current_function_sym = save_function;

        printf("DEBUG: eval() - Function call complete, returned to scope %p\n",
               (void *)current_scope);
        return result;
    }

    default:
        printf("DEBUG: eval() - Unhandled node type '%c'\n", a->nodetype);
        result.type = TYPE_INT;
        result.value.i_val = 0;
        break;
    }

    printf("DEBUG: eval() - Restoring scope from %p to %p\n",
           (void *)current_scope, (void *)entry_scope);
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
    printf("DEBUG: Starting cleanup\n");

    // Only clean up if we're not already in an error state
    if (!error_state)
    {
        // Clean up root AST if it exists
        if (root)
        {
            printf("DEBUG: Cleaning up root AST\n");
            treefree(root);
            root = NULL;
        }

        // Clean up all scopes
        while (current_scope)
        {
            printf("DEBUG: Cleaning up scope %p\n", (void *)current_scope);
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

    printf("DEBUG: Cleanup complete, exiting with status %d\n", status);
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
    char *out = processed;

    // Skip opening quote
    str++;

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
/*                              Symbol List Management                        */
/* ========================================================================== */

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
/*                              Program Initialization                        */
/* ========================================================================== */

void init_scope_system(void)
{
    printf("\nDEBUG: Initializing scope system\n");
    current_scope = NULL;
    current_type = NO_TYPE;
    struct scope *global_scope = push_scope();
    printf("DEBUG: Created global scope at %p\n", (void *)global_scope);
    debug_print_symbol_table("Initial symbol table state");
}

int run_script(const char *filename)
{
    FILE *prev_yyin = yyin; // Save current input
    yyin = fopen(filename, "r");

    if (!yyin)
    {
        return 1; // Error opening file
    }

    // Save interactive mode state and temporarily disable it
    int prev_interactive = interactive_mode;
    interactive_mode = 0;

    // Parse the file
    int result = yyparse();

    // Restore previous state
    fclose(yyin);
    yyin = prev_yyin;
    interactive_mode = prev_interactive;

    return result;
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