#include "semantic.h"
#include "../symbol_table/symbol_table.h"
#include <stdarg.h>
#include <string.h>

int error_count = 0;

static void semantic_error(int line, const char *fmt, ...) {
    va_list args;
    fprintf(stderr, "Semantic Error (line %d): ", line);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    error_count++;
}

static int is_numeric(const char *type) {
    return strcmp(type, "int") == 0 || strcmp(type, "float") == 0;
}

/* Forward declarations -- statements and expressions are mutually
   recursive (a block contains statements, statements contain
   expressions, expressions can't contain statements here, but
   blocks are themselves reached through statements). */
static void analyze_stmt(ASTNode *stmt);
static void analyze_block(ASTNode *block);

/* Determine (and check) the type of an expression subtree.
   Returns "error" for a subtree that already produced an error,
   so that one bad operand does not cause a cascade of further
   type-mismatch complaints about the same expression. */
static const char *analyze_expr(ASTNode *e) {
    /* Leaves: identifiers and literals */
    if (e->kind == NK_IDENT) {
        int idx = lookup(e->label);
        if (idx == -1) {
            semantic_error(e->line, "undeclared variable '%s'", e->label);
            return "error";
        }
        return symtab[idx].type;
    }
    if (e->kind == NK_INT_LIT) return "int";
    if (e->kind == NK_FLOAT_LIT) return "float";
    if (e->kind == NK_BOOL_LIT) return "bool";

    /* Arithmetic operators: require numeric operands, result widens
       to float if either operand is float. */
    if (strcmp(e->label, "+") == 0 || strcmp(e->label, "-") == 0 ||
        strcmp(e->label, "*") == 0 || strcmp(e->label, "/") == 0 ||
        strcmp(e->label, "%") == 0) {
        const char *t1 = analyze_expr(e->children[0]);
        const char *t2 = analyze_expr(e->children[1]);
        if (strcmp(t1, "error") == 0 || strcmp(t2, "error") == 0) return "error";
        if (!is_numeric(t1) || !is_numeric(t2)) {
            semantic_error(e->line, "operator '%s' requires numeric operands, got %s and %s",
                            e->label, t1, t2);
            return "error";
        }
        return (strcmp(t1, "float") == 0 || strcmp(t2, "float") == 0) ? "float" : "int";
    }

    /* Relational operators: require numeric operands, result is bool. */
    if (strcmp(e->label, "<") == 0 || strcmp(e->label, ">") == 0 ||
        strcmp(e->label, "<=") == 0 || strcmp(e->label, ">=") == 0) {
        const char *t1 = analyze_expr(e->children[0]);
        const char *t2 = analyze_expr(e->children[1]);
        if (strcmp(t1, "error") == 0 || strcmp(t2, "error") == 0) return "error";
        if (!is_numeric(t1) || !is_numeric(t2)) {
            semantic_error(e->line, "operator '%s' requires numeric operands, got %s and %s",
                            e->label, t1, t2);
            return "error";
        }
        return "bool";
    }

    /* Equality operators: operands must match (numeric-with-numeric
       is allowed so int == float works), result is bool. */
    if (strcmp(e->label, "==") == 0 || strcmp(e->label, "!=") == 0) {
        const char *t1 = analyze_expr(e->children[0]);
        const char *t2 = analyze_expr(e->children[1]);
        if (strcmp(t1, "error") == 0 || strcmp(t2, "error") == 0) return "error";
        int both_numeric = is_numeric(t1) && is_numeric(t2);
        if (!both_numeric && strcmp(t1, t2) != 0) {
            semantic_error(e->line, "cannot compare %s with %s", t1, t2);
            return "error";
        }
        return "bool";
    }

    /* Logical operators: require boolean operands, result is bool. */
    if (strcmp(e->label, "&&") == 0 || strcmp(e->label, "||") == 0) {
        const char *t1 = analyze_expr(e->children[0]);
        const char *t2 = analyze_expr(e->children[1]);
        if (strcmp(t1, "error") == 0 || strcmp(t2, "error") == 0) return "error";
        if (strcmp(t1, "bool") != 0 || strcmp(t2, "bool") != 0) {
            semantic_error(e->line, "operator '%s' requires boolean operands, got %s and %s",
                            e->label, t1, t2);
            return "error";
        }
        return "bool";
    }

    /* Unary logical not: requires a boolean operand. */
    if (strcmp(e->label, "!") == 0) {
        const char *t1 = analyze_expr(e->children[0]);
        if (strcmp(t1, "error") == 0) return "error";
        if (strcmp(t1, "bool") != 0) {
            semantic_error(e->line, "operator '!' requires a boolean operand, got %s", t1);
            return "error";
        }
        return "bool";
    }

    /* Unary minus: requires a numeric operand, result keeps its type. */
    if (strcmp(e->label, "UMINUS") == 0) {
        const char *t1 = analyze_expr(e->children[0]);
        if (strcmp(t1, "error") == 0) return "error";
        if (!is_numeric(t1)) {
            semantic_error(e->line, "unary '-' requires a numeric operand, got %s", t1);
            return "error";
        }
        return t1;
    }

    /* Should not be reached if the grammar and this function agree
       on what an expr node can look like. */
    semantic_error(e->line, "internal error: unrecognized expression node '%s'", e->label);
    return "error";
}

