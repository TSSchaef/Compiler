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

Type *type_int();
Type *type_char();
Type *type_float();
Type *type_void();


Type *type_array(Type *elem_type);

Type *type_func(Type *ret, Type **params, int param_count);

void type_check(AST *root);

#endif
