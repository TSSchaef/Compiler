#include "ast.h"
#include <stdlib.h>
#include <string.h>

AST *ast_int(int v) {
    AST *n = malloc(sizeof(AST));
    n->kind = AST_INT_LITERAL;
    n->intval = v;
    n->type = NULL;
    return n;
}

AST *ast_id(char *name) {
    AST *n = malloc(sizeof(AST));
    n->kind = AST_ID;
    n->id = strdup(name);
    n->type = NULL;
    return n;
}

AST *ast_binop(char op, AST *l, AST *r) {
    AST *n = malloc(sizeof(AST));
    n->kind = AST_BINOP;
    n->binop.op = op;
    n->binop.left = l;
    n->binop.right = r;
    n->type = NULL;
    return n;
}

AST *ast_decl(char *name, struct Type *decl_type, AST *init) {
    AST *n = malloc(sizeof(AST));
    n->kind = AST_DECL;
    n->decl.name = strdup(name);
    n->decl.decl_type = decl_type;
    n->decl.init = init;
    n->type = NULL;
    return n;
}

AST *ast_func(char *name, AST *params, AST *body) {
    AST *n = malloc(sizeof(AST));
    n->kind = AST_FUNC;
    n->func.name = strdup(name);
    n->func.params = params;
    n->func.body = body;
    n->type = NULL;
    return n;
}

AST *ast_block(AST **statements, int count) {
    AST *n = malloc(sizeof(AST));
    n->kind = AST_BLOCK;
    n->block.statements = statements;
    n->block.count = count;
    n->type = NULL;
    return n;
}
