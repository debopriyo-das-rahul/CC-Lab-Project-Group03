#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHILDREN 64

/* Leaf classification. Needed by the semantic analyzer (src/semantic)
   to tell identifiers and literals apart reliably -- the label text
   alone is ambiguous (e.g. a float literal "100.0" can print as
   "100", which looks identical to an int literal). Non-leaf /
   operator / statement nodes are left as NK_OTHER. */
typedef enum {
    NK_OTHER,
    NK_IDENT,
    NK_INT_LIT,
    NK_FLOAT_LIT,
    NK_BOOL_LIT
} NodeKind;

/* A single generic AST node. The label identifies the language
   construct it represents (e.g. "PROGRAM", "IF", "WHILE", "ASSIGN",
   "DECL", an operator like "+", or an identifier/literal's text).
   This follows the same generic-node approach as the parse tree in
   Lab 3, extended with a line number (needed later for semantic
   error reporting, Project Manual Section 4.5) and literal values. */
typedef struct ASTNode {
    char label[32];
    int line;
    NodeKind kind;

    int ival;      /* used only by INT_LIT leaves */
    double fval;    /* used only by FLOAT_LIT leaves */

    int num_children;
    struct ASTNode *children[MAX_CHILDREN];
} ASTNode;

ASTNode *make_node(const char *label, int line);
void add_child(ASTNode *parent, ASTNode *child);
void print_tree(ASTNode *n, int depth);
void free_tree(ASTNode *n);

#endif
