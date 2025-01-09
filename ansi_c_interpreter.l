%option noyywrap nodefault yylineno
%{
/* ========================================================================== */
/*                              Includes and Declarations                     */
/* ========================================================================== */

#include "ansi_c_interpreter.h"
#include "ansi_c_interpreter.tab.h"

int column = 0;
void comment(void);
void count(void);
int check_type(void);

%}

/* ========================================================================== */
/*                              Regular Definitions                           */
/* ========================================================================== */

/* Basic character classes */
D           [0-9]
L           [a-zA-Z_]

/* Number-related patterns */
E           ([Ee][+-]?{D}+)
FS          (f|F|l|L)
IS          ((u|U)|(u|U)?(l|L|ll|LL)|(l|L|ll|LL)(u|U))

/* ========================================================================== */
/*                              Lexical Rules                                 */
/* ========================================================================== */
%%

    /* Comments */
"/*"            { comment(); }
"//"[^\n]*      { /* consume //-comment */ }

    /* Keywords */
"double"        { count(); return(DOUBLE); }
"else"          { count(); return(ELSE); }
"float"         { count(); return(FLOAT); }
"if"            { count(); return(IF); }
"int"           { count(); return(INT); }
"return"        { count(); return(RETURN); }
"while"         { count(); return(WHILE); }
"run"           { count(); return(RUN); }

    /* Built-in Functions */
"printf"        { count(); yylval.fn = B_printf; return BUILTIN; }
"scanf"         { count(); yylval.fn = B_scanf; return BUILTIN; }

    /* Identifiers */
{L}({L}|{D})*  { 
    count();
    struct symbol *s = malloc(sizeof(struct symbol));
    if (!s) {
        error("out of memory");
        exit(0);
    }
    s->name = strdup(yytext);
    s->type = NO_TYPE;
    s->func = NULL;
    s->syms = NULL;
    yylval.s = s;
    return IDENTIFIER; 
}

    /* Integer Constants */
{D}+{IS}?      { 
    count();
    yylval.v.type = TYPE_INT;
    yylval.v.value.i_val = atoi(yytext);
    return(CONSTANT); 
}

    /* Floating Point Constants */
{D}+{E}{FS}?   { 
    count();
    yylval.v.type = TYPE_DOUBLE;
    yylval.v.value.d_val = atof(yytext);
    return(CONSTANT);
}

{D}*"."{D}+({E})?{FS}?    { 
    count();
    yylval.v.type = TYPE_DOUBLE;
    yylval.v.value.d_val = atof(yytext);
    return(CONSTANT);
}

{D}+"."{D}*({E})?{FS}?    { 
    count();
    yylval.v.type = TYPE_DOUBLE;
    yylval.v.value.d_val = atof(yytext);
    return(CONSTANT);
}

    /* String Literals */
L?\"(\\.|[^\\"\n])*\"     { 
    count();
    char *processed = process_string_literal(yytext);
    struct format_string *fs = malloc(sizeof(struct format_string));
    fs->str = processed;
    yylval.format_str = fs;
    return STRING_LITERAL; 
}

    /* Operators */
"<="            { count(); return(LE_OP); }
">="            { count(); return(GE_OP); }
"=="            { count(); return(EQ_OP); }
"!="            { count(); return(NE_OP); }
";"             { count(); return(';'); }
("{"|"<%")      { count(); return('{'); }
("}"|"%>")      { count(); return('}'); }
","             { count(); return(','); }
"="             { count(); return('='); }
"("             { count(); return('('); }
")"             { count(); return(')'); }
"."             { count(); return('.'); }
"-"             { count(); return('-'); }
"+"             { count(); return('+'); }
"*"             { count(); return('*'); }
"/"             { count(); return('/'); }
"%"             { count(); return('%'); }
"<"             { count(); return('<'); }
">"             { count(); return('>'); }

    /* Whitespace */
[ \t\v\n\f]     { count(); }

    /* Unmatched Characters */
.               { /* Add code to complain about unmatched characters */ }

%%
/* ========================================================================== */
/*                              Support Functions                             */
/* ========================================================================== */

/* Process C-style comments */
void comment(void) {
    char c, prev = 0;
  
    while ((c = input()) != 0) {    /* (EOF maps to 0) */
        if (c == '/' && prev == '*')
            return;
        prev = c;
    }
    error("unterminated comment");
}

/* Track column position for error reporting */
void count(void) {
    int i;

    for (i = 0; yytext[i] != '\0'; i++) {
        if (yytext[i] == '\n')
            column = 0;
        else if (yytext[i] == '\t')
            column += 8 - (column % 8);
        else
            column++;
    }
}

/* Symbol lookup helper */
int check_type(void) {
    yylval.s = lookup(yytext);  /* Create/lookup symbol and store in yylval */
    return IDENTIFIER;
}