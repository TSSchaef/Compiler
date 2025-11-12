#include "ast.h"

AST *ast_alloc() {
    AST *n = malloc(sizeof(AST));
    memset(n, 0, sizeof(AST));
    n->type = NULL;
    n->next = NULL;
    n->filename = strdup(getCurrentFileName());
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
    AST *n = ast_alloc();
    n->kind = AST_INT_LITERAL;
    n->intval = v;
    return n;
}

AST *ast_id(const char *name) {
    AST *n = ast_alloc();
    n->kind = AST_ID;
    n->id = strdup(name ? name : "");
    return n;
}

AST *ast_float(double v){
    AST *n = ast_alloc();
    n->kind = AST_FLOAT_LITERAL;
    n->floatval = v;
    return n;
}

AST *ast_string(char *s){
    AST *n = ast_alloc();
    n->kind = AST_STRING_LITERAL;
    n->strval = strdup(s ? s : "");
    return n;
}

AST *ast_char(char c){
    AST *n = ast_alloc();
    n->kind = AST_CHAR_LITERAL;
    n->charval = c;
    return n;
}

AST *ast_bool(bool b){
    AST *n = ast_alloc();
    n->kind = AST_CHAR_LITERAL;
    n->boolval = b;
    return n;
}

AST *ast_array_access(AST *array, AST *index){
    AST *n = ast_alloc();
    n->kind = AST_ARRAY_ACCESS;
    n->line_no = array->line_no; /* inherit line number from array expr */
    n->array.array = array;
    n->array.index = index;
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
    AST *n = ast_alloc();
    if (!n) return NULL;
    n->kind = AST_UNARY;
    n->unary.op = op;
    n->unary.operand = operand;
    n->unary.cast_type = NULL; /* only used for UOP_CAST */
    return n;
}

AST *ast_cast(struct Type *target, AST *operand) {
    AST *n = ast_unary(UOP_CAST, operand);
    if (!n) return NULL;
    n->unary.cast_type = target;
    return n;
}

AST *ast_decl(const char *name, struct Type *decl_type, AST *init) {
    AST *n = ast_alloc();
    n->kind = AST_DECL;
    n->decl.name = strdup(name ? name : "");
    n->decl.decl_type = decl_type;
    n->decl.init = init;
    return n;
}

AST *ast_func(const char *name, struct Type *return_type, AST *params, AST *body){
    AST *n = ast_alloc();
    n->kind = AST_FUNC;
    n->func.name = strdup(name ? name : "");
    n->func.return_type = return_type;
    n->func.params = params;
    n->func.body = body;
    return n;
}

AST *ast_func_call(AST *callee, AST *args) {
    AST *n = ast_alloc();
    n->kind = AST_FUNC_CALL;
    n->call.callee = callee;
    n->call.args = args;
    n->call.arg_count = 0;

    /* optional: compute arg_count now */
    for (AST *p = args; p; p = p->next) n->call.arg_count++;

    return n;
}

AST *ast_block(AST **statements, int count) {
    AST *n = ast_alloc();
    n->kind = AST_BLOCK;
    n->block.statements = statements;
    n->block.count = count;
    return n;
}

AST *ast_member_access(AST *object, const char *member_name) {
    AST *n = ast_alloc();
    n->kind = AST_MEMBER_ACCESS;
    n->line_no = object->line_no; /* inherit line number from object expr */
    n->member.object = object;
    n->member.member_name = strdup(member_name ? member_name : "");
    return n;
}

AST *ast_struct_def(const char *name, AST *members) {
    AST *n = ast_alloc();
    n->kind = AST_STRUCT_DEF;
    n->struct_def.name = strdup(name ? name : "");
    n->struct_def.members = members;
    return n;
}


AST *ast_list_prepend(AST *node, AST *head) {
    if (!head) return node;
    if (!node) return head;

    // Find the last node in the list being prepended
    AST *last = node;
    int count = 0;

    while (last->next) {
        last = last->next;

        count++;
        if (count > 10000) {  // Safety check
            fprintf(stderr, "ERROR: Infinite loop detected in ast_list_prepend!\n");
            fprintf(stderr, "Node kind: %d, at line %d\n", node->kind, node->line_no);
            exit(1);
        }
    }

    // Connect the last node of the prepended list to the head
    last->next = head;
    return node;
}

AST *ast_list_append(AST *node, AST *head){
    if (!head) return node;
    AST *p = head;
    while (p->next) p = p->next;
    p->next = node;
    return head;
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

static const char *binop_to_string(BinOpKind op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_MOD: return "%";
        case OP_BIT_AND: return "&";
        case OP_BIT_OR: return "|";
        case OP_BIT_XOR: return "^";
        case OP_SHL: return "<<";
        case OP_SHR: return ">>";
        case OP_EQ: return "==";
        case OP_NEQ: return "!=";
        case OP_LT: return "<";
        case OP_GT: return ">";
        case OP_LE: return "<=";
        case OP_GE: return ">=";
        default: return "?";
    }
}

