%option noyywrap nodefault yylineno
%{
# include "ansi_c_interpreter.h"
# include "ansi_c_interpreter.tab.h"
int column = 0;
void comment();
void count();
int check_type();
%}

D			[0-9]
L			[a-zA-Z_]
E			([Ee][+-]?{D}+)
FS			(f|F|l|L)
IS          ((u|U)|(u|U)?(l|L|ll|LL)|(l|L|ll|LL)(u|U))

%%
"/*"			{ comment(); }
"//"[^\n]*      { /* consume //-comment */ }


"double"		{ count(); return(DOUBLE); }
"else"			{ count(); return(ELSE); }
"float"			{ count(); return(FLOAT); }
"if"			{ count(); return(IF); }
"int"			{ count(); return(INT); }
"return"		{ count(); return(RETURN); }
"void"			{ count(); return(VOID); }
"while"			{ count(); return(WHILE); }
"printf"        { count(); yylval.fn = B_printf; return BUILTIN; }
"scanf"         { count(); yylval.fn = B_scanf; return BUILTIN; }

{L}({L}|{D})*		{ 
    count(); 
    yylval.s = lookup(yytext);
    return(IDENTIFIER); 
}

{D}+{IS}?		{ 
    count();
    yylval.v.type = TYPE_INT;
    yylval.v.value.i_val = atoi(yytext);
    printf("DEBUG: Scanned integer constant: %d\n", yylval.v.value.i_val);
    return(CONSTANT); 
}

{D}+{E}{FS}?		{ 
    count();
    yylval.v.type = TYPE_DOUBLE;
    yylval.v.value.d_val = atof(yytext);
    printf("DEBUG: Scanned float constant with exponent: %g\n", yylval.v.value.d_val);
    return(CONSTANT);
}

{D}*"."{D}+({E})?{FS}?	{ 
    count();
    yylval.v.type = TYPE_DOUBLE;
    yylval.v.value.d_val = atof(yytext);
    printf("DEBUG: Scanned float constant: %g\n", yylval.v.value.d_val);
    return(CONSTANT);
}

{D}+"."{D}*({E})?{FS}?	{ 
    count();
    yylval.v.type = TYPE_DOUBLE;
    yylval.v.value.d_val = atof(yytext);
    printf("DEBUG: Scanned float constant: %g\n", yylval.v.value.d_val);
    return(CONSTANT);
}

L?\"(\\.|[^\\"\n])*\"	{ 
    count();
    char *processed = process_string_literal(yytext);
    struct format_string *fs = malloc(sizeof(struct format_string));
    fs->str = processed;
    yylval.format_str = fs;
    return STRING_LITERAL; 
}

"<="			{ count(); return(LE_OP); }
">="			{ count(); return(GE_OP); }
"=="			{ count(); return(EQ_OP); }
"!="			{ count(); return(NE_OP); }
";"			    { count(); return(';'); }
("{"|"<%")		{ count(); return('{'); }
("}"|"%>")		{ count(); return('}'); }
","			    { count(); return(','); }
"="			    { count(); return('='); }
"("			    { count(); return('('); }
")"			    { count(); return(')'); }
"."			    { count(); return('.'); }
"-"			    { count(); return('-'); }
"+"			    { count(); return('+'); }
"*"			    { count(); return('*'); }
"/"			    { count(); return('/'); }
"%"			    { count(); return('%'); }
"<"			    { count(); return('<'); }
">"			    { count(); return('>'); }

[ \t\v\n\f]		{ count(); }
.			    { /* Add code to complain about unmatched characters */ }

%%

void comment(void)
{
	char c, prev = 0;
  
	while ((c = input()) != 0)      /* (EOF maps to 0) */
	{
		if (c == '/' && prev == '*')
			return;
		prev = c;
	}
	error("unterminated comment");
}

void count(void)
{
	int i;

	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n')
			column = 0;
		else if (yytext[i] == '\t')
			column += 8 - (column % 8);
		else
			column++;
}

int check_type(void)
{
    yylval.s = lookup(yytext);  /* Create/lookup symbol and store in yylval */
    return IDENTIFIER;
}