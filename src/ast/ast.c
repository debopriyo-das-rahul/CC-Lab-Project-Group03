#include "ast.h"

ASTNode *make_node(const char *label, int line) {
    ASTNode *n = (ASTNode *)malloc(sizeof(ASTNode));
    strncpy(n->label, label, 31);
    n->label[31] = '\0';
    n->line = line;
    n->kind = NK_OTHER;
    n->ival = 0;
    n->fval = 0;
    n->num_children = 0;
    return n;
}

void add_child(ASTNode *parent, ASTNode *child) {
    if (parent->num_children < MAX_CHILDREN)
        parent->children[parent->num_children++] = child;
}

void print_tree(ASTNode *n, int depth) {
    int i;
    if (n == NULL) return;
    for (i = 0; i < depth * 2; i++) putchar(' ');
    if (n->num_children == 0)
        printf("%s\n", n->label);      /* leaf : print label only */
    else
        printf("[%s]\n", n->label);    /* internal : wrap in brackets */
    for (i = 0; i < n->num_children; i++)
        print_tree(n->children[i], depth + 1);
}

void free_tree(ASTNode *n) {
    int i;
    if (n == NULL) return;
    for (i = 0; i < n->num_children; i++)
        free_tree(n->children[i]);
    free(n);
}
