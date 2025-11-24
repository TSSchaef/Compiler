#include "ir.h"

void irlist_init(IRList *l) {
    l->head = NULL;
    l->tail = NULL;
}

void ir_emit(IRList *l, IRKind k, const char *s, int i) {
    IRInstruction *n = calloc(1, sizeof(*n));
    n->kind = k;
    n->s = s;
    n->i = i;
    n->f = 0.0f;

    if (!l->head)
        l->head = l->tail = n;
    else {
        l->tail->next = n;
        l->tail = n;
    }
}

void ir_emit_float(IRList *l, IRKind k, float f) {
    IRInstruction *n = calloc(1, sizeof(*n));
    n->kind = k;
    n->s = NULL;
    n->i = 0;
    n->f = f;

    if (!l->head)
        l->head = l->tail = n;
    else {
        l->tail->next = n;
        l->tail = n;
    }
}

static void gen_expr(AST *n, IRList *out);

static void gen_binop(AST *n, IRList *out) {
    gen_expr(n->binop.left, out);
    gen_expr(n->binop.right, out);
    
    switch(n->binop.op) {
        case OP_ADD: ir_emit(out, IR_ADD, NULL, 0); break;
        case OP_SUB: ir_emit(out, IR_SUB, NULL, 0); break;
        case OP_MUL: ir_emit(out, IR_MUL, NULL, 0); break;
        case OP_DIV: ir_emit(out, IR_DIV, NULL, 0); break;
        case OP_MOD: ir_emit(out, IR_MOD, NULL, 0); break;
        case OP_BIT_AND: ir_emit(out, IR_BIT_AND, NULL, 0); break;
        case OP_BIT_OR: ir_emit(out, IR_BIT_OR, NULL, 0); break;
        case OP_BIT_XOR: ir_emit(out, IR_BIT_XOR, NULL, 0); break;
        case OP_SHL: ir_emit(out, IR_SHL, NULL, 0); break;
        case OP_SHR: ir_emit(out, IR_SHR, NULL, 0); break;
        case OP_EQ: ir_emit(out, IR_EQ, NULL, 0); break;
        case OP_NEQ: ir_emit(out, IR_NEQ, NULL, 0); break;
        case OP_LT: ir_emit(out, IR_LT, NULL, 0); break;
        case OP_GT: ir_emit(out, IR_GT, NULL, 0); break;
        case OP_LE: ir_emit(out, IR_LE, NULL, 0); break;
        case OP_GE: ir_emit(out, IR_GE, NULL, 0); break;
        default: break;
    }
}

static void gen_unary(AST *n, IRList *out) {
    gen_expr(n->unary.operand, out);
    
    switch(n->unary.op) {
        case UOP_NEG:
            ir_emit(out, IR_NEG, NULL, 0);
            break;
        case UOP_BITWISE_NOT:
            ir_emit(out, IR_BIT_NOT, NULL, 0);
            break;
        case UOP_LOGICAL_NOT:
            // !x is equivalent to x == 0
            ir_emit(out, IR_PUSH_INT, NULL, 0);
            ir_emit(out, IR_EQ, NULL, 0);
            break;
        case UOP_CAST:
            // Handle type casts
            if (n->unary.cast_type && n->unary.operand->type) {
                Type *from = n->unary.operand->type;
                Type *to = n->unary.cast_type;
                
                if (from->kind == TY_INT && to->kind == TY_FLT)
                    ir_emit(out, IR_CAST_I2F, NULL, 0);
                else if (from->kind == TY_FLT && to->kind == TY_INT)
                    ir_emit(out, IR_CAST_F2I, NULL, 0);
            }
            break;
        case UOP_PRE_INC:
        case UOP_POST_INC:
            // For x++: load x, dup if post-inc, push 1, add, store
            if (n->unary.operand->kind == AST_ID) {
                if (n->unary.op == UOP_POST_INC) {
                    ir_emit(out, IR_DUP, NULL, 0);
                }
                ir_emit(out, IR_PUSH_INT, NULL, 1);
                ir_emit(out, IR_ADD, NULL, 0);
                ir_emit(out, IR_STORE_GLOBAL, n->unary.operand->id, 0);
                if (n->unary.op == UOP_PRE_INC) {
                    ir_emit(out, IR_LOAD_GLOBAL, n->unary.operand->id, 0);
                }
            }
            break;
        case UOP_PRE_DEC:
        case UOP_POST_DEC:
            if (n->unary.operand->kind == AST_ID) {
                if (n->unary.op == UOP_POST_DEC) {
                    ir_emit(out, IR_DUP, NULL, 0);
                }
                ir_emit(out, IR_PUSH_INT, NULL, 1);
                ir_emit(out, IR_SUB, NULL, 0);
                ir_emit(out, IR_STORE_GLOBAL, n->unary.operand->id, 0);
                if (n->unary.op == UOP_PRE_DEC) {
                    ir_emit(out, IR_LOAD_GLOBAL, n->unary.operand->id, 0);
                }
            }
            break;
        default:
            break;
    }
}