static void analyze_stmt(ASTNode *stmt) {
    if (strcmp(stmt->label, "DECL") == 0) {
        ASTNode *type_node = stmt->children[0];
        ASTNode *id_node = stmt->children[1];
        if (!insert(id_node->label, type_node->label, id_node->line)) {
            semantic_error(id_node->line, "redeclaration of variable '%s'", id_node->label);
        }
        return;
    }

    if (strcmp(stmt->label, "ASSIGN") == 0) {
        ASTNode *id_node = stmt->children[0];
        ASTNode *expr_node = stmt->children[1];
        const char *expr_type = analyze_expr(expr_node);

        int idx = lookup(id_node->label);
        if (idx == -1) {
            semantic_error(id_node->line, "undeclared variable '%s'", id_node->label);
            return;
        }
        if (strcmp(expr_type, "error") == 0) return; /* already reported below */

        const char *var_type = symtab[idx].type;
        /* Allow int -> float widening; everything else must match
           exactly (Section 4.5: "Assigning a bool expression to an
           int variable, or similar"). */
        int ok = (strcmp(var_type, expr_type) == 0) ||
                 (strcmp(var_type, "float") == 0 && strcmp(expr_type, "int") == 0);
        if (!ok) {
            semantic_error(expr_node->line,
                            "invalid assignment: cannot assign %s to variable '%s' of type %s",
                            expr_type, id_node->label, var_type);
        }
        return;
    }

    if (strcmp(stmt->label, "IF") == 0 || strcmp(stmt->label, "IF_ELSE") == 0) {
        const char *cond_type = analyze_expr(stmt->children[0]);
        if (strcmp(cond_type, "error") != 0 && strcmp(cond_type, "bool") != 0) {
            semantic_error(stmt->children[0]->line,
                            "if condition must be bool, got %s", cond_type);
        }
        analyze_block(stmt->children[1]);
        if (strcmp(stmt->label, "IF_ELSE") == 0)
            analyze_block(stmt->children[2]);
        return;
    }

    if (strcmp(stmt->label, "WHILE") == 0) {
        const char *cond_type = analyze_expr(stmt->children[0]);
        if (strcmp(cond_type, "error") != 0 && strcmp(cond_type, "bool") != 0) {
            semantic_error(stmt->children[0]->line,
                            "while condition must be bool, got %s", cond_type);
        }
        analyze_block(stmt->children[1]);
        return;
    }

    if (strcmp(stmt->label, "PRINT") == 0) {
        analyze_expr(stmt->children[0]);
        return;
    }

    if (strcmp(stmt->label, "BLOCK") == 0) {
        /* a bare { ... } used directly as a statement */
        analyze_block(stmt);
        return;
    }
}

/* A block introduces a new nested scope (Section 4.4): declarations
   inside it are not visible once the block ends. */
static void analyze_block(ASTNode *block) {
    int i;
    enter_scope();
    for (i = 0; i < block->num_children; i++)
        analyze_stmt(block->children[i]);
    exit_scope();
}

int analyze(ASTNode *root) {
    int i;
    error_count = 0;
    /* root is the program's top-level statement list -- this IS the
       global scope, so we walk its children directly without an
       extra enter_scope()/exit_scope() pair. */
    for (i = 0; i < root->num_children; i++)
        analyze_stmt(root->children[i]);
    return error_count;
}
