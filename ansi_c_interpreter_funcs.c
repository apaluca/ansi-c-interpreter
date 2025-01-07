#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "ansi_c_interpreter.h"

/* Global variables */
int debug = 1; /* debug flag */
struct ast *root = NULL;
enum value_type current_type = NO_TYPE;
struct scope *current_scope = NULL;
struct function_context function_stack[MAX_FUNCTION_DEPTH];
int function_depth = 0;
static struct symbol *current_function_sym = NULL;

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
    return new_scope;
}

void pop_scope(void)
{
    if (!current_scope)
        return;

    // Free all symbols in current scope
    struct symbol_table *st = current_scope->symbols;
    while (st)
    {
        struct symbol_table *next = st->next;
        // Don't free the symbol itself as it might be referenced elsewhere
        free(st);
        st = next;
    }

    struct scope *old = current_scope;
    current_scope = current_scope->parent;
    free(old);
}

struct symbol *scope_lookup(char *name)
{
    if (!current_scope)
    {
        return NULL;
    }

    struct symbol_table *st = current_scope->symbols;
    while (st)
    {
        if (strcmp(st->sym->name, name) == 0)
        {
            printf("DEBUG: Found %s in current scope\n", name);
            return st->sym;
        }
        st = st->next;
    }

    printf("DEBUG: %s not found in current scope\n", name);
    return NULL;
}

struct symbol *lookup_all_scopes(char *name)
{
    struct scope *s = current_scope;
    printf("DEBUG: Searching all scopes for: %s\n", name);

    while (s && s->parent)
    { // Only search parent scopes
        s = s->parent;
        struct symbol_table *st = s->symbols;
        while (st)
        {
            if (strcmp(st->sym->name, name) == 0)
            {
                printf("DEBUG: Found %s in outer scope\n", name);
                return st->sym;
            }
            st = st->next;
        }
    }
    return NULL;
}

/* Type system helper functions */
void settype(struct symbol *sym, enum value_type type)
{
    printf("DEBUG: Setting type for symbol %s\n", sym->name);
    // Only set type if it hasn't been set before
    if (sym->type != type)
    {
        printf("DEBUG: Changing type from %d to %d\n", sym->type, type);
        sym->type = type;
        // Initialize value to 0 of the appropriate type
        switch (type)
        {
        case TYPE_INT:
            printf("DEBUG: Initializing as INT\n");
            sym->value.i_val = 0;
            break;
        case TYPE_FLOAT:
            printf("DEBUG: Initializing as FLOAT\n");
            sym->value.f_val = 0.0f;
            break;
        case TYPE_DOUBLE:
            printf("DEBUG: Initializing as DOUBLE\n");
            sym->value.d_val = 0.0;
            break;
        }
    }
    else
    {
        printf("DEBUG: Type unchanged\n");
    }
}

enum value_type promote_types(enum value_type t1, enum value_type t2)
{
    if (t1 == TYPE_DOUBLE || t2 == TYPE_DOUBLE)
        return TYPE_DOUBLE;
    if (t1 == TYPE_FLOAT || t2 == TYPE_FLOAT)
        return TYPE_FLOAT;
    return TYPE_INT;
}

void convert_value(void *dest, enum value_type dest_type, void *src, enum value_type src_type)
{
    printf("DEBUG: Converting from type %d to type %d\n", src_type, dest_type);
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
            printf("DEBUG: Converted int %d to float %f\n", s->i_val, d->f_val);
            break;
        case TYPE_DOUBLE:
            d->d_val = (double)s->i_val;
            printf("DEBUG: Converted int %d to double %g\n", s->i_val, d->d_val);
            break;
        }
        break;
    case TYPE_FLOAT:
        switch (dest_type)
        {
        case TYPE_INT:
            d->i_val = (int)s->f_val;
            printf("DEBUG: Converted float %f to int %d\n", s->f_val, d->i_val);
            break;
        case TYPE_DOUBLE:
            d->d_val = (double)s->f_val;
            printf("DEBUG: Converted float %f to double %g\n", s->f_val, d->d_val);
            break;
        }
        break;
    case TYPE_DOUBLE:
        switch (dest_type)
        {
        case TYPE_INT:
            d->i_val = (int)s->d_val;
            printf("DEBUG: Converted double %g to int %d\n", s->d_val, d->i_val);
            break;
        case TYPE_FLOAT:
            d->f_val = (float)s->d_val;
            printf("DEBUG: Converted double %g to float %f\n", s->d_val, d->f_val);
            break;
        }
        break;
    }
}

struct symbol *lookup_function(char *name)
{
    struct scope *s = current_scope;
    printf("DEBUG: Looking up function: %s\n", name);

