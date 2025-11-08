#include "typecheck.h"
#include <string.h>

Type *type_int() {
    static Type int_type = { .kind = TY_INT };
    return &int_type;
}

Type *type_func(Type *ret, Type **params, int param_count) {
    Type *ft = malloc(sizeof(Type));
    ft->kind = TY_FUNC;
    ft->return_type = ret;
    ft->params = params;
    ft->param_count = param_count;
    return ft;
}

// Example error helper
static void error(const char *msg) {
    fprintf(stderr, "Type Error: %s\n", msg);
}

void type_check(AST *node){
    if (!node) return;

    switch (node->kind) {
    case AST_INT_LITERAL:
        node->type = type_int();
        break;
    case AST_ID: {
        Symbol *s = lookup_symbol(node->id);
        if (!s) {
            error("undeclared id");
            node->type = NULL;
        } else {
            node->type = s->type;
        }
        break;
    }
    case AST_BINOP:
        type_check(node->binop.left);
        type_check(node->binop.right);
        if (!node->binop.left->type || !node->binop.right->type ||
            node->binop.left->type->kind != TY_INT ||
            node->binop.right->type->kind != TY_INT) {
            error("type error in binary op");
        }
        node->type = type_int();
        break;
    case AST_DECL:
        type_check(node->decl.init); // type check initializer if present
        add_symbol(node->decl.name, node->decl.decl_type);
        node->type = node->decl.decl_type;
        break;
    case AST_FUNC: {
        enter_scope();
        // Register parameters (assume params is a linked list of AST_DECL nodes)
        AST *param = node->func.params;
        int param_count = 0;
        Type **param_types = NULL;
        while (param) {
            type_check(param);
            param_types = realloc(param_types, sizeof(Type*) * (param_count+1));
            param_types[param_count++] = param->type;
            param = param->decl.init; // treat init as 'next' for params
        }
        Type *ft = type_func(type_int(), param_types, param_count); // assuming int return type
        add_symbol(node->func.name, ft);

        type_check(node->func.body);
        exit_scope();
        node->type = ft;
        break;
    }
    case AST_BLOCK:
        enter_scope();
        for (int i = 0; i < node->block.count; i++) {
            type_check(node->block.statements[i]);
        }
        exit_scope();
        break;
    default:
        break;
    }
}
