#ifndef IR_H
#define IR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "symtab.h"


typedef enum {
    IR_NOP,
    IR_LABEL,
    IR_JUMP,
    IR_JUMP_IF_ZERO,
    IR_LOAD_GLOBAL,
    IR_STORE_GLOBAL,
    IR_LOAD_LOCAL,
    IR_STORE_LOCAL,
    IR_PUSH_INT,
    IR_PUSH_FLOAT,
    IR_PUSH_STRING,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_MOD,
    IR_NEG,
    IR_BIT_AND,
    IR_BIT_OR,
    IR_BIT_XOR,
    IR_BIT_NOT,
    IR_SHL,
    IR_SHR,
    IR_EQ,
    IR_NEQ,
    IR_LT,
    IR_GT,
    IR_LE,
    IR_GE,
    IR_CALL,
    IR_RETURN,
    IR_RETURN_VOID,
    IR_POP,
    IR_DUP,
    IR_CAST_I2F,
    IR_CAST_F2I,
    IR_CAST_I2D,
    IR_CAST_D2I,
    IR_CAST_F2D,
    IR_CAST_D2F,
} IRKind;

typedef struct IRInstruction {
    IRKind kind;
    const char *s;  // optional symbol (variable name, function name, etc.)
    int i;          // optional integer literal or local variable index
    float f;        // optional float literal
    struct IRInstruction *next;
} IRInstruction;

typedef struct {
    IRInstruction *head;
    IRInstruction *tail;
} IRList;

void irlist_init(IRList *l);
void ir_emit(IRList *l, IRKind k, const char *s, int i);
void ir_emit_float(IRList *l, IRKind k, float f);

void generate_ir_from_ast(AST *ast, IRList *out);

#endif
