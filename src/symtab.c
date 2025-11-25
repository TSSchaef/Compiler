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
    s->struct_table = calloc(s->bucket_count, sizeof(Symbol *));
    s->parent = parent;
    s->local_count = 0;  // NEW: initialize local count
    return s;
}

// Helper to create basic types
static Type *type_int() {
    Type *t = malloc(sizeof(Type));
    t->kind = TY_INT;
    t->is_const = false;
    t->return_type = NULL;
    t->array_of = NULL;
    t->params = NULL;
    t->param_count = 0;
    t->struct_name = NULL;
    t->members = NULL;
    t->member_count = 0;
    return t;
}

static Type *type_float() {
    Type *t = malloc(sizeof(Type));
    t->kind = TY_FLT;
    t->is_const = false;
    t->return_type = NULL;
    t->array_of = NULL;
    t->params = NULL;
    t->param_count = 0;
    t->struct_name = NULL;
    t->members = NULL;
    t->member_count = 0;
    return t;
}

static Type *type_void() {
    Type *t = malloc(sizeof(Type));
    t->kind = TY_VOID;
    t->is_const = false;
    t->return_type = NULL;
    t->array_of = NULL;
    t->params = NULL;
    t->param_count = 0;
    t->struct_name = NULL;
    t->members = NULL;
    t->member_count = 0;
    return t;
}

static Type *type_char_array() {
    Type *char_type = malloc(sizeof(Type));
    char_type->kind = TY_CHAR;
    char_type->is_const = true; // const char
    char_type->return_type = NULL;
    char_type->array_of = NULL;
    char_type->params = NULL;
    char_type->param_count = 0;
    char_type->struct_name = NULL;
    char_type->members = NULL;
    char_type->member_count = 0;
    
    Type *array_type = malloc(sizeof(Type));
    array_type->kind = TY_ARRAY;
    array_type->is_const = false;
    array_type->return_type = NULL;
    array_type->array_of = char_type;
    array_type->params = NULL;
    array_type->param_count = 0;
    array_type->struct_name = NULL;
    array_type->members = NULL;
    array_type->member_count = 0;
    
    return array_type;
}

static Type *type_function(Type *return_type, Type **params, int param_count) {
    Type *t = malloc(sizeof(Type));
    t->kind = TY_FUNC;
    t->is_const = false;
    t->return_type = return_type;
    t->array_of = NULL;
    t->params = params;
    t->param_count = param_count;
    t->struct_name = NULL;
    t->members = NULL;
    t->member_count = 0;
    return t;
}

void init_stdlib() {
    if (!current_scope) {
        fprintf(stderr, "Error: init_stdlib() called before init_symtab()\n");
        return;
    }
    
    // int getchar()
    {
        Type *func_type = type_function(type_int(), NULL, 0);
        add_symbol("getchar", func_type);
    }
    
    // int putchar(int c)
    {
        Type **params = malloc(sizeof(Type *));
        params[0] = type_int();
        Type *func_type = type_function(type_int(), params, 1);
        add_symbol("putchar", func_type);
    }
    
    // int getint()
    {
        Type *func_type = type_function(type_int(), NULL, 0);
        add_symbol("getint", func_type);
    }
    
    // void putint(int x)
    {
        Type **params = malloc(sizeof(Type *));
        params[0] = type_int();
        Type *func_type = type_function(type_void(), params, 1);
        add_symbol("putint", func_type);
    }
    
    // float getfloat()
    {
        Type *func_type = type_function(type_float(), NULL, 0);
        add_symbol("getfloat", func_type);
    }
    
    // void putfloat(float x)
    {
        Type **params = malloc(sizeof(Type *));
        params[0] = type_float();
        Type *func_type = type_function(type_void(), params, 1);
        add_symbol("putfloat", func_type);
    }
    
    // void putstring(const char s[])
    {
        Type **params = malloc(sizeof(Type *));
        params[0] = type_char_array();
        Type *func_type = type_function(type_void(), params, 1);
        add_symbol("putstring", func_type);
    }
}

