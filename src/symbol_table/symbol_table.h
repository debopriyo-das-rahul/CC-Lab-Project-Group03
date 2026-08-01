#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOLS 100
#define MAX_SCOPES 32

/* One symbol table entry:
   Name, Type, Scope, and Line Declared. 'active' is 1 while the
   declaring block/scope is still open, and 0 once it has exited --
   this lets print_symtab() show the FULL history of every symbol
   ever declared (with its scope level), not just what's still
   in scope by the end of the program. */
struct symbol {
    char name[32];
    char type[12];
    int scope;
    int line;
    int active;
};

extern struct symbol symtab[MAX_SYMBOLS];

/* Enter/exit a nested block scope. Call enter_scope() when a '{'
   is opened and exit_scope() when the matching '}' closes, so
   that variables declared inside a block are not visible outside
   it. */
void enter_scope(void);
void exit_scope(void);

/* Insert a new symbol into the CURRENT scope only. Returns 1 on
   success, or 0 if 'name' is already declared in the current
   scope. */
int insert(char *name, char *type, int line);

/* Look up 'name' starting in the current scope and searching
   outward through enclosing scopes (nearest declaration wins).
   Returns the symtab index if found and visible, or -1 if the
   name is undeclared or out of scope. */
int lookup(char *name);

/* Print the full symbol table (all scopes). */
void print_symtab(void);

#endif
