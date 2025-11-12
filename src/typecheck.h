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

Type *type_int_const(bool is_const) ;
Type *type_char_const(bool is_const) ;
Type *type_float_const(bool is_const) ;
Type *type_void_const(bool is_const) ;

Type *set_const(Type *t);

Type *type_array(Type *elem_type);

Type *type_func(Type *ret, Type **params, int param_count);

void type_check(AST *root);
void type_check_program(AST *root);

#endif