static void gen_assign(AST *n, IRList *out) {
    // Handle compound assignment operators
    if (n->assign.op != AOP_ASSIGN) {
        // For compound assignment like +=, we need to load, operate, then store
        if (n->assign.lhs->kind == AST_ID) {
            ir_emit(out, IR_LOAD_GLOBAL, n->assign.lhs->id, 0);
            gen_expr(n->assign.rhs, out);
            
            switch(n->assign.op) {
                case AOP_ADD_ASSIGN: ir_emit(out, IR_ADD, NULL, 0); break;
                case AOP_SUB_ASSIGN: ir_emit(out, IR_SUB, NULL, 0); break;
                case AOP_MUL_ASSIGN: ir_emit(out, IR_MUL, NULL, 0); break;
                case AOP_DIV_ASSIGN: ir_emit(out, IR_DIV, NULL, 0); break;
                case AOP_MOD_ASSIGN: ir_emit(out, IR_MOD, NULL, 0); break;
                case AOP_AND_ASSIGN: ir_emit(out, IR_BIT_AND, NULL, 0); break;
                case AOP_OR_ASSIGN: ir_emit(out, IR_BIT_OR, NULL, 0); break;
                case AOP_XOR_ASSIGN: ir_emit(out, IR_BIT_XOR, NULL, 0); break;
                case AOP_SHL_ASSIGN: ir_emit(out, IR_SHL, NULL, 0); break;
                case AOP_SHR_ASSIGN: ir_emit(out, IR_SHR, NULL, 0); break;
                default: break;
            }
            
            ir_emit(out, IR_STORE_GLOBAL, n->assign.lhs->id, 0);
            ir_emit(out, IR_LOAD_GLOBAL, n->assign.lhs->id, 0);
        }
    } else {
        // Simple assignment
        gen_expr(n->assign.rhs, out);
        
        if (n->assign.lhs->kind == AST_ID) {
            ir_emit(out, IR_DUP, NULL, 0);
            ir_emit(out, IR_STORE_GLOBAL, n->assign.lhs->id, 0);
        }
    }
}

static void gen_call(AST *n, IRList *out) {
    // Generate code for each argument
    AST *arg = n->call.args;
    while (arg) {
        gen_expr(arg, out);
        arg = arg->next;
    }
    
    // Get function name
    const char *func_name = NULL;
    if (n->call.callee->kind == AST_ID) {
        func_name = n->call.callee->id;
    }
    
    if (func_name) {
        ir_emit(out, IR_CALL, func_name, n->call.arg_count);
    }
}

static void gen_expr(AST *n, IRList *out) {
    if (!n) return;
    
    switch(n->kind) {
        case AST_INT_LITERAL:
            ir_emit(out, IR_PUSH_INT, NULL, n->intval);
            break;
            
        case AST_FLOAT_LITERAL:
            ir_emit_float(out, IR_PUSH_FLOAT, n->floatval);
            break;
            
        case AST_STRING_LITERAL:
            ir_emit(out, IR_PUSH_STRING, n->strval, 0);
            break;
            
        case AST_CHAR_LITERAL:
            ir_emit(out, IR_PUSH_INT, NULL, (int)n->charval);
            break;
            
        case AST_BOOL_LITERAL:
            ir_emit(out, IR_PUSH_INT, NULL, n->boolval ? 1 : 0);
            break;
            
        case AST_ID:
            ir_emit(out, IR_LOAD_GLOBAL, n->id, 0);
            break;
            
        case AST_BINOP:
            gen_binop(n, out);
            break;
            
        case AST_ASSIGN:
            gen_assign(n, out);
            break;
            
        case AST_UNARY:
            gen_unary(n, out);
            break;
            
        case AST_FUNC_CALL:
            gen_call(n, out);
            break;
            
        case AST_LOGICAL_OR:
        case AST_LOGICAL_AND:
            // For phase 5, we'll generate simple (non-short-circuit) code
            gen_expr(n->logical.left, out);
            gen_expr(n->logical.right, out);
            if (n->kind == AST_LOGICAL_OR)
                ir_emit(out, IR_BIT_OR, NULL, 0);
            else
                ir_emit(out, IR_BIT_AND, NULL, 0);
            break;
            
        default:
            break;
    }
}

static void gen_stmt(AST *n, IRList *out) {
    if (!n) return;
    
    switch(n->kind) {
        case AST_RETURN:
            if (n->ret.expr) {
                gen_expr(n->ret.expr, out);
                ir_emit(out, IR_RETURN, NULL, 0);
            } else {
                ir_emit(out, IR_RETURN_VOID, NULL, 0);
            }
            break;
            
        case AST_BLOCK:
            for (int i = 0; i < n->block.count; i++) {
                gen_stmt(n->block.statements[i], out);
            }
            break;
            
        case AST_FUNC_CALL:
            gen_expr(n, out);
            // Pop result if not used
            ir_emit(out, IR_POP, NULL, 0);
            break;
            
        case AST_ASSIGN:
        case AST_UNARY:
            // Expression statements
            gen_expr(n, out);
            ir_emit(out, IR_POP, NULL, 0);
            break;
            
        default:
            break;
    }
}

void generate_ir_from_ast(AST *ast, IRList *out) {
    if (!ast) return;
    
    irlist_init(out);
    
    if (ast->kind == AST_FUNC) {
        // Generate IR for function body
        if (ast->func.body) {
            gen_stmt(ast->func.body, out);
        }
    } else {
        gen_stmt(ast, out);
    }
}