    while (s)
    {
        struct symbol_table *st = s->symbols;
        while (st)
        {
            if (strcmp(st->sym->name, name) == 0)
            {
                // Check if it's actually a function (has func pointer)
                if (st->sym->func != NULL)
                {
                    printf("DEBUG: Found function %s\n", name);
                    return st->sym;
                }
            }
            st = st->next;
        }
        s = s->parent;
    }
    printf("DEBUG: Function %s not found\n", name);
    return NULL;
}

struct symbol *lookup(char *sym)
{
    printf("DEBUG: Looking up symbol: %s\n", sym);

    // Always try to find in current scope first
    struct symbol *current = scope_lookup(sym);
    if (current)
    {
        printf("DEBUG: Found symbol %s in current scope\n", sym);
        return current;
    }

    // For variable references (not declarations), search outer scopes
    if (current_type == NO_TYPE)
    {
        struct symbol *found = lookup_all_scopes(sym);
        if (found)
        {
            printf("DEBUG: Found symbol %s in outer scope\n", sym);
            return found;
        }
        // Only error if we're not in init_declarator
        printf("DEBUG: Symbol %s not found in any scope\n", sym);
        error("reference to undefined variable %s\n", sym);
        return NULL;
    }

    // For declarations, create new symbol only if we're directly declaring it
    printf("DEBUG: Creating new symbol %s in current scope\n", sym);
    struct symbol *sp = malloc(sizeof(struct symbol));
    if (!sp)
    {
        error("out of memory");
        abort();
    }

    sp->name = strdup(sym);
    if (!sp->name)
    {
        free(sp);
        error("out of memory");
        abort();
    }

    sp->type = current_type;
    sp->func = NULL;
    sp->syms = NULL;

    // Initialize value based on type
    switch (current_type)
    {
    case TYPE_INT:
        sp->value.i_val = 0;
        break;
    case TYPE_FLOAT:
        sp->value.f_val = 0.0f;
        break;
    case TYPE_DOUBLE:
        sp->value.d_val = 0.0;
        break;
    }

    // Add to current scope
    struct symbol_table *st = malloc(sizeof(struct symbol_table));
    if (!st)
    {
        free(sp->name);
        free(sp);
        error("out of space");
        exit(0);
    }
    st->sym = sp;
    st->next = current_scope->symbols;
    current_scope->symbols = st;

    return sp;
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

/* AST building functions */
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

struct ast *newstring(char *s)
{
    struct strval *sv = malloc(sizeof(struct strval));
    if (!sv)
    {
        error("out of space");
        exit(0);
    }
    sv->nodetype = 'S';
    sv->str = s; // Takes ownership of the string
    return (struct ast *)sv;
}

struct ast *newdecl(struct symbol *s)
{
    struct ast *a = malloc(sizeof(struct ast));
    if (!a)
    {
        error("out of space");
        exit(0);
    }
    a->nodetype = 'D';      // D for declaration
    a->l = (struct ast *)s; // Store symbol pointer
    a->r = NULL;
    a->result_type = s->type; // Store declared type
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
    a->result_type = TYPE_INT; // comparisons always return int
    return a;
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
    ((struct ast *)a)->result_type = TYPE_DOUBLE; // built-in functions return double
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
    ((struct ast *)a)->result_type = s->type; // function return type
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
    a->nodetype = 'N';

    // Temporarily set current_type to NO_TYPE to force a lookup
    enum value_type saved_type = current_type;
    current_type = NO_TYPE;

    // Look up the symbol again to ensure we find it in any scope
    struct symbol *found = lookup_all_scopes(s->name);

    // Restore the original current_type
    current_type = saved_type;

    if (found)
    {
        a->s = found;
    }
    else
    {
        a->s = s;
    }

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
    printf("DEBUG: Defining function %s\n", name->name);
    if (name->syms)
        symlistfree(name->syms);
    if (name->func)
        treefree(name->func);
    name->syms = reverse_symbol_list(syms); // Reverse the parameter list
    name->func = func;
}

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

/* Push new function context */
void push_function(struct symbol *func)
{
    if (function_depth >= MAX_FUNCTION_DEPTH)
    {
        error("maximum function call depth exceeded");
        return;
    }
    function_stack[function_depth].function = func;
    function_depth++;
}

/* Pop function context */
struct function_context *pop_function(void)
{
    if (function_depth <= 0)
        return NULL;
    function_depth--;
    return &function_stack[function_depth];
}

/* Get current function context */
struct function_context *current_function(void)
{
    if (function_depth <= 0)
        return NULL;
    return &function_stack[function_depth - 1];
}

/* Helper to count and store arguments in correct order */
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
    struct value result;

    push_function(func);

    if (body)
    {
        result = eval(body);

        /* Check if we got a return value */
        struct function_context *ctx = current_function();
    }
    else
    {
        /* Default return value is 0 */
        result.type = func->type;
        result.value.i_val = 0;
    }

