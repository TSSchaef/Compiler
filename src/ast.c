#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* --- Constructors --- */

AST *ast_int(int v) {
    AST *n = malloc(sizeof(AST));
    memset(n, 0, sizeof(AST));
    n->kind = AST_INT_LITERAL;
    n->intval = v;
    n->type = NULL;
    n->next = NULL;
    return n;
}

AST *ast_id(const char *name) {
    AST *n = malloc(sizeof(AST));
    memset(n, 0, sizeof(AST));
    n->kind = AST_ID;
    n->id = strdup(name ? name : "");
    n->type = NULL;
    n->next = NULL;
    return n;
}

AST *ast_binop(char op, AST *l, AST *r) {
    AST *n = malloc(sizeof(AST));
    memset(n, 0, sizeof(AST));
    n->kind = AST_BINOP;
    n->binop.op = op;
    n->binop.left = l;
    n->binop.right = r;
    n->type = NULL;
    n->next = NULL;
    return n;
}

AST *ast_decl(const char *name, struct Type *decl_type, AST *init) {
    AST *n = malloc(sizeof(AST));
    memset(n, 0, sizeof(AST));
    n->kind = AST_DECL;
    n->decl.name = strdup(name ? name : "");
    n->decl.decl_type = decl_type;
    n->decl.init = init;
    n->type = NULL;
    n->next = NULL;
    return n;
}

AST *ast_func(const char *name, AST *params, AST *body) {
    AST *n = malloc(sizeof(AST));
    memset(n, 0, sizeof(AST));
    n->kind = AST_FUNC;
    n->func.name = strdup(name ? name : "");
    n->func.params = params;
    n->func.body = body;
    n->type = NULL;
    n->next = NULL;
    return n;
}

AST *ast_block(AST **statements, int count) {
    AST *n = malloc(sizeof(AST));
    memset(n, 0, sizeof(AST));
    n->kind = AST_BLOCK;
    n->block.statements = statements;
    n->block.count = count;
    n->type = NULL;
    n->next = NULL;
    return n;
}


AST *ast_list_prepend(AST *head, AST *node) {
    if (!node) return head;
    node->next = head;
    return node;
}

/* Convert a linked list (AST->next) into an AST_BLOCK node. The list nodes
   themselves are used as the elements of the block; the function allocates an
   array and clears the next pointers in the array elements (so they behave as
   proper tree children). If head==NULL, returns an empty block (count=0, statements=NULL). */

AST *ast_block_from_list(AST *head) {
    if (!head) return ast_block(NULL, 0);

    /* count nodes */
    int count = 0;
    for (AST *p = head; p; p = p->next) count++;

    AST **arr = malloc(sizeof(AST*) * count);
    int i = 0;
    for (AST *p = head; p; p = p->next) {
        arr[i++] = p;
    }

    /* clear next pointers in array elements to avoid accidental linked-list traversals later */
    for (int j = 0; j < count; ++j) arr[j]->next = NULL;

    return ast_block(arr, count);
}



/* --- Utilities --- */

static void ast_print_indent(int indent) {
    for (int i = 0; i < indent; ++i) putchar(' ');
}

void ast_print(AST *node) {
    /* simple wrapper that calls recursive printer */
    if (!node) {
        printf("(null)\n");
        return;
    }
    /* For convenience, if the node is a block, print each statement; otherwise print the node */
    switch (node->kind) {
        case AST_BLOCK:
            printf("AST_BLOCK (count=%d)\n", node->block.count);
            for (int i = 0; i < node->block.count; ++i) {
                ast_print(node->block.statements[i]);
            }
            break;
        default: {
            /* generic single-node pretty print */
            switch (node->kind) {
                case AST_INT_LITERAL:
                    printf("INT %d\n", node->intval);
                    break;
                case AST_ID:
                    printf("ID %s\n", node->id ? node->id : "(null)");
                    break;
                case AST_BINOP:
                    printf("BINOP '%c'\n", node->binop.op);
                    ast_print(node->binop.left);
                    ast_print(node->binop.right);
                    break;
                case AST_DECL:
                    printf("DECL %s\n", node->decl.name ? node->decl.name : "(null)");
                    if (node->decl.init) {
                        printf("  init:\n");
                        ast_print(node->decl.init);
                    }
                    break;
                case AST_FUNC:
                    printf("FUNC %s\n", node->func.name ? node->func.name : "(null)");
                    if (node->func.params) {
                        printf("  params:\n");
                        for (AST *p = node->func.params; p; p = p->next) ast_print(p);
                    }
                    if (node->func.body) {
                        printf("  body:\n");
                        ast_print(node->func.body);
                    }
                    break;
                default:
                    printf("Unknown AST node kind %d\n", node->kind);
            }
            break;
        }
    }
}

/* Recursively free an AST. This is conservative: it will traverse child pointers
   according to node kind and free strings and arrays. It will NOT try to free
   nodes reachable via the new `next` link unless they are reachable from a
   block's statements (to avoid double-free). Use ast_block_from_list before
   freeing a top-level linked-list to ensure the block owns the nodes. */
void ast_free(AST *node) {
    if (!node) return;

    switch (node->kind) {
        case AST_INT_LITERAL:
            free(node);
            break;
        case AST_ID:
            free(node->id);
            free(node);
            break;
        case AST_BINOP:
            ast_free(node->binop.left);
            ast_free(node->binop.right);
            free(node);
            break;
        case AST_DECL:
            free(node->decl.name);
            if (node->decl.init) ast_free(node->decl.init);
            free(node);
            break;
        case AST_FUNC:
            free(node->func.name);
            /* params are expected to be a linked list of decl nodes; free them */
            for (AST *p = node->func.params; p; ) {
                AST *next = p->next;
                ast_free(p);
                p = next;
            }
            if (node->func.body) ast_free(node->func.body);
            free(node);
            break;
        case AST_BLOCK:
            if (node->block.statements) {
                for (int i = 0; i < node->block.count; ++i) {
                    ast_free(node->block.statements[i]);
                }
                free(node->block.statements);
            }
            free(node);
            break;
        default:
            free(node);
            break;
    }
}
