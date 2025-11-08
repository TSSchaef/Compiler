#ifndef TYPE_CHECK_H
#define TYPE_CHECK_H

#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "symtab.h"

// Type system (expand as wanted)
struct Type {
    enum { TY_INT, TY_FUNC } kind;
    struct Type *return_type;
    struct Type **params;
    int param_count;
};

Type *type_int();
Type *type_func(Type *ret, Type **params, int param_count);

void type_check(AST *root);

#endif