    pop_function();
    return result;
}

struct value eval(struct ast *a)
{
    struct value v1, v2, result;

    if (!a)
    {
        error("internal error, null eval");
        result.type = TYPE_INT;
        result.value.i_val = 0;
        return result;
    }

    switch (a->nodetype)
    {
    /* return */
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
        struct symbol *s = (struct symbol *)a->l;
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
        if (debug)
        {
            printf("Declared %s as ", s->name);
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
        }
        return result;
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
        result.type = s->s->type;
        result.value = s->s->value;
        return result;
    }

    /* assignment */
    case '=':
    {
        struct symasgn *sa = (struct symasgn *)a;
        v1 = eval(sa->v);

        // Convert assigned value to the variable's type
        convert_value(&sa->s->value, sa->s->type, &v1.value, v1.type);

        // Return the value after conversion, maintaining the variable's type
        result.type = sa->s->type;
        result.value = sa->s->value;

        // For debug output
        if (debug)
        {
            printf("Assignment to %s (type: ", sa->s->name);
            switch (sa->s->type)
            {
            case TYPE_INT:
                printf("INT");
                break;
            case TYPE_FLOAT:
                printf("FLOAT");
                break;
            case TYPE_DOUBLE:
                printf("DOUBLE");
                break;
            }
            printf(")\n");
        }

        return result;
    }

