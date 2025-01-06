%option noyywrap nodefault yylineno
%{
# include "ansi_c_interpreter.h"
# include "ansi_c_interpreter.tab.h"
%}

/* float exponent */
EXP    ([Ee][-+]?[0-9]+)

%%
 /* single character ops */
"+" |
"-" |
"*" |
"/" |
"=" |
"|" |
"," |
";" |
"(" |
")"     { return yytext[0]; }

 /* comparison ops */
">"     { yylval.fn = 1; return CMP; }
"<"     { yylval.fn = 2; return CMP; }
"<>"    { yylval.fn = 3; return CMP; }
"=="    { yylval.fn = 4; return CMP; }
">="    { yylval.fn = 5; return CMP; }
"<="    { yylval.fn = 6; return CMP; }

 /* keywords */
"if"    { return IF; }
"then"  { return THEN; }
"else"  { return ELSE; }
"while" { return WHILE; }
"do"    { return DO; }
"let"   { return LET; }
"int"   { return TYPE_INT_T; }
"float" { return TYPE_FLOAT_T; }
"double" { return TYPE_DOUBLE_T; }

 /* built in functions */
"sqrt"  { yylval.fn = B_sqrt; return FUNC; }
"exp"   { yylval.fn = B_exp; return FUNC; }
"log"   { yylval.fn = B_log; return FUNC; }
"print" { yylval.fn = B_print; return FUNC; }

 /* debug hack */
"debug"[0-9]+ { debug = atoi(&yytext[5]); printf("debug set to %d\n", debug); }

 /* names */
[a-zA-Z][a-zA-Z0-9]*  { yylval.s = lookup(yytext); return NAME; }

 /* numbers */
[0-9]+      { 
    yylval.v.type = TYPE_INT;
    yylval.v.value.i_val = atoi(yytext); 
    return NUMBER; 
}

[0-9]+"."[0-9]*{EXP}?[fF] |
"."?[0-9]+{EXP}?[fF]    { 
    yylval.v.type = TYPE_FLOAT;
    yylval.v.value.f_val = atof(yytext);
    return NUMBER; 
}

[0-9]+"."[0-9]*{EXP}? |
"."?[0-9]+{EXP}?    { 
    yylval.v.type = TYPE_DOUBLE;
    yylval.v.value.d_val = atof(yytext);
    return NUMBER; 
}

"//".*  
[ \t]   /* ignore white space */ 
\\\n    printf("c> "); /* ignore line continuation */
"\n"    { return EOL; }

.    { yyerror("Mystery character %c\n", *yytext); }
%%