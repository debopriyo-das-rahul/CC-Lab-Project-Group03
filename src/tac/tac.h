#ifndef TAC_H
#define TAC_H

#include "../ast/ast.h"

/* Walk the (semantically valid) AST and emit Three-Address Code as
   a quadruple table (Project Manual Section 4.6), using temporaries
   and labels exactly as introduced in Lab 4, Sections 2.2-2.5. */
void generate_tac(ASTNode *root);

/* Print the TAC as a readable instruction listing, e.g.:
       t1 = b * 2
       t2 = a + t1
       c = t2
   matching the illustrative output shown in Project Manual 4.6. */
void print_tac(void);

#endif