    /* expressions */
    case '+':
    case '-':
    case '*':
    case '/':
    {
        v1 = eval(a->l);
        v2 = eval(a->r);

        result.type = promote_types(v1.type, v2.type);
        union value_union temp1, temp2;

        convert_value(&temp1, result.type, &v1.value, v1.type);
        convert_value(&temp2, result.type, &v2.value, v2.type);

        switch (result.type)
        {
        case TYPE_INT:
            switch (a->nodetype)
            {
            case '+':
                result.value.i_val = temp1.i_val + temp2.i_val;
                break;
            case '-':
                result.value.i_val = temp1.i_val - temp2.i_val;
                break;
            case '*':
                result.value.i_val = temp1.i_val * temp2.i_val;
                break;
            case '/':
                if (temp2.i_val == 0)
                {
                    error("division by zero");
                    result.value.i_val = 0;
                }
                else
                {
                    result.value.i_val = temp1.i_val / temp2.i_val;
                }
                break;
            }
            break;

        case TYPE_FLOAT:
            switch (a->nodetype)
            {
            case '+':
                result.value.f_val = temp1.f_val + temp2.f_val;
                break;
            case '-':
                result.value.f_val = temp1.f_val - temp2.f_val;
                break;
            case '*':
                result.value.f_val = temp1.f_val * temp2.f_val;
                break;
            case '/':
                if (temp2.f_val == 0.0f)
                {
                    error("division by zero");
                    result.value.f_val = 0.0f;
                }
                else
                {
                    result.value.f_val = temp1.f_val / temp2.f_val;
                }
                break;
            }
            break;

        case TYPE_DOUBLE:
            switch (a->nodetype)
            {
            case '+':
                result.value.d_val = temp1.d_val + temp2.d_val;
                break;
            case '-':
                result.value.d_val = temp1.d_val - temp2.d_val;
                break;
            case '*':
                result.value.d_val = temp1.d_val * temp2.d_val;
                break;
            case '/':
                if (temp2.d_val == 0.0)
                {
                    error("division by zero");
                    result.value.d_val = 0.0;
                }
                else
                {
                    result.value.d_val = temp1.d_val / temp2.d_val;
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

    case 'M': // unary minus
    {
        printf("DEBUG: Evaluating unary minus\n");
        v1 = eval(a->l);
        result.type = v1.type;

        switch (v1.type)
        {
        case TYPE_INT:
            printf("DEBUG: Negating int value %d\n", v1.value.i_val);
            result.value.i_val = -v1.value.i_val;
            break;
        case TYPE_FLOAT:
            printf("DEBUG: Negating float value %f\n", v1.value.f_val);
            result.value.f_val = -v1.value.f_val;
            break;
        case TYPE_DOUBLE:
            printf("DEBUG: Negating double value %g\n", v1.value.d_val);
            result.value.d_val = -v1.value.d_val;
            break;
        }
        printf("DEBUG: Unary minus result: ");
        switch (result.type)
        {
        case TYPE_INT:
            printf("%d\n", result.value.i_val);
            break;
        case TYPE_FLOAT:
            printf("%f\n", result.value.f_val);
            break;
        case TYPE_DOUBLE:
            printf("%g\n", result.value.d_val);
            break;
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

    /* function calls */
    case 'L':
    {
        eval(a->l);
        result = eval(a->r);
        return result;
    }

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

        // Get format string
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
        {
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

            result.type = TYPE_INT;
            result.value.i_val = chars_written;
            break;
        }

        case B_scanf:
        {
            // Check format string
            for (const char *p = format; *p; p++)
            {
                if (*p == '%')
                {
                    p++;
                    if (*p != 'd' && *p != 'f')
                    {
                        error("unsupported format specifier %%%c for scanf", *p);
                        result.type = TYPE_INT;
                        result.value.i_val = 0;
                        return result;
                    }
                }
            }

            // Get the variable reference
            if (!args || args->nodetype != 'N')
            {
                error("scanf requires a variable reference");
                result.type = TYPE_INT;
                result.value.i_val = 0;
                return result;
            }

            struct symref *ref = (struct symref *)args;
            int items_read = 0;

            // Handle different format specifiers
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
                    printf("DEBUG: scanf read integer: %d\n", ref->s->value.i_val);
                }
                break;

            case 'f':
                if (ref->s->type == TYPE_FLOAT)
                {
                    items_read = scanf("%f", &ref->s->value.f_val);
                    printf("DEBUG: scanf read float: %f\n", ref->s->value.f_val);
                }
                else if (ref->s->type == TYPE_DOUBLE)
                {
                    items_read = scanf("%lf", &ref->s->value.d_val);
                    printf("DEBUG: scanf read double: %g\n", ref->s->value.d_val);
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

            // Clear any remaining characters in the input buffer
            int c;
            while ((c = getchar()) != '\n' && c != EOF)
                ;

            result.type = TYPE_INT;
            result.value.i_val = items_read;
            return result;
        }
        }

        return result;
    }

    case 'C':
    {
        struct ufncall *f = (struct ufncall *)a;
        struct scope *save_scope = current_scope;
        struct symbol *save_function = current_function_sym;

        if (!f->s->func)
        {
            error("call to undefined function %s", f->s->name);
            result.type = TYPE_INT;
            result.value.i_val = 0;
            return result;
        }

        /* Create new scope for function */
        push_scope();
        current_function_sym = f->s;

        /* Evaluate arguments */
        int arg_count = 0;
        struct value *args = evaluate_arguments(f->l, &arg_count);

        /* Bind parameters in correct order */
        struct symbol_list *sl = f->s->syms;
        for (int i = 0; i < arg_count && sl; i++)
        {
            printf("DEBUG: Binding argument %d (value: ", i + 1);
            switch (args[i].type)
            {
            case TYPE_INT:
                printf("%d", args[i].value.i_val);
                break;
            case TYPE_FLOAT:
                printf("%f", args[i].value.f_val);
                break;
            case TYPE_DOUBLE:
                printf("%g", args[i].value.d_val);
                break;
            }
            printf(") to parameter %s\n", sl->sym->name);

            convert_value(&sl->sym->value, sl->sym->type, &args[i].value, args[i].type);
            sl = sl->next;
        }

        if (args)
        {
            free(args);
        }

        /* Evaluate function body */
        printf("DEBUG: Evaluating function body for %s\n", f->s->name);
        result = eval(f->s->func);
        printf("DEBUG: Function evaluation complete, return type %d\n", result.type);

        /* Restore state */
        pop_scope();
        current_scope = save_scope;
        current_function_sym = save_function;

        return result;
    }

    default:
        printf("internal error: bad node %c\n", a->nodetype);
        result.type = TYPE_INT;
        result.value.i_val = 0;
        return result;
    }
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

void treefree(struct ast *a)
{
    if (!a)
        return;

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
        treefree(a->r);
        /* fall through */

    /* one subtree */
    case '|':
    case 'M':
    case 'C':
    case 'F':
        treefree(a->l);
        break;

    /* no subtree */
    case 'D':
    case 'K':
    case 'N':
        break;

    case '=':
        free(((struct symasgn *)a)->v);
        break;

    case 'I':
    case 'W':
        free(((struct flow *)a)->cond);
        if (((struct flow *)a)->tl)
            free(((struct flow *)a)->tl);
        if (((struct flow *)a)->el)
            free(((struct flow *)a)->el);
        break;

    case 'S':
        free(((struct strval *)a)->str);
        break;

    default:
        error("internal error: free bad node %c\n", a->nodetype);
    }

    free(a); /* always free the node itself */
}

void error(char *s, ...)
{
    va_list ap;
    va_start(ap, s);
    fprintf(stderr, "%d: error: ", yylineno);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}

/* debug: dump out an AST */
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
    case 'L':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
        printf("binop %c\n", a->nodetype);
        dumpast(a->l, level);
        dumpast(a->r, level);
        return;

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

void debug_print(const char *rule, const char *msg)
{
    printf("DEBUG: %s - %s\n", rule, msg);
    fflush(stdout);
}

static void init_scope_system(void)
{
    // Create global scope
    current_scope = push_scope();
}

int main(void)
{
    init_scope_system();
    printf("> ");
    return yyparse();
}