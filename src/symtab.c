#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "symtab.h"

#define DEFAULT_BUCKETS 211   // Good prime number for hashing

static Scope *current_scope = NULL;

static unsigned hash(const char *s, int mod) {
    unsigned h = 0;
    while (*s) h = 31*h + (unsigned char)*s++;
    return h % mod;
}

static Scope *new_scope(Scope *parent) {
    Scope *s = malloc(sizeof(Scope));
    s->bucket_count = DEFAULT_BUCKETS;
    s->table = calloc(s->bucket_count, sizeof(Symbol *));
    s->parent = parent;
    return s;
}

void init_symtab() {
    current_scope = new_scope(NULL); // global scope
}

void enter_scope() {
    current_scope = new_scope(current_scope);
}

void exit_scope() {
    if (!current_scope) return;

    // free symbols in this scope
    for (int i = 0; i < current_scope->bucket_count; i++) {
        Symbol *sym = current_scope->table[i];
        while (sym) {
            Symbol *next = sym->next;
            free(sym->name);
            free(sym);
            sym = next;
        }
    }

    Scope *parent = current_scope->parent;
    free(current_scope->table);
    free(current_scope);
    current_scope = parent;
}

bool add_symbol(const char *name, Type *type) {
    if (!current_scope) init_symtab();

    // Check if already exists in current scope
    if (lookup_symbol_current(name)) {
        fprintf(stderr, "Error: redeclaration of '%s'\n", name);
        return false;
    }

    unsigned idx = hash(name, current_scope->bucket_count);

    Symbol *sym = malloc(sizeof(Symbol));
    sym->name = strdup(name);
    sym->type = type;
    sym->next = current_scope->table[idx];

    current_scope->table[idx] = sym;
    return true;
}

Symbol *lookup_symbol_current(const char *name) {
    if (!current_scope) return NULL;
    unsigned idx = hash(name, current_scope->bucket_count);
    Symbol *sym = current_scope->table[idx];
    while (sym) {
        if (strcmp(sym->name, name) == 0)
            return sym;
        sym = sym->next;
    }
    return NULL;
}

Symbol *lookup_symbol(const char *name) {
    for (Scope *s = current_scope; s; s = s->parent) {
        // Search in scope s, not current_scope
        unsigned idx = hash(name, s->bucket_count);
        Symbol *sym = s->table[idx];
        while (sym) {
            if (strcmp(sym->name, name) == 0)
                return sym;
            sym = sym->next;
        }
    }
    return NULL;
}

