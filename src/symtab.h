#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdbool.h>

/* ===== Type System Forward Declaration ===== */
typedef struct Type Type;   // You'll implement this later

/* ===== Symbol ===== */
typedef struct Symbol {
    char *name;
    Type *type;
    struct Symbol *next;  // Linked list for hash bucket
} Symbol;

/* ===== Scope ===== */
typedef struct Scope {
    Symbol **table;        // Hash table (array of buckets)
    int bucket_count;      // Usually prime, e.g. 211
    struct Scope *parent;  // Parent scope
} Scope;

/* ===== API ===== */
void init_symtab();        // Initialize global scope
void enter_scope();        // Push new scope
void exit_scope();         // Pop scope

bool add_symbol(const char *name, Type *type);
Symbol *lookup_symbol(const char *name);       // search current and parents
Symbol *lookup_symbol_current(const char *name); // search only this scope

#endif

