#ifndef JBCGEN_H
#define JBCGEN_H

#include <stdio.h>
#include <string.h>

#include "ir.h"
#include "symtab.h"
#include "ast.h"

void emit_class_header(FILE *out, const char *classname);
void emit_global_field(FILE *out, const char *name, Type *type);
void emit_method_header(FILE *out, const char *classname, const char *name, Type *return_type, AST *params);
void emit_method_footer(FILE *out);
void emit_init_method(FILE *out, const char *classname);
void emit_java_main(FILE *out, const char *classname);

void emit_java_from_ir(FILE *out, const char *classname, IRList *ir);

// Main code generation entry point
void generate_code(AST *program);

#endif
