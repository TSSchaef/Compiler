#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdbool.h>

typedef struct Type Type; // forward declare for type system

typedef struct Symbol {
    char *name;
    Type *type;
    struct Symbol *next;
} Symbol;

typedef struct Scope {
    Symbol **table;
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
