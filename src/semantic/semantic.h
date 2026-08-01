#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "../ast/ast.h"

/* Walk the AST produced by the parser and check the rules in
   Project Manual Section 4.5: undeclared variables, redeclaration,
   scope violations, type mismatches, invalid assignments, and
   invalid expressions. Prints a "Semantic Error: ..." message with
   a line number for each problem found. Returns the number of
   errors found (0 means the program is semantically valid). */
int analyze(ASTNode *root);

#endif
