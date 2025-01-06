/* ansi_c_interpreter_funcs.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "ansi_c_interpreter.h"

/* Global variables */
struct symbol symtab[NHASH]; /* symbol table */
int debug = 1;               /* debug flag */
struct ast *root = NULL;
enum value_type current_type = TYPE_INT;

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

/* symbol table */
static unsigned symhash(char *sym)
{
    unsigned int hash = 0;
    unsigned c;
    while ((c = *sym++))
        hash = hash * 9 ^ c;
    return hash;
}

struct symbol *lookup(char *sym)
{
    printf("DEBUG: Looking up symbol: %s\n", sym);
    unsigned int hash = symhash(sym);
    printf("DEBUG: Symbol hash: %u\n", hash);

    struct symbol *sp = &symtab[hash % NHASH];
    int scount = NHASH;

    while (--scount >= 0)
    {
        if (sp->name && !strcmp(sp->name, sym))
        {
            printf("DEBUG: Found existing symbol: %s\n", sp->name);
            return sp;
        }

        if (!sp->name)
        {
            printf("DEBUG: Creating new symbol: %s\n", sym);
            sp->name = strdup(sym);
            if (!sp->name)
            {
                printf("ERROR: Memory allocation failed for symbol name\n");
                abort();
            }
            sp->type = TYPE_DOUBLE; // Default type until declared
            sp->value.d_val = 0;
            sp->func = NULL;
            sp->syms = NULL;
            return sp;
        }

        if (++sp >= symtab + NHASH)
            sp = symtab;
    }
    printf("ERROR: Symbol table overflow\n");
    error("symbol table overflow\n");
    abort();
}

struct symlist *newsymlist(struct symbol *sym, struct symlist *next)
{
    struct symlist *sl = malloc(sizeof(struct symlist));
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
    a->s = s;
    ((struct ast *)a)->result_type = s->type;
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

void dodef(struct symbol *name, struct symlist *syms, struct ast *func)
{
    if (name->syms)
        symlistfree(name->syms);
    if (name->func)
        treefree(name->func);
    name->syms = syms;
    name->func = func;
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
    case 'W':
    {
        if (a->nodetype == 'I')
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
        }
        else
        { // WHILE
            result.type = TYPE_INT;
            result.value.i_val = 0;
            if (((struct flow *)a)->tl)
            {
                while ((v1 = eval(((struct flow *)a)->cond)).value.i_val)
                {
                    result = eval(((struct flow *)a)->tl);
                }
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
        v1 = eval(f->l);

        // Convert to double for built-in functions
        union value_union temp;
        convert_value(&temp, TYPE_DOUBLE, &v1.value, v1.type);

        result.type = TYPE_DOUBLE;

        switch (f->functype)
        {
        case B_sqrt:
            if (temp.d_val < 0)
            {
                error("sqrt of negative number");
                result.value.d_val = 0;
            }
            else
            {
                result.value.d_val = sqrt(temp.d_val);
            }
            break;
        case B_exp:
            result.value.d_val = exp(temp.d_val);
            break;
        case B_log:
            if (temp.d_val <= 0)
            {
                error("log of non-positive number");
                result.value.d_val = 0;
            }
            else
            {
                result.value.d_val = log(temp.d_val);
            }
            break;
        case B_print:
            switch (v1.type)
            {
            case TYPE_INT:
                printf("= %d\n", v1.value.i_val);
                break;
            case TYPE_FLOAT:
                printf("= %f\n", v1.value.f_val);
                break;
            case TYPE_DOUBLE:
                printf("= %g\n", v1.value.d_val);
                break;
            }
            result = v1;
            break;
        default:
            error("Unknown built-in function %d", f->functype);
            result.value.d_val = 0;
        }
        return result;
    }

    case 'C':
    {
        struct ufncall *f = (struct ufncall *)a;
        if (!f->s->func)
        {
            error("call to undefined function %s", f->s->name);
            result.type = TYPE_INT;
            result.value.i_val = 0;
            return result;
        }

        // Evaluate and store arguments
        struct ast *args = f->l;
        struct symlist *sl = f->s->syms;
        struct value *oldval = NULL;
        int nargs;

        for (nargs = 0, sl = f->s->syms; sl; sl = sl->next)
            nargs++;

        if (nargs > 0)
            oldval = malloc(nargs * sizeof(struct value));

        for (int i = 0; sl && args; i++)
        {
            if (args->nodetype == 'L')
            {
                oldval[i].type = sl->sym->type;
                oldval[i].value = sl->sym->value;
                v1 = eval(args->l);
                convert_value(&sl->sym->value, sl->sym->type, &v1.value, v1.type);
                args = args->r;
            }
            else
            {
                oldval[i].type = sl->sym->type;
                oldval[i].value = sl->sym->value;
                v1 = eval(args);
                convert_value(&sl->sym->value, sl->sym->type, &v1.value, v1.type);
                args = NULL;
            }
            sl = sl->next;
        }

        result = eval(f->s->func);

        // Restore old values
        sl = f->s->syms;
        for (int i = 0; i < nargs; i++)
        {
            sl->sym->type = oldval[i].type;
            sl->sym->value = oldval[i].value;
            sl = sl->next;
        }

        if (oldval)
            free(oldval);

        return result;
    }

    default:
        printf("internal error: bad node %c\n", a->nodetype);
        result.type = TYPE_INT;
        result.value.i_val = 0;
        return result;
    }
}

void symlistfree(struct symlist *sl)
{
    struct symlist *nsl;

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

int main(void)
{
    printf("> ");
    return yyparse();
}