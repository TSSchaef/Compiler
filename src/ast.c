#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

AST *ast_alloc() {
    AST *n = malloc(sizeof(AST));
    memset(n, 0, sizeof(AST));
    n->type = NULL;
    n->next = NULL;
    return n;
}

AST *ast_set_line_no(AST *node, int line_no){
    if (node) {
        node->line_no = line_no;
    }
    return node;
}

int ast_get_line_no(AST *node){
    if (node) {
        return node->line_no;
    }
    return -1;
}

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

AST *ast_float(double v){
    AST *n = malloc(sizeof(AST));
    memset(n, 0, sizeof(AST));
    n->kind = AST_FLOAT_LITERAL;
    n->floatval = v;
    n->type = NULL;
    n->next = NULL;
    return n;
}

AST *ast_string(char *s){
    AST *n = malloc(sizeof(AST));
    memset(n, 0, sizeof(AST));
    n->kind = AST_STRING_LITERAL;
    n->strval = strdup(s ? s : "");
    n->type = NULL;
    n->next = NULL;
    return n;
}

AST *ast_char(char c){
    AST *n = malloc(sizeof(AST));
    memset(n, 0, sizeof(AST));
    n->kind = AST_CHAR_LITERAL;
    n->charval = c;
    n->type = NULL;
    n->next = NULL;
    return n;
}

AST *ast_bool(bool b){
    AST *n = malloc(sizeof(AST));
    memset(n, 0, sizeof(AST));
    n->kind = AST_CHAR_LITERAL;
    n->boolval = b;
    n->type = NULL;
    n->next = NULL;
    return n;
}

AST *ast_binop(BinOpKind op, AST *l, AST *r) {
    AST *n = ast_alloc(); /* your helper: malloc+memset */
    n->kind = AST_BINOP;
    n->binop.op = op;
    n->binop.left = l;
    n->binop.right = r;
    return n;
}
AST *ast_assign(AssignOpKind op, AST *lhs, AST *rhs) {
    AST *n = ast_alloc();
    n->kind = AST_ASSIGN;
    n->assign.op = op;
    n->assign.lhs = lhs;
    n->assign.rhs = rhs;
    return n;
}

AST *ast_logical_or(AST *l, AST *r) {
    AST *n = ast_alloc();
    n->kind = AST_LOGICAL_OR;
    n->logical.left = l;
    n->logical.right = r;
    return n;
}

AST *ast_logical_and(AST *l, AST *r) {
    AST *n = ast_alloc();
    n->kind = AST_LOGICAL_AND;
    n->logical.left = l;
    n->logical.right = r;
    return n;
}

AST *ast_ternary(AST *cond, AST *t, AST *f) {
    AST *n = ast_alloc();
    n->kind = AST_TERNARY;
    n->ternary.cond = cond;
    n->ternary.iftrue = t;
    n->ternary.iffalse = f;
    return n;
}

AST *ast_unary(UnaryOpKind op, AST *operand) {
    AST *n = malloc(sizeof(AST));
    if (!n) return NULL;
    memset(n, 0, sizeof(AST));
    n->kind = AST_UNARY;
    n->unary.op = op;
    n->unary.operand = operand;
    n->unary.cast_type = NULL; /* only used for UOP_CAST */
    n->type = NULL;
    n->next = NULL;
    return n;
}

AST *ast_cast(struct Type *target, AST *operand) {
    AST *n = ast_unary(UOP_CAST, operand);
    if (!n) return NULL;
    n->unary.cast_type = target;
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

AST *ast_func(const char *name, struct Type *return_type, AST *params, AST *body){
    AST *n = malloc(sizeof(AST));
    memset(n, 0, sizeof(AST));
    n->kind = AST_FUNC;
    n->func.name = strdup(name ? name : "");
    n->func.return_type = return_type;
    n->func.params = params;
    n->func.body = body;
    n->type = NULL;
    n->next = NULL;
    return n;
}

AST *ast_func_call(AST *callee, AST *args) {
    AST *n = malloc(sizeof(AST));
    if (!n) return NULL;
    memset(n, 0, sizeof(AST));
    n->kind = AST_FUNC_CALL;
    n->call.callee = callee;
    n->call.args = args;
    n->call.arg_count = 0;
    n->type = NULL;
    n->next = NULL;

    /* optional: compute arg_count now */
    for (AST *p = args; p; p = p->next) n->call.arg_count++;

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

AST *ast_if(AST *cond, AST *then_branch, AST *else_branch) {
    AST *n = ast_alloc(); /* use your existing allocator helper */
    n->kind = AST_IF;
    n->if_stmt.cond = cond;
    n->if_stmt.then_branch = then_branch;
    n->if_stmt.else_branch = else_branch;
    return n;
}

AST *ast_while(AST *cond, AST *body) {
    AST *n = ast_alloc();
    n->kind = AST_WHILE;
    n->while_stmt.cond = cond;
    n->while_stmt.body = body;
    return n;
}

AST *ast_do_while(AST *body, AST *cond) {
    AST *n = ast_alloc();
    n->kind = AST_DO_WHILE;
    n->do_while.body = body;
    n->do_while.cond = cond;
    return n;
}

AST *ast_for(AST *init, AST *cond, AST *post, AST *body) {
    AST *n = ast_alloc();
    n->kind = AST_FOR;
    n->for_stmt.init = init;
    n->for_stmt.cond = cond;
    n->for_stmt.post = post;
    n->for_stmt.body = body;
    return n;
}

AST *ast_return(AST *expr) {
    AST *n = ast_alloc();
    n->kind = AST_RETURN;
    n->ret.expr = expr;
    return n;
}

AST *ast_break(void) {
    AST *n = ast_alloc();
    n->kind = AST_BREAK;
    return n;
}

AST *ast_continue(void) {
    AST *n = ast_alloc();
    n->kind = AST_CONTINUE;
    return n;
}



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
