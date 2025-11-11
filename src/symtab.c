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
    s->var_table = calloc(s->bucket_count, sizeof(Symbol *));
    s->func_table = calloc(s->bucket_count, sizeof(Symbol *));
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

    // free symbols in variable table
    for (int i = 0; i < current_scope->bucket_count; i++) {
        Symbol *sym = current_scope->var_table[i];
        while (sym) {
            Symbol *next = sym->next;
            free(sym->name);
            free(sym);
            sym = next;
        }
    }

    // free symbols in function table
    for (int i = 0; i < current_scope->bucket_count; i++) {
        Symbol *sym = current_scope->func_table[i];
        while (sym) {
            Symbol *next = sym->next;
            free(sym->name);
            free(sym);
            sym = next;
        }
    }

    Scope *parent = current_scope->parent;
    free(current_scope->var_table);
    free(current_scope->func_table);
    free(current_scope);
    current_scope = parent;
}

bool add_symbol(const char *name, Type *type) {
    if (!current_scope) init_symtab();

    bool is_func = (type && type->kind == TY_FUNC);
    Symbol **table = is_func ? current_scope->func_table : current_scope->var_table;
    
    // Check if already exists in the appropriate table
    unsigned idx = hash(name, current_scope->bucket_count);
    Symbol *sym = table[idx];
    while (sym) {
        if (strcmp(sym->name, name) == 0) {
            return false;
        }
        sym = sym->next;
    }

    // Add new symbol to appropriate table
    Symbol *new_sym = malloc(sizeof(Symbol));
    new_sym->name = strdup(name);
    new_sym->type = type;
    new_sym->next = table[idx];

    table[idx] = new_sym;
    return true;
}

Symbol *lookup_symbol_current(const char *name) {
    if (!current_scope) return NULL;
    unsigned idx = hash(name, current_scope->bucket_count);
    
    // Search variable table first
    Symbol *sym = current_scope->var_table[idx];
    while (sym) {
        if (strcmp(sym->name, name) == 0)
            return sym;
        sym = sym->next;
    }
    
    // Then search function table
    sym = current_scope->func_table[idx];
    while (sym) {
        if (strcmp(sym->name, name) == 0)
            return sym;
        sym = sym->next;
    }
    
    return NULL;
}

Symbol *lookup_symbol(const char *name) {
    for (Scope *s = current_scope; s; s = s->parent) {
        unsigned idx = hash(name, s->bucket_count);
        
        // Search variable table first
        Symbol *sym = s->var_table[idx];
        while (sym) {
            if (strcmp(sym->name, name) == 0)
                return sym;
            sym = sym->next;
        }
        
        // Then search function table
        sym = s->func_table[idx];
        while (sym) {
            if (strcmp(sym->name, name) == 0)
                return sym;
            sym = sym->next;
        }
    }
    return NULL;
}