static const char *unaryop_to_string(UnaryOpKind op) {
    switch (op) {
        case UOP_PLUS: return "+";
        case UOP_NEG: return "-";
        case UOP_PRE_INC: return "++";
        case UOP_PRE_DEC: return "--";
        case UOP_POST_INC: return "++";
        case UOP_POST_DEC: return "--";
        case UOP_ADDR: return "&";
        case UOP_DEREF: return "*";
        case UOP_LOGICAL_NOT: return "!";
        case UOP_BITWISE_NOT: return "~";
        case UOP_CAST: return "(cast)";
        default: return "?";
    }
}

static const char *assignop_to_string(AssignOpKind op) {
    switch (op) {
        case AOP_ASSIGN: return "=";
        case AOP_ADD_ASSIGN: return "+=";
        case AOP_SUB_ASSIGN: return "-=";
        case AOP_MUL_ASSIGN: return "*=";
        case AOP_DIV_ASSIGN: return "/=";
        case AOP_MOD_ASSIGN: return "%=";
        case AOP_AND_ASSIGN: return "&=";
        case AOP_OR_ASSIGN: return "|=";
        case AOP_XOR_ASSIGN: return "^=";
        case AOP_SHL_ASSIGN: return "<<=";
        case AOP_SHR_ASSIGN: return ">>=";
        default: return "?";
    }
}

