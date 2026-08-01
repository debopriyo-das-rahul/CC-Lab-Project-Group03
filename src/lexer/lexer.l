%{
/* Section 1: Definitions */
/* C headers and declarations needed by our code */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../parser/parser.tab.h"
%}

%option yylineno

/* Named Definitions -- makes patterns reusable */
DIGIT       [0-9]
LETTER      [a-zA-Z_]
IDENTIFIER  {LETTER}({LETTER}|{DIGIT})*
INT_LIT     {DIGIT}+
FLOAT_LIT   {DIGIT}+"."{DIGIT}+
WHITESPACE  [ \t]+
NEWLINE     \n

%%
    /* Section 2: Rules */
    /* Keywords must come before the generic IDENTIFIER rule so they are
       matched first (see Lab 1, Section 2.4 Tip). */

"int"       { return INT; }
"float"     { return FLOAT; }
"bool"      { return BOOL; }
"if"        { return IF; }
"else"      { return ELSE; }
"while"     { return WHILE; }
"print"     { return PRINT; }
"true"      { return TRUE; }
"false"     { return FALSE; }

{FLOAT_LIT} { yylval.fval = atof(yytext); return FLOAT_LIT; }   /* placed before INT_LIT: longest match still applies, but ordering keeps this explicit */
{INT_LIT}   { yylval.ival = atoi(yytext); return INT_LIT; }
{IDENTIFIER} { yylval.sval = strdup(yytext); return IDENTIFIER; }

"=="        { return EQ; }
"!="        { return NE; }
"<="        { return LE; }
">="        { return GE; }
"<"         { return LT; }
">"         { return GT; }

"&&"        { return AND; }
"||"        { return OR; }
"!"         { return NOT; }

"+"         { return PLUS; }
"-"         { return MINUS; }
"*"         { return STAR; }
"/"         { return SLASH; }
"%"         { return MOD; }
"="         { return ASSIGN; }

"{"         { return LBRACE; }
"}"         { return RBRACE; }
"("         { return LPAREN; }
")"         { return RPAREN; }
";"         { return SEMI; }

"//".*      { /* single-line comment -- discarded */ }
"/*"([^*]|\*+[^*/])*\*+"/" { /* block comment -- discarded */ }

{WHITESPACE} { /* skip whitespace */ }
{NEWLINE}    { /* skip newlines -- yylineno tracks line numbers automatically */ }

.           { fprintf(stderr, "Lexical Error: invalid token '%s' at line %d\n", yytext, yylineno); }

%%
/* Section 3: User Code */
/* No main() here -- the parser (parser.y) owns main() and calls
   yylex() indirectly through yyparse(), same as the mini_calc
   example in Lab 1, Exercise 4. */

