#ifndef AST_H
#define AST_H

typedef enum {
    AST_INT_LITERAL,
    AST_ID,
    AST_BINOP,
    AST_DECL,
    AST_FUNC,
    AST_BLOCK,
    // ... add more (return, if, while, etc.)
} ASTKind;

struct Type; // forward declare for type checking and symbol table

typedef struct AST {
    ASTKind kind;
    struct Type *type;        // Set later by type checker

    /* generic next pointer used to build lists (top-level, stmt lists, params) */
    struct AST *next;

    union {
        int intval;
        char *id;

        struct {
            char op;
            struct AST *left;
            struct AST *right;
        } binop;

        struct {
            char *name;
            struct Type *decl_type;
            struct AST *init; // allow initializer
        } decl;

        struct {
            char *name;
            struct AST *params; // linked list of param decls
            struct AST *body;
        } func;

        struct {
            struct AST **statements;
            int count;
        } block;
    };
} AST;

// Node constructors
AST *ast_int(int v);
AST *ast_id(const char *name);
AST *ast_binop(char op, AST *l, AST *r);
AST *ast_decl(const char *name, struct Type *decl_type, AST *init);
AST *ast_func(const char *name, AST *params, AST *body);
AST *ast_block(AST **statements, int count);

AST *ast_list_prepend(AST *head, AST *node); /* prepends node to linked list head */
AST *ast_block_from_list(AST *head); /* converts a linked list (via AST->next) into an AST_BLOCK */

// Utility
void ast_free(AST *node);
void ast_print(AST *node);

#endif