bool is_stdlib_function(const char *name) {
    return strcmp(name, "getchar") == 0 ||
           strcmp(name, "putchar") == 0 ||
           strcmp(name, "getint") == 0 ||
           strcmp(name, "putint") == 0 ||
           strcmp(name, "getfloat") == 0 ||
           strcmp(name, "putfloat") == 0 ||
           strcmp(name, "putstring") == 0;
}

void init_symtab() {
    current_scope = new_scope(NULL); // global scope
    init_stdlib(); // Initialize standard library functions
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

    // free symbols in struct table
    for (int i = 0; i < current_scope->bucket_count; i++) {
        Symbol *sym = current_scope->struct_table[i];
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
    free(current_scope->struct_table);
    free(current_scope);
    current_scope = parent;
}

// NEW: Check if we're in global scope
bool is_global_scope() {
    return current_scope && current_scope->parent == NULL;
}

// NEW: Get the current local variable count
int get_local_count() {
    if (!current_scope) return 0;
    return current_scope->local_count;
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

    new_sym->is_local = !is_global_scope() && !is_func;
    
    if (new_sym->is_local) {
        new_sym->local_index = current_scope->local_count++;
    } else {
        new_sym->local_index = -1;
    }

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

// Struct-specific functions

bool add_struct(const char *name, Type *struct_type) {
    if (!current_scope) init_symtab();
    
    unsigned idx = hash(name, current_scope->bucket_count);
    
    // Check if struct already exists in current scope
    Symbol *sym = current_scope->struct_table[idx];
    while (sym) {
        if (strcmp(sym->name, name) == 0) {
            return false; // Struct already defined in this scope
        }
        sym = sym->next;
    }
    
    // Add new struct definition
    Symbol *new_sym = malloc(sizeof(Symbol));
    new_sym->name = strdup(name);
    new_sym->type = struct_type;
    new_sym->is_local = false;  // NEW: structs are not local variables
    new_sym->local_index = -1;  // NEW: no local index for structs
    new_sym->next = current_scope->struct_table[idx];
    
    current_scope->struct_table[idx] = new_sym;
    return true;
}

Type *lookup_struct_current(const char *name) {
    if (!current_scope) return NULL;
    unsigned idx = hash(name, current_scope->bucket_count);
    
    Symbol *sym = current_scope->struct_table[idx];
    while (sym) {
        if (strcmp(sym->name, name) == 0)
            return sym->type;
        sym = sym->next;
    }
    
    return NULL;
}

Type *lookup_struct(const char *name) {
    for (Scope *s = current_scope; s; s = s->parent) {
        unsigned idx = hash(name, s->bucket_count);
        
        Symbol *sym = s->struct_table[idx];
        while (sym) {
            if (strcmp(sym->name, name) == 0)
                return sym->type;
            sym = sym->next;
        }
    }
    return NULL;
}

// Helper functions for struct types and members

Type *type_struct(const char *name, StructMember *members, int member_count) {
    Type *t = malloc(sizeof(Type));
    t->kind = TY_STRUCT;
    t->is_const = false;
    t->return_type = NULL;
    t->array_of = NULL;
    t->params = NULL;
    t->param_count = 0;
    t->struct_name = strdup(name);
    t->members = members;
    t->member_count = member_count;
    return t;
}

StructMember *struct_member_create(const char *name, Type *type) {
    StructMember *m = malloc(sizeof(StructMember));
    m->name = strdup(name);
    m->type = type;
    m->next = NULL;
    return m;
}

StructMember *struct_member_find(Type *struct_type, const char *member_name) {
    if (!struct_type || struct_type->kind != TY_STRUCT) {
        return NULL;
    }
    
    for (StructMember *m = struct_type->members; m; m = m->next) {
        if (strcmp(m->name, member_name) == 0) {
            return m;
        }
    }
    
    return NULL;
}

Symbol *copy_symbol(const Symbol *sym){
    if(!sym) return NULL;
    Symbol *new_sym = malloc(sizeof(Symbol));
    new_sym->name = strdup(sym->name);
    new_sym->type = sym->type; // shallow copy of type
    new_sym->is_local = sym->is_local;
    new_sym->local_index = sym->local_index;

    return new_sym;
}