static void ast_print_helper(AST *node, int indent) {
    if (!node) {
        ast_print_indent(indent);
        printf("(null)\n");
        return;
    }

    ast_print_indent(indent);

    switch (node->kind) {
        case AST_INT_LITERAL:
            printf("INT_LITERAL: %d\n", node->intval);
            break;

        case AST_FLOAT_LITERAL:
            printf("FLOAT_LITERAL: %f\n", node->floatval);
            break;

        case AST_STRING_LITERAL:
            printf("STRING_LITERAL: \"%s\"\n", node->strval ? node->strval : "");
            break;

        case AST_CHAR_LITERAL:
            printf("CHAR_LITERAL: '%c'\n", node->charval);
            break;

        case AST_BOOL_LITERAL:
            printf("BOOL_LITERAL: %s\n", node->boolval ? "true" : "false");
            break;

        case AST_ID:
            printf("ID: ");
            if (node->id) {
                // Safely print the string, checking for valid characters
                bool valid = true;
                for (int i = 0; node->id[i] != '\0' && i < 1000; i++) {
                    if (node->id[i] < 32 || node->id[i] > 126) {
                        if (node->id[i] != '\0') {
                            valid = false;
                            break;
                        }
                    }
                }
                if (valid) {
                    printf("%s", node->id);
                } else {
                    printf("(corrupted: %p)", (void*)node->id);
                }
            } else {
                printf("(null)");
            }
            printf("\n");
            break;

        case AST_ARRAY_ACCESS:
            printf("ARRAY_ACCESS\n");
            ast_print_indent(indent + 2);
            printf("array:\n");
            ast_print_helper(node->array.array, indent + 4);
            ast_print_indent(indent + 2);
            printf("index:\n");
            ast_print_helper(node->array.index, indent + 4);
            break;

        case AST_BINOP:
            printf("BINOP: %s\n", binop_to_string(node->binop.op));
            ast_print_helper(node->binop.left, indent + 2);
            ast_print_helper(node->binop.right, indent + 2);
            break;

        case AST_ASSIGN:
            printf("ASSIGN: %s\n", assignop_to_string(node->assign.op));
            ast_print_helper(node->assign.lhs, indent + 2);
            ast_print_helper(node->assign.rhs, indent + 2);
            break;

        case AST_LOGICAL_OR:
            printf("LOGICAL_OR\n");
            ast_print_helper(node->logical.left, indent + 2);
            ast_print_helper(node->logical.right, indent + 2);
            break;

        case AST_LOGICAL_AND:
            printf("LOGICAL_AND\n");
            ast_print_helper(node->logical.left, indent + 2);
            ast_print_helper(node->logical.right, indent + 2);
            break;

        case AST_TERNARY:
            printf("TERNARY\n");
            ast_print_indent(indent + 2);
            printf("condition:\n");
            ast_print_helper(node->ternary.cond, indent + 4);
            ast_print_indent(indent + 2);
            printf("if_true:\n");
            ast_print_helper(node->ternary.iftrue, indent + 4);
            ast_print_indent(indent + 2);
            printf("if_false:\n");
            ast_print_helper(node->ternary.iffalse, indent + 4);
            break;

        case AST_UNARY:
            printf("UNARY: %s", unaryop_to_string(node->unary.op));
            if (node->unary.op == UOP_POST_INC || node->unary.op == UOP_POST_DEC) {
                printf(" (postfix)");
            }
            printf("\n");
            ast_print_helper(node->unary.operand, indent + 2);
            break;

        case AST_DECL:
            printf("DECL: ");
            if (node->decl.name) {
                printf("%s", node->decl.name);
            }
            printf("\n");
            if (node->decl.init) {
                ast_print_indent(indent + 2);
                printf("init:\n");
                ast_print_helper(node->decl.init, indent + 4);
            }
            break;

        case AST_FUNC:
            printf("FUNC: ");
            if (node->func.name) {
                // Safely validate and print the string
                bool valid = true;
                for (int i = 0; node->func.name[i] != '\0' && i < 1000; i++) {
                    if (node->func.name[i] < 32 || node->func.name[i] > 126) {
                        if (node->func.name[i] != '\0') {
                            valid = false;
                            break;
                        }
                    }
                }
                if (valid) {
                    printf("%s", node->func.name);
                } else {
                    printf("(corrupted: %p)", (void*)node->func.name);
                }
            } else {
                printf("(null)");
            }
            printf("\n");
            if (node->func.params) {
                ast_print_indent(indent + 2);
                printf("params:\n");
                for (AST *p = node->func.params; p; p = p->next) {
                    ast_print_helper(p, indent + 4);
                }
            }
            if (node->func.body) {
                ast_print_indent(indent + 2);
                printf("body:\n");
                ast_print_helper(node->func.body, indent + 4);
            }
            break;

        case AST_FUNC_CALL:
            printf("FUNC_CALL\n");
            ast_print_indent(indent + 2);
            printf("callee:\n");
            ast_print_helper(node->call.callee, indent + 4);
            if (node->call.args) {
                ast_print_indent(indent + 2);
                printf("arguments (%d):\n", node->call.arg_count);
                for (AST *arg = node->call.args; arg; arg = arg->next) {
                    ast_print_helper(arg, indent + 4);
                }
            }
            break;

        case AST_BLOCK:
            printf("BLOCK (count=%d)\n", node->block.count);
            for (int i = 0; i < node->block.count; ++i) {
                ast_print_helper(node->block.statements[i], indent + 2);
            }
            break;

        case AST_MEMBER_ACCESS:
            printf("MEMBER_ACCESS\n");
            ast_print_indent(indent + 2);
            printf("object:\n");
            ast_print_helper(node->member.object, indent + 4);
            ast_print_indent(indent + 2);
            printf("member: %s\n", node->member.member_name ? node->member.member_name : "(null)");
            break;

        case AST_STRUCT_DEF:
            printf("STRUCT_DEF: %s\n", node->struct_def.name ? node->struct_def.name : "(null)");
            if (node->struct_def.members) {
                ast_print_indent(indent + 2);
                printf("members:\n");
                for (AST *m = node->struct_def.members; m; m = m->next) {
                    ast_print_helper(m, indent + 4);
                }
            }
            break;

        case AST_IF:
            printf("IF\n");
            ast_print_indent(indent + 2);
            printf("condition:\n");
            ast_print_helper(node->if_stmt.cond, indent + 4);
            ast_print_indent(indent + 2);
            printf("then:\n");
            ast_print_helper(node->if_stmt.then_branch, indent + 4);
            if (node->if_stmt.else_branch) {
                ast_print_indent(indent + 2);
                printf("else:\n");
                ast_print_helper(node->if_stmt.else_branch, indent + 4);
            }
            break;

        case AST_WHILE:
            printf("WHILE\n");
            ast_print_indent(indent + 2);
            printf("condition:\n");
            ast_print_helper(node->while_stmt.cond, indent + 4);
            ast_print_indent(indent + 2);
            printf("body:\n");
            ast_print_helper(node->while_stmt.body, indent + 4);
            break;

        case AST_DO_WHILE:
            printf("DO_WHILE\n");
            ast_print_indent(indent + 2);
            printf("body:\n");
            ast_print_helper(node->do_while.body, indent + 4);
            ast_print_indent(indent + 2);
            printf("condition:\n");
            ast_print_helper(node->do_while.cond, indent + 4);
            break;

        case AST_FOR:
            printf("FOR\n");
            if (node->for_stmt.init) {
                ast_print_indent(indent + 2);
                printf("init:\n");
                ast_print_helper(node->for_stmt.init, indent + 4);
            }
            if (node->for_stmt.cond) {
                ast_print_indent(indent + 2);
                printf("condition:\n");
                ast_print_helper(node->for_stmt.cond, indent + 4);
            }
            if (node->for_stmt.post) {
                ast_print_indent(indent + 2);
                printf("post:\n");
                ast_print_helper(node->for_stmt.post, indent + 4);
            }
            ast_print_indent(indent + 2);
            printf("body:\n");
            ast_print_helper(node->for_stmt.body, indent + 4);
            break;

        case AST_RETURN:
            printf("RETURN\n");
            if (node->ret.expr) {
                ast_print_helper(node->ret.expr, indent + 2);
            }
            break;

        case AST_BREAK:
            printf("BREAK\n");
            break;

        case AST_CONTINUE:
            printf("CONTINUE\n");
            break;

        case AST_SWITCH:
            printf("SWITCH (not fully implemented)\n");
            break;

        case AST_CASE:
            printf("CASE (not fully implemented)\n");
            break;

        default:
            printf("UNKNOWN AST node kind %d\n", node->kind);
            break;
    }
}

