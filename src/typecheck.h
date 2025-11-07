#ifndef TYPE_CHECK_H
#define TYPE_CHECK_H

#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "symtab.h"

void type_check(AST *root);

#endif
