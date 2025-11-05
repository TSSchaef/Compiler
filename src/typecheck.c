#include "typecheck.h"


void type_check(AST *root){

    switch (root->kind) {
        case AST_INT_LITERAL:
            root->type = type_int();
            break;
        case AST_ID: {
            Symbol *s = lookup(root->id);
            if (!s) error("undeclared id");
            root->type = s->type;
            break;
        }
        case AST_BINOP:
            type_check(root->binop.left);
            type_check(root->binop.right);

            if (root->binop.left->type != type_int() ||
                root->binop.right->type != type_int()) {
                error("type error in +");
            }

            root->type = type_int();
            break;
    }
}

