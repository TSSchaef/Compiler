#ifndef SYMTAB_H
#define SYMTAB_H

#include <stdbool.h>

// Forward declaration for struct members
typedef struct StructMember StructMember;

typedef struct Type {
    enum { TY_INT, TY_CHAR, TY_FLT, TY_VOID, TY_ARRAY, TY_FUNC, TY_STRUCT } kind;
    bool is_const;
    struct Type *return_type;
    struct Type *array_of;
    struct Type **params;
    int param_count;
    
    // For struct type
    char *struct_name;           // Name of the struct type
    StructMember *members;       // Linked list of struct members
    int member_count;            // Number of members
} Type;

// Struct member definition
struct StructMember {
    char *name;
    Type *type;
    struct StructMember *next;
};

typedef struct Symbol {
    char *name;
    Type *type;
    struct Symbol *next;
} Symbol;

typedef struct Scope {
    Symbol **var_table;      // Separate table for variables
    Symbol **func_table;     // Separate table for functions
    Symbol **struct_table;   // Separate table for struct definitions
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

// Struct-specific functions
bool add_struct(const char *name, Type *struct_type);
Type *lookup_struct(const char *name);             // search current + ancestors
Type *lookup_struct_current(const char *name);     // search only current scope

// Helper to create struct types
Type *type_struct(const char *name, StructMember *members, int member_count);
StructMember *struct_member_create(const char *name, Type *type);
StructMember *struct_member_find(Type *struct_type, const char *member_name);

// Initialize standard library functions (ComS 440 standard library)
void init_stdlib();

// Helper to check if a function is a standard library function
bool is_stdlib_function(const char *name);

#endif
