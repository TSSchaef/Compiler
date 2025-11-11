#ifndef AST_H
#define AST_H

#include <stdbool.h>

typedef enum {
    AST_INT_LITERAL,
    AST_FLOAT_LITERAL,
    AST_STRING_LITERAL,
    AST_CHAR_LITERAL,
    AST_BOOL_LITERAL,

    AST_ID,

    AST_BINOP,
    AST_ASSIGN,      
    AST_LOGICAL_OR,  /* short-circuit || */
    AST_LOGICAL_AND, /* short-circuit && */
    AST_TERNARY,
    AST_UNARY,

    AST_DECL,
    AST_FUNC,
    AST_FUNC_CALL,
    AST_BLOCK,

    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_DO_WHILE,
    AST_RETURN,
    AST_BREAK,
    AST_CONTINUE,
    AST_SWITCH,
    AST_CASE,
} ASTKind;


typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_BIT_AND,
    OP_BIT_OR,
    OP_BIT_XOR,
    OP_SHL,
    OP_SHR,
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_GT,
    OP_LE,
    OP_GE,

    OP_UNKNOWN
} BinOpKind;

typedef enum {
    UOP_PLUS,        /* +x (unary plus) */
    UOP_NEG,         /* -x (unary minus) */
    UOP_PRE_INC,     /* ++x */
    UOP_PRE_DEC,     /* --x */
    UOP_POST_INC,    /* x++ */
    UOP_POST_DEC,    /* x-- */
    UOP_ADDR,        /* &x (address-of) */
    UOP_DEREF,       /* *x (dereference) */
    UOP_LOGICAL_NOT, /* !x */
    UOP_BITWISE_NOT, /* ~x */
    UOP_CAST,        /* (TYPE) x  - cast to a type (cast target stored in node) */
    UOP_UNKNOWN
} UnaryOpKind;

typedef enum {
    AOP_ASSIGN,      /* = */
    AOP_ADD_ASSIGN,  /* += */
    AOP_SUB_ASSIGN,  /* -= */
    AOP_MUL_ASSIGN,  /* *= */
    AOP_DIV_ASSIGN,  /* /= */
    AOP_MOD_ASSIGN,  /* %= */
    AOP_AND_ASSIGN,  /* &= */
    AOP_OR_ASSIGN,   /* |= */
    AOP_XOR_ASSIGN,  /* ^= */
    AOP_SHL_ASSIGN,  /* <<= */
    AOP_SHR_ASSIGN   /* >>= */
} AssignOpKind;

struct Type; // forward declare for type checking and symbol table

typedef struct AST {
    ASTKind kind;
    struct Type *type;        // Set later by type checker

    /* generic next pointer used to build lists (top-level, stmt lists, params) */
    struct AST *next;

    int line_no; // for error reporting

    union {
        int intval;
        float floatval;
        char *strval;
        char charval;
        bool boolval;

        char *id;

        struct {
            BinOpKind op;
            struct AST *left;
            struct AST *right;
        } binop;

        struct {
            AssignOpKind op;
            struct AST *lhs;   /* MUST be an lvalue node */
            struct AST *rhs;
        } assign;

         struct {
            struct AST *left;
            struct AST *right;
        } logical; // separate from binop for short-circuiting

        struct {
            struct AST *cond;
            struct AST *iftrue;
            struct AST *iffalse;
        } ternary;

        struct {
            UnaryOpKind op;
            struct AST *operand;
            /* used only for UOP_CAST: target type to cast to */
            struct Type *cast_type;
        } unary;

        struct {
            char *name;
            struct Type *decl_type;
            struct AST *init; // allow initializer
        } decl;

        struct {
            char *name;
            struct Type *return_type;
            struct AST *params; // linked list of param decls
            struct AST *body;
        } func;

        struct {
            struct AST *callee;  
            struct AST *args;   
            int arg_count;     
        } call;

        struct {
            struct AST **statements;
            int count;
        } block;

        /* if: cond, then_branch, else_branch (else_branch may be NULL) */
        struct { 
            struct AST *cond;
            struct AST *then_branch;
            struct AST *else_branch;
        } if_stmt;

        struct { 
            struct AST *cond;
            struct AST *body;
        } while_stmt;

        struct { 
            struct AST *body;
            struct AST *cond;
        } do_while;

        struct {
            struct AST *init;
            struct AST *cond;
            struct AST *post;
            struct AST *body;
        } for_stmt;

        struct { struct AST *expr; } ret;
    };
} AST;

// Node constructors
AST *ast_int(int v);
AST *ast_id(const char *name);
AST *ast_float(double v);
AST *ast_string(char *s);
AST *ast_char(char c);
AST *ast_bool(bool b);

AST *ast_binop(BinOpKind op, AST *l, AST *r);
AST *ast_assign(AssignOpKind op, AST *lhs, AST *rhs);
AST *ast_logical_or(AST *l, AST *r);
AST *ast_logical_and(AST *l, AST *r);
AST *ast_ternary(AST *cond, AST *t, AST *f);
AST *ast_unary(UnaryOpKind op, AST *operand);
AST *ast_cast(struct Type *target, AST *operand);

AST *ast_decl(const char *name, struct Type *decl_type, AST *init);
AST *ast_func(const char *name, struct Type *return_type, AST *params, AST *body);
AST *ast_func_call(AST *callee, AST *args); /* args is linked list via AST->next; may be NULL */
AST *ast_block(AST **statements, int count);

AST *ast_if(AST *cond, AST *then_branch, AST *else_branch);
AST *ast_while(AST *cond, AST *body);
AST *ast_do_while(AST *body, AST *cond);
AST *ast_for(AST *init, AST *cond, AST *post, AST *body);
AST *ast_return(AST *expr);
AST *ast_break(void);
AST *ast_continue(void);

AST *ast_list_prepend(AST *head, AST *node); /* prepends node to linked list head */
AST *ast_block_from_list(AST *head); /* converts a linked list (via AST->next) into an AST_BLOCK */

AST *ast_set_line_no(AST *node, int line_no);
int ast_get_line_no(AST *node);

// Utility
void ast_free(AST *node);
void ast_print(AST *node);

#endif
