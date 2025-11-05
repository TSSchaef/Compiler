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