void ast_print(AST *node) {
    if (!node) {
        printf("(null)\n");
        return;
    }
    ast_print_helper(node, 0);
}

void ast_free(AST *node) {
    if (!node) return;

    switch (node->kind) {
        case AST_INT_LITERAL:
        case AST_FLOAT_LITERAL:
        case AST_CHAR_LITERAL:
        case AST_BOOL_LITERAL:
            free(node);
            break;

        case AST_STRING_LITERAL:
            free(node->strval);
            free(node);
            break;

        case AST_ID:
            free(node->id);
            free(node);
            break;

        case AST_ARRAY_ACCESS:
            ast_free(node->array.array);
            ast_free(node->array.index);
            free(node);
            break;

        case AST_BINOP:
            ast_free(node->binop.left);
            ast_free(node->binop.right);
            free(node);
            break;

        case AST_ASSIGN:
            ast_free(node->assign.lhs);
            ast_free(node->assign.rhs);
            free(node);
            break;

        case AST_LOGICAL_OR:
        case AST_LOGICAL_AND:
            ast_free(node->logical.left);
            ast_free(node->logical.right);
            free(node);
            break;

        case AST_TERNARY:
            ast_free(node->ternary.cond);
            ast_free(node->ternary.iftrue);
            ast_free(node->ternary.iffalse);
            free(node);
            break;

        case AST_UNARY:
            ast_free(node->unary.operand);
            free(node);
            break;

        case AST_DECL:
            free(node->decl.name);
            if (node->decl.init) ast_free(node->decl.init);
            free(node);
            break;

        case AST_FUNC:
            free(node->func.name);
            // Free parameters (linked list)
            for (AST *p = node->func.params; p; ) {
                AST *next = p->next;
                ast_free(p);
                p = next;
            }
            if (node->func.body) ast_free(node->func.body);
            free(node);
            break;

        case AST_FUNC_CALL:
            ast_free(node->call.callee);
            // Free arguments (linked list)
            for (AST *arg = node->call.args; arg; ) {
                AST *next = arg->next;
                ast_free(arg);
                arg = next;
            }
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

        case AST_MEMBER_ACCESS:
            ast_free(node->member.object);
            free(node->member.member_name);
            free(node);
            break;

        case AST_STRUCT_DEF:
            free(node->struct_def.name);
            // Free members (linked list)
            for (AST *m = node->struct_def.members; m; ) {
                AST *next = m->next;
                ast_free(m);
                m = next;
            }
            free(node);
            break;

        case AST_IF:
            ast_free(node->if_stmt.cond);
            ast_free(node->if_stmt.then_branch);
            if (node->if_stmt.else_branch) {
                ast_free(node->if_stmt.else_branch);
            }
            free(node);
            break;

        case AST_WHILE:
            ast_free(node->while_stmt.cond);
            ast_free(node->while_stmt.body);
            free(node);
            break;

        case AST_DO_WHILE:
            ast_free(node->do_while.body);
            ast_free(node->do_while.cond);
            free(node);
            break;

        case AST_FOR:
            if (node->for_stmt.init) ast_free(node->for_stmt.init);
            if (node->for_stmt.cond) ast_free(node->for_stmt.cond);
            if (node->for_stmt.post) ast_free(node->for_stmt.post);
            ast_free(node->for_stmt.body);
            free(node);
            break;

        case AST_RETURN:
            if (node->ret.expr) ast_free(node->ret.expr);
            free(node);
            break;

        case AST_BREAK:
        case AST_CONTINUE:
            free(node);
            break;

        case AST_SWITCH:
        case AST_CASE:
            // Not fully implemented - add proper cleanup if needed
            free(node);
            break;

        default:
            free(node);
            break;
    }
}
