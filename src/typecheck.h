#ifndef TYPE_CHECK_H
#define TYPE_CHECK_H

#include <stdio.h>
#include <stdlib.h>

//#include "global.h"
#include "ast.h"
#include "symtab.h"

extern FILE *outputFile;

extern char *getOutputFileName();
extern char *getCurrentFileName();


// Type system (expand as wanted)
struct Type {
    enum { TY_INT, TY_CHAR, TY_FLT, TY_VOID, TY_ARRAY, TY_FUNC } kind;
    struct Type *return_type;
    struct Type *array_of;
    struct Type **params;
    int param_count;
};

Type *type_int();
Type *type_char();
Type *type_float();
Type *type_void();

Type *type_func(Type *ret, Type **params, int param_count);

void type_check(AST *root);

#endif
