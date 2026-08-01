%{
/* Section 1: Prologue -- C declarations */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ast/ast.h"
#include "../semantic/semantic.h"
#include "../tac/tac.h"
#include "../symbol_table/symbol_table.h"

int yylex(void);
void yyerror(const char *s);
extern int yylineno;

ASTNode *ast_root; /* the finished AST is stashed here after a successful parse */
int syntax_error_count = 0;
%}

%union {
    int ival;
    double fval;
    char *sval;
    struct ASTNode *node;
}

/* Token declarations -- these become #defines in parser.tab.h */
%token INT FLOAT BOOL
%token IF ELSE WHILE PRINT
%token TRUE FALSE
%token <sval> IDENTIFIER
%token <ival> INT_LIT
%token <fval> FLOAT_LIT

%token PLUS MINUS STAR SLASH MOD
%token LT GT LE GE EQ NE
%token AND OR NOT
%token ASSIGN
%token SEMI LBRACE RBRACE LPAREN RPAREN

/* Precedence / associativity, lowest to highest (see Lab 2, Section 2.5) */
%left OR
%left AND
%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left STAR SLASH MOD
%right NOT UMINUS

%type <node> program stmt_list stmt block decl assign if_stmt while_stmt print_stmt expr type

%%
/* Section 2: Grammar Rules */

program:
    stmt_list { ast_root = $1; }
    ;

stmt_list:
    stmt_list stmt
    {
        $$ = $1;
        add_child($$, $2);
    }
    | /* empty */
    {
        $$ = make_node("BLOCK", yylineno);
    }
    ;

stmt:
    decl
    | assign
    | if_stmt
    | while_stmt
    | print_stmt
    | block
    | error SEMI
    {
        /* basic panic-mode recovery (Section 4.2): discard tokens up
           to the next ';' and resume parsing as if this had been a
           statement, instead of aborting the whole parse */
        $$ = make_node("ERROR", yylineno);
        yyerrok;
    }
    ;

block:
    LBRACE stmt_list RBRACE
    {
        $$ = $2;
    }
    ;

type:
    INT   { $$ = make_node("int", yylineno); }
    | FLOAT { $$ = make_node("float", yylineno); }
    | BOOL  { $$ = make_node("bool", yylineno); }
    ;

decl:
    type IDENTIFIER SEMI
    {
        $$ = make_node("DECL", yylineno);
        add_child($$, $1);
        ASTNode *id = make_node($2, yylineno);
        add_child($$, id);
        free($2);
    }
    ;

assign:
    IDENTIFIER ASSIGN expr SEMI
    {
        $$ = make_node("ASSIGN", yylineno);
        ASTNode *id = make_node($1, yylineno);
        add_child($$, id);
        add_child($$, $3);
        free($1);
    }
    ;

if_stmt:
    IF LPAREN expr RPAREN block
    {
        $$ = make_node("IF", yylineno);
        add_child($$, $3);
        add_child($$, $5);
    }
    | IF LPAREN expr RPAREN block ELSE block
    {
        $$ = make_node("IF_ELSE", yylineno);
        add_child($$, $3);
        add_child($$, $5);
        add_child($$, $7);
    }
    ;

while_stmt:
    WHILE LPAREN expr RPAREN block
    {
        $$ = make_node("WHILE", yylineno);
        add_child($$, $3);
        add_child($$, $5);
    }
    ;

print_stmt:
    PRINT expr SEMI
    {
        $$ = make_node("PRINT", yylineno);
        add_child($$, $2);
    }
    ;

expr:
    expr PLUS expr  { $$ = make_node("+", yylineno);  add_child($$, $1); add_child($$, $3); }
    | expr MINUS expr { $$ = make_node("-", yylineno);  add_child($$, $1); add_child($$, $3); }
    | expr STAR expr  { $$ = make_node("*", yylineno);  add_child($$, $1); add_child($$, $3); }
    | expr SLASH expr { $$ = make_node("/", yylineno);  add_child($$, $1); add_child($$, $3); }
    | expr MOD expr   { $$ = make_node("%", yylineno);  add_child($$, $1); add_child($$, $3); }
    | expr LT expr    { $$ = make_node("<", yylineno);  add_child($$, $1); add_child($$, $3); }
    | expr GT expr    { $$ = make_node(">", yylineno);  add_child($$, $1); add_child($$, $3); }
    | expr LE expr    { $$ = make_node("<=", yylineno); add_child($$, $1); add_child($$, $3); }
    | expr GE expr    { $$ = make_node(">=", yylineno); add_child($$, $1); add_child($$, $3); }
    | expr EQ expr    { $$ = make_node("==", yylineno); add_child($$, $1); add_child($$, $3); }
    | expr NE expr    { $$ = make_node("!=", yylineno); add_child($$, $1); add_child($$, $3); }
    | expr AND expr   { $$ = make_node("&&", yylineno); add_child($$, $1); add_child($$, $3); }
    | expr OR expr    { $$ = make_node("||", yylineno); add_child($$, $1); add_child($$, $3); }
    | NOT expr        { $$ = make_node("!", yylineno);  add_child($$, $2); }
    | MINUS expr %prec UMINUS { $$ = make_node("UMINUS", yylineno); add_child($$, $2); }
    | LPAREN expr RPAREN { $$ = $2; }
    | IDENTIFIER
    {
        $$ = make_node($1, yylineno);
        $$->kind = NK_IDENT;
        free($1);
    }
    | INT_LIT
    {
        char buf[16];
        sprintf(buf, "%d", $1);
        $$ = make_node(buf, yylineno);
        $$->kind = NK_INT_LIT;
        $$->ival = $1;
    }
    | FLOAT_LIT
    {
        char buf[32];
        sprintf(buf, "%g", $1);
        $$ = make_node(buf, yylineno);
        $$->kind = NK_FLOAT_LIT;
        $$->fval = $1;
    }
    | TRUE  { $$ = make_node("true", yylineno); $$->kind = NK_BOOL_LIT; $$->ival = 1; }
    | FALSE { $$ = make_node("false", yylineno); $$->kind = NK_BOOL_LIT; $$->ival = 0; }
    ;

%%
/* Section 3: Epilogue -- C functions */
void yyerror(const char *s) {
    fprintf(stderr, "Syntax Error: %s at line %d\n", s, yylineno);
    syntax_error_count++;
}

int main() {
    printf("--- Parsing ---\n");
    if (yyparse() == 0) {
        printf("--- Abstract Syntax Tree ---\n");
        print_tree(ast_root, 0);

        if (syntax_error_count > 0) {
            printf("--- Semantic Analysis ---\n");
            printf("Skipped: %d syntax error(s) found, fix these first.\n", syntax_error_count);
        } else {
            printf("--- Semantic Analysis ---\n");
            int errors = analyze(ast_root);
            print_symtab();
            if (errors == 0) {
                printf("No semantic errors found.\n");
                generate_tac(ast_root);
                print_tac();
            } else {
                printf("%d semantic error(s) found.\n", errors);
            }
        }

        free_tree(ast_root);
    }
    return 0;
}
