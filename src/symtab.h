
#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdbool.h>

typedef struct Type {
    enum { TY_INT, TY_CHAR, TY_FLT, TY_VOID, TY_ARRAY, TY_FUNC } kind;
    struct Type *return_type;
    struct Type *array_of;
    struct Type **params;
    int param_count;
} Type;

typedef struct Symbol {
    char *name;
    Type *type;
    struct Symbol *next;
} Symbol;

typedef struct Scope {
    Symbol **var_table;      // Separate table for variables
    Symbol **func_table;     // Separate table for functions
    int bucket_count;
    struct Scope *parent;
} Scope;

// API
void init_symtab();         // call once at start
void enter_scope();         // call on block/function entry
void exit_scope();          // call on block/function exit

bool add_symbol(const char *name, Type *type);
Symbol *lookup_symbol(const char *name);           // search current + ancestors
Symbol *lookup_symbol_current(const char *name);   // search only current scope

#endif
