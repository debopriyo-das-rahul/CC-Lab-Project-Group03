#include "tac.h"
#include <string.h>

#define MAX_QUADS 1000

/* Quadruple table: (op, arg1, arg2, result) -- same layout as
   Lab 4, Section 5.2's Reusable Quadruple Table Code. */
char *quadOp[MAX_QUADS], *quadArg1[MAX_QUADS], *quadArg2[MAX_QUADS], *quadRes[MAX_QUADS];
int quadCount = 0;

static int tempCount = 0;
static int labelCount = 0;

/* Same newTemp() as Lab 4, Exercise 1, Step 1. */
static char *newTemp(void) {
    char *buf = malloc(8);
    sprintf(buf, "t%d", ++tempCount);
    return buf;
}

/* Label generator, same idea as newTemp() but for control flow
   targets (Lab 4, Section 2.5). */
static char *newLabel(void) {
    char *buf = malloc(8);
    sprintf(buf, "L%d", ++labelCount);
    return buf;
}

/* Low-level: append one row to the quadruple table (Lab 4, Section
   5.2's emit(), generalized to take an explicit result instead of
   always allocating a fresh temporary -- goto/label rows don't
   need one). */
static void emit_quad(const char *op, const char *arg1, const char *arg2, const char *result) {
    quadOp[quadCount] = strdup(op);
    quadArg1[quadCount] = strdup(arg1 && arg1[0] ? arg1 : "-");
    quadArg2[quadCount] = strdup(arg2 && arg2[0] ? arg2 : "-");
    quadRes[quadCount] = strdup(result && result[0] ? result : "-");
    quadCount++;
}

/* x = y op z */
static char *emit_binop(const char *op, char *arg1, char *arg2) {
    char *result = newTemp();
    emit_quad(op, arg1, arg2, result);
    return result;
}

/* x = op y */
static char *emit_unop(const char *op, char *arg1) {
    char *result = newTemp();
    emit_quad(op, arg1, "", result);
    return result;
}

/* x = y (copy) */
static void emit_copy(char *dest, char *src) {
    emit_quad("=", src, "", dest);
}

static void emit_goto(char *label) {
    emit_quad("goto", "", "", label);
}

static void emit_ifFalse(char *cond, char *label) {
    emit_quad("ifFalse", cond, "", label);
}

static void emit_label(char *label) {
    emit_quad("label", "", "", label);
}

static void emit_print(char *arg1) {
    emit_quad("print", arg1, "", "");
}

static void gen_stmt(ASTNode *stmt);
static void gen_block(ASTNode *block);

/* Translate an expression subtree bottom-up (Lab 4, Section 2.4).
   Leaves (identifiers and literals) return their own text with no
   quadruple emitted; every operator emits exactly one quadruple
   and returns the temporary holding its result. */
static char *gen_expr(ASTNode *e) {
    if (e->kind == NK_IDENT || e->kind == NK_INT_LIT ||
        e->kind == NK_FLOAT_LIT || e->kind == NK_BOOL_LIT) {
        return e->label;
    }

    if (strcmp(e->label, "!") == 0)
        return emit_unop("not", gen_expr(e->children[0]));

    if (strcmp(e->label, "UMINUS") == 0)
        return emit_unop("uminus", gen_expr(e->children[0]));

    /* every other expr node is a binary operator: +, -, *, /, %,
       <, >, <=, >=, ==, !=, &&, || */
    {
        char *arg1 = gen_expr(e->children[0]);
        char *arg2 = gen_expr(e->children[1]);
        return emit_binop(e->label, arg1, arg2);
    }
}

static void gen_stmt(ASTNode *stmt) {
    if (strcmp(stmt->label, "DECL") == 0) {
        return; /* a declaration has no run-time effect, so no TAC is emitted */
    }

    if (strcmp(stmt->label, "ASSIGN") == 0) {
        char *place = gen_expr(stmt->children[1]);
        emit_copy(stmt->children[0]->label, place);
        return;
    }

    if (strcmp(stmt->label, "IF") == 0) {
        char *cond = gen_expr(stmt->children[0]);
        char *L1 = newLabel();
        emit_ifFalse(cond, L1);
        gen_block(stmt->children[1]);
        emit_label(L1);
        return;
    }

    if (strcmp(stmt->label, "IF_ELSE") == 0) {
        char *cond = gen_expr(stmt->children[0]);
        char *L1 = newLabel();
        char *L2 = newLabel();
        emit_ifFalse(cond, L1);
        gen_block(stmt->children[1]);
        emit_goto(L2);
        emit_label(L1);
        gen_block(stmt->children[2]);
        emit_label(L2);
        return;
    }

    if (strcmp(stmt->label, "WHILE") == 0) {
        char *L1 = newLabel();
        char *L2 = newLabel();
        emit_label(L1);
        char *cond = gen_expr(stmt->children[0]);
        emit_ifFalse(cond, L2);
        gen_block(stmt->children[1]);
        emit_goto(L1);
        emit_label(L2);
        return;
    }

    if (strcmp(stmt->label, "PRINT") == 0) {
        emit_print(gen_expr(stmt->children[0]));
        return;
    }

    if (strcmp(stmt->label, "BLOCK") == 0) {
        gen_block(stmt);
        return;
    }
}

static void gen_block(ASTNode *block) {
    int i;
    for (i = 0; i < block->num_children; i++)
        gen_stmt(block->children[i]);
}

void generate_tac(ASTNode *root) {
    gen_block(root);
}

void print_tac(void) {
    int i;
    printf("\n=== Three-Address Code ===\n");
    for (i = 0; i < quadCount; i++) {
        const char *op = quadOp[i];
        if (strcmp(op, "label") == 0)
            printf("%s:\n", quadRes[i]);
        else if (strcmp(op, "goto") == 0)
            printf("goto %s\n", quadRes[i]);
        else if (strcmp(op, "ifFalse") == 0)
            printf("ifFalse %s goto %s\n", quadArg1[i], quadRes[i]);
        else if (strcmp(op, "print") == 0)
            printf("print %s\n", quadArg1[i]);
        else if (strcmp(op, "=") == 0)
            printf("%s = %s\n", quadRes[i], quadArg1[i]);
        else if (strcmp(op, "uminus") == 0 || strcmp(op, "not") == 0)
            printf("%s = %s %s\n", quadRes[i], op, quadArg1[i]);
        else
            printf("%s = %s %s %s\n", quadRes[i], quadArg1[i], op, quadArg2[i]);
    }
}
