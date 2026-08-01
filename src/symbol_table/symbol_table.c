#include "symbol_table.h"

struct symbol symtab[MAX_SYMBOLS];
int symcount = 0;

/* scope_start[i] records the symtab index where scope level i
   began. exit_scope() uses this to find which symbols belong to
   the scope that just closed. */
int scope_start[MAX_SCOPES];
int current_scope = 0;

void enter_scope(void) {
    current_scope++;
    scope_start[current_scope] = symcount;
}

void exit_scope(void) {
    int i;
    /* mark every symbol declared in this scope as no longer active,
       but keep the entries in the table -- that's what lets
       print_symtab() show the complete history at the end */
    for (i = scope_start[current_scope]; i < symcount; i++)
        symtab[i].active = 0;
    current_scope--;
}

int insert(char *name, char *type, int line) {
    int i;
    /* redeclaration check: only look at ACTIVE symbols within the
       CURRENT scope */
    for (i = scope_start[current_scope]; i < symcount; i++) {
        if (symtab[i].active && strcmp(symtab[i].name, name) == 0)
            return 0; /* already declared in this scope */
    }
    strcpy(symtab[symcount].name, name);
    strcpy(symtab[symcount].type, type);
    symtab[symcount].scope = current_scope;
    symtab[symcount].line = line;
    symtab[symcount].active = 1;
    symcount++;
    return 1;
}

int lookup(char *name) {
    int i;
    /* search backwards from the most recently declared symbol so
       that the nearest (innermost) declaration is found first --
       only ACTIVE symbols are visible */
    for (i = symcount - 1; i >= 0; i--) {
        if (symtab[i].active && strcmp(symtab[i].name, name) == 0)
            return i;
    }
    return -1;
}

void print_symtab(void) {
    int i;
    printf("\n=== Symbol Table ===\n");
    printf("%-4s %-20s %-10s %-8s %-6s %-8s\n",
           "No.", "Name", "Type", "Scope", "Line", "Status");
    for (i = 0; i < symcount; i++)
        printf("%-4d %-20s %-10s %-8d %-6d %-8s\n",
               i + 1, symtab[i].name, symtab[i].type,
               symtab[i].scope, symtab[i].line,
               symtab[i].active ? "active" : "exited");
}
