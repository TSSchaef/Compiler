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

typedef struct AST {
    ASTKind kind;
    struct Type *type;        // Set later by type checker
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
        } decl;

        struct {
            char *name;
            struct AST *params;
            struct AST *body;
        } func;

        struct {
            struct AST **statements;
            int count;
        } block;
    };
} AST;

AST *ast_int(int v);
AST *ast_id(char *name);
AST *ast_binop(char op, AST *l, AST *r);
void ast_free(AST *node);
void ast_print(AST *node);

#endif

