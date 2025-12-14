#include "ir.h"
#include "symtab.h"

static int label_counter = 0;

// Stack to track break/continue labels for nested loops
#define MAX_LOOP_DEPTH 32
static struct {
    char *break_label;
    char *continue_label;
} loop_stack[MAX_LOOP_DEPTH];
static int loop_depth = 0;

// Helper to generate unique labels
static char* gen_label() {
    static char buf[32];
    snprintf(buf, sizeof(buf), "L%d", label_counter++);
    return strdup(buf);
}

static void push_loop(char *break_label, char *continue_label) {
    if (loop_depth < MAX_LOOP_DEPTH) {
        loop_stack[loop_depth].break_label = break_label;
        loop_stack[loop_depth].continue_label = continue_label;
        loop_depth++;
    }
}

static void pop_loop() {
    if (loop_depth > 0) {
        loop_depth--;
    }
}

static char* get_break_label() {
    return (loop_depth > 0) ? loop_stack[loop_depth - 1].break_label : NULL;
}

static char* get_continue_label() {
    return (loop_depth > 0) ? loop_stack[loop_depth - 1].continue_label : NULL;
}

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
    n->symbol = NULL;

    if (!l->head)
        l->head = l->tail = n;
    else {
        l->tail->next = n;
        l->tail = n;
    }
}

void ir_emit_with_symbol(IRList *l, IRKind k, const char *s, int i, Symbol *sym) {
    IRInstruction *n = calloc(1, sizeof(*n));
    n->kind = k;
    n->s = s;
    n->i = i;
    n->f = 0.0f;
    n->symbol = sym;

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
    n->symbol = NULL;

    if (!l->head)
        l->head = l->tail = n;
    else {
        l->tail->next = n;
        l->tail = n;
    }
}

// Helper to create a dummy symbol with just type information
static Symbol* create_type_symbol(Type *t) {
    Symbol *s = calloc(1, sizeof(Symbol));
    s->name = NULL;
    s->type = t;
    s->is_local = false;
    s->local_index = -1;
    return s;
}

// Print IR in readable format for debugging
void ir_print(IRList *ir, FILE *out) {
    if (!ir || !out) return;
    
    fprintf(out, "=== IR Instructions ===\n");
    int count = 0;
    for (IRInstruction *p = ir->head; p; p = p->next) {
        fprintf(out, "%3d: ", count++);
        
        switch(p->kind) {
            case IR_NOP:
                fprintf(out, "NOP\n");
                break;
            case IR_LABEL:
                fprintf(out, "LABEL %s\n", p->s ? p->s : "?");
                break;
            case IR_JUMP:
                fprintf(out, "JUMP %s\n", p->s ? p->s : "?");
                break;
            case IR_JUMP_IF_ZERO:
                fprintf(out, "JUMP_IF_ZERO %s\n", p->s ? p->s : "?");
                break;
            case IR_LOAD_GLOBAL:
                fprintf(out, "LOAD_GLOBAL %s\n", p->s ? p->s : "?");
                break;
            case IR_STORE_GLOBAL:
                fprintf(out, "STORE_GLOBAL %s\n", p->s ? p->s : "?");
                break;
            case IR_LOAD_LOCAL:
                fprintf(out, "LOAD_LOCAL %d\n", p->i);
                break;
            case IR_STORE_LOCAL:
                fprintf(out, "STORE_LOCAL %d\n", p->i);
                break;
            case IR_PUSH_INT:
                fprintf(out, "PUSH_INT %d\n", p->i);
                break;
            case IR_PUSH_FLOAT:
                fprintf(out, "PUSH_FLOAT %f\n", p->f);
                break;
            case IR_PUSH_STRING:
                fprintf(out, "PUSH_STRING %s\n", p->s ? p->s : "");
                break;
            case IR_ADD:
                fprintf(out, "ADD\n");
                break;
            case IR_SUB:
                fprintf(out, "SUB\n");
                break;
            case IR_MUL:
                fprintf(out, "MUL\n");
                break;
            case IR_DIV:
                fprintf(out, "DIV\n");
                break;
            case IR_MOD:
                fprintf(out, "MOD\n");
                break;
            case IR_NEG:
                fprintf(out, "NEG\n");
                break;
            case IR_BIT_AND:
                fprintf(out, "BIT_AND\n");
                break;
            case IR_BIT_OR:
                fprintf(out, "BIT_OR\n");
                break;
            case IR_BIT_XOR:
                fprintf(out, "BIT_XOR\n");
                break;
            case IR_BIT_NOT:
                fprintf(out, "BIT_NOT\n");
                break;
            case IR_SHL:
                fprintf(out, "SHL\n");
                break;
            case IR_SHR:
                fprintf(out, "SHR\n");
                break;
            case IR_EQ:
                fprintf(out, "EQ\n");
                break;
            case IR_NEQ:
                fprintf(out, "NEQ\n");
                break;
            case IR_LT:
                fprintf(out, "LT\n");
                break;
            case IR_GT:
                fprintf(out, "GT\n");
                break;
            case IR_LE:
                fprintf(out, "LE\n");
                break;
            case IR_GE:
                fprintf(out, "GE\n");
                break;
            case IR_CALL:
                fprintf(out, "CALL %s (argc=%d)\n", p->s ? p->s : "?", p->i);
                break;
            case IR_RETURN:
                fprintf(out, "RETURN\n");
                break;
            case IR_RETURN_VOID:
                fprintf(out, "RETURN_VOID\n");
                break;
            case IR_POP:
                fprintf(out, "POP\n");
                break;
            case IR_DUP:
                fprintf(out, "DUP\n");
                break;
            case IR_DUP2:
                fprintf(out, "DUP2\n");
                break;
            case IR_DUP_X2:
                fprintf(out, "DUP_X2\n");
                break;
            case IR_CAST_I2F:
                fprintf(out, "CAST_I2F\n");
                break;
            case IR_CAST_F2I:
                fprintf(out, "CAST_F2I\n");
                break;
            case IR_CAST_I2D:
                fprintf(out, "CAST_I2D\n");
                break;
            case IR_CAST_D2I:
                fprintf(out, "CAST_D2I\n");
                break;
            case IR_CAST_F2D:
                fprintf(out, "CAST_F2D\n");
                break;
            case IR_CAST_D2F:
                fprintf(out, "CAST_D2F\n");
                break;
            case IR_ARRAY_LOAD:
                fprintf(out, "ARRAY_LOAD\n");
                break;
            case IR_ARRAY_STORE:
                fprintf(out, "ARRAY_STORE\n");
                break;
            case IR_ALLOC_ARRAY:
                fprintf(out, "ALLOC_ARRAY size=%d\n", p->i);
                break;
            default:
                fprintf(out, "UNKNOWN(%d)\n", p->kind);
                break;
        }
    }
    fprintf(out, "=== End IR ===\n\n");
}

static void gen_expr(AST *n, IRList *out);
static void gen_stmt(AST *n, IRList *out);

static void gen_binop(AST *n, IRList *out) {
    gen_expr(n->binop.left, out);
    gen_expr(n->binop.right, out);

    // Pass result type with the operation
    Symbol *type_sym = create_type_symbol(n->type);

    switch(n->binop.op) {
        case OP_ADD: ir_emit_with_symbol(out, IR_ADD, NULL, 0, type_sym); break;
        case OP_SUB: ir_emit_with_symbol(out, IR_SUB, NULL, 0, type_sym); break;
        case OP_MUL: ir_emit_with_symbol(out, IR_MUL, NULL, 0, type_sym); break;
        case OP_DIV: ir_emit_with_symbol(out, IR_DIV, NULL, 0, type_sym); break;
        case OP_MOD: ir_emit_with_symbol(out, IR_MOD, NULL, 0, type_sym); break;
        case OP_BIT_AND: ir_emit(out, IR_BIT_AND, NULL, 0); break;
        case OP_BIT_OR: ir_emit(out, IR_BIT_OR, NULL, 0); break;
        case OP_BIT_XOR: ir_emit(out, IR_BIT_XOR, NULL, 0); break;
        case OP_SHL: ir_emit(out, IR_SHL, NULL, 0); break;
        case OP_SHR: ir_emit(out, IR_SHR, NULL, 0); break;
                     // For comparisons, pass the LEFT operand type (what we're comparing)
        case OP_EQ: ir_emit_with_symbol(out, IR_EQ, NULL, 0, create_type_symbol(n->binop.left->type)); break;
        case OP_NEQ: ir_emit_with_symbol(out, IR_NEQ, NULL, 0, create_type_symbol(n->binop.left->type)); break;
        case OP_LT: ir_emit_with_symbol(out, IR_LT, NULL, 0, create_type_symbol(n->binop.left->type)); break;
        case OP_GT: ir_emit_with_symbol(out, IR_GT, NULL, 0, create_type_symbol(n->binop.left->type)); break;
        case OP_LE: ir_emit_with_symbol(out, IR_LE, NULL, 0, create_type_symbol(n->binop.left->type)); break;
        case OP_GE: ir_emit_with_symbol(out, IR_GE, NULL, 0, create_type_symbol(n->binop.left->type)); break;
        default: break;
    }
}

static void gen_unary(AST *n, IRList *out) {
    gen_expr(n->unary.operand, out);
    
    Symbol *type_sym = create_type_symbol(n->type);
    
    switch(n->unary.op) {
        case UOP_NEG:
            ir_emit_with_symbol(out, IR_NEG, NULL, 0, type_sym);
            break;
        case UOP_BITWISE_NOT:
            ir_emit(out, IR_BIT_NOT, NULL, 0);
            break;
        case UOP_LOGICAL_NOT:
            ir_emit(out, IR_PUSH_INT, NULL, 0);
            ir_emit(out, IR_EQ, NULL, 0);
            break;
        case UOP_CAST:
            if (n->unary.cast_type && n->unary.operand->type) {
                Type *from = n->unary.operand->type;
                Type *to = n->unary.cast_type;
                
                if (from->kind == TY_INT && to->kind == TY_FLT) {
                    ir_emit(out, IR_CAST_I2F, NULL, 0);
                } else if (from->kind == TY_FLT && to->kind == TY_INT) {
                    ir_emit(out, IR_CAST_F2I, NULL, 0);
                }
            }
            break;
        case UOP_PRE_INC:
        case UOP_POST_INC:
            if (n->unary.operand->kind == AST_ID) {
                Symbol *sym = n->unary.operand->symbol;
                if (n->unary.op == UOP_POST_INC) {
                    ir_emit(out, IR_DUP, NULL, 0);
                }
                ir_emit(out, IR_PUSH_INT, NULL, 1);
                ir_emit_with_symbol(out, IR_ADD, NULL, 0, type_sym);
                
                if (sym && sym->is_local) {
                    ir_emit_with_symbol(out, IR_STORE_LOCAL, NULL, sym->local_index, sym);
                } else {
                    ir_emit_with_symbol(out, IR_STORE_GLOBAL, n->unary.operand->id, 0, sym);
                }
                
                if (n->unary.op == UOP_PRE_INC) {
                    if (sym && sym->is_local) {
                        ir_emit_with_symbol(out, IR_LOAD_LOCAL, NULL, sym->local_index, sym);
                    } else {
                        ir_emit_with_symbol(out, IR_LOAD_GLOBAL, n->unary.operand->id, 0, sym);
                    }
                }
            }
            break;
        case UOP_PRE_DEC:
        case UOP_POST_DEC:
            if (n->unary.operand->kind == AST_ID) {
                Symbol *sym = n->unary.operand->symbol;
                if (n->unary.op == UOP_POST_DEC) {
                    ir_emit(out, IR_DUP, NULL, 0);
                }
                ir_emit(out, IR_PUSH_INT, NULL, 1);
                ir_emit_with_symbol(out, IR_SUB, NULL, 0, type_sym);
                
                if (sym && sym->is_local) {
                    ir_emit_with_symbol(out, IR_STORE_LOCAL, NULL, sym->local_index, sym);
                } else {
                    ir_emit_with_symbol(out, IR_STORE_GLOBAL, n->unary.operand->id, 0, sym);
                }
                
                if (n->unary.op == UOP_PRE_DEC) {
                    if (sym && sym->is_local) {
                        ir_emit_with_symbol(out, IR_LOAD_LOCAL, NULL, sym->local_index, sym);
                    } else {
                        ir_emit_with_symbol(out, IR_LOAD_GLOBAL, n->unary.operand->id, 0, sym);
                    }
                }
            }
            break;
        default:
            break;
    }
}

static void gen_assign(AST *n, IRList *out, bool need_value) {
    if (n->assign.op != AOP_ASSIGN) {
        if (n->assign.lhs->kind == AST_ARRAY_ACCESS) {
            Symbol *array_sym = n->assign.lhs->array.array->symbol;
            
            gen_expr(n->assign.lhs->array.array, out);
            gen_expr(n->assign.lhs->array.index, out);
            ir_emit(out, IR_DUP2, NULL, 0);
            ir_emit_with_symbol(out, IR_ARRAY_LOAD, NULL, 0, array_sym);
            gen_expr(n->assign.rhs, out);
            
            Symbol *type_sym = create_type_symbol(n->type);
            
            switch(n->assign.op) {
                case AOP_ADD_ASSIGN: ir_emit_with_symbol(out, IR_ADD, NULL, 0, type_sym); break;
                case AOP_SUB_ASSIGN: ir_emit_with_symbol(out, IR_SUB, NULL, 0, type_sym); break;
                case AOP_MUL_ASSIGN: ir_emit_with_symbol(out, IR_MUL, NULL, 0, type_sym); break;
                case AOP_DIV_ASSIGN: ir_emit_with_symbol(out, IR_DIV, NULL, 0, type_sym); break;
                case AOP_MOD_ASSIGN: ir_emit_with_symbol(out, IR_MOD, NULL, 0, type_sym); break;
                case AOP_AND_ASSIGN: ir_emit(out, IR_BIT_AND, NULL, 0); break;
                case AOP_OR_ASSIGN: ir_emit(out, IR_BIT_OR, NULL, 0); break;
                case AOP_XOR_ASSIGN: ir_emit(out, IR_BIT_XOR, NULL, 0); break;
                case AOP_SHL_ASSIGN: ir_emit(out, IR_SHL, NULL, 0); break;
                case AOP_SHR_ASSIGN: ir_emit(out, IR_SHR, NULL, 0); break;
                default: break;
            }
            
            if (need_value) {
                ir_emit(out, IR_DUP_X2, NULL, 0);
            }
            
            ir_emit_with_symbol(out, IR_ARRAY_STORE, NULL, 0, array_sym);
            
        } else if (n->assign.lhs->kind == AST_ID) {
            Symbol *sym = n->assign.lhs->symbol;
            
            if (sym && sym->is_local) {
                ir_emit_with_symbol(out, IR_LOAD_LOCAL, NULL, sym->local_index, sym);
            } else {
                ir_emit_with_symbol(out, IR_LOAD_GLOBAL, n->assign.lhs->id, 0, sym);
            }
            
            gen_expr(n->assign.rhs, out);
            
            Symbol *type_sym = create_type_symbol(n->type);
            
            switch(n->assign.op) {
                case AOP_ADD_ASSIGN: ir_emit_with_symbol(out, IR_ADD, NULL, 0, type_sym); break;
                case AOP_SUB_ASSIGN: ir_emit_with_symbol(out, IR_SUB, NULL, 0, type_sym); break;
                case AOP_MUL_ASSIGN: ir_emit_with_symbol(out, IR_MUL, NULL, 0, type_sym); break;
                case AOP_DIV_ASSIGN: ir_emit_with_symbol(out, IR_DIV, NULL, 0, type_sym); break;
                case AOP_MOD_ASSIGN: ir_emit_with_symbol(out, IR_MOD, NULL, 0, type_sym); break;
                case AOP_AND_ASSIGN: ir_emit(out, IR_BIT_AND, NULL, 0); break;
                case AOP_OR_ASSIGN: ir_emit(out, IR_BIT_OR, NULL, 0); break;
                case AOP_XOR_ASSIGN: ir_emit(out, IR_BIT_XOR, NULL, 0); break;
                case AOP_SHL_ASSIGN: ir_emit(out, IR_SHL, NULL, 0); break;
                case AOP_SHR_ASSIGN: ir_emit(out, IR_SHR, NULL, 0); break;
                default: break;
            }
            
            if (need_value) {
                ir_emit(out, IR_DUP, NULL, 0);
            }
            
            if (sym && sym->is_local) {
                ir_emit_with_symbol(out, IR_STORE_LOCAL, NULL, sym->local_index, sym);
            } else {
                ir_emit_with_symbol(out, IR_STORE_GLOBAL, n->assign.lhs->id, 0, sym);
            }
        }
    } else {
        if (n->assign.lhs->kind == AST_ARRAY_ACCESS) {
            gen_expr(n->assign.lhs->array.array, out);
            gen_expr(n->assign.lhs->array.index, out);
            gen_expr(n->assign.rhs, out);
            
            if (need_value) {
                ir_emit(out, IR_DUP_X2, NULL, 0);
            }
            
            Symbol *array_sym = n->assign.lhs->array.array->symbol;
            ir_emit_with_symbol(out, IR_ARRAY_STORE, NULL, 0, array_sym);
            
        } else if (n->assign.lhs->kind == AST_ID) {
            gen_expr(n->assign.rhs, out);
            Symbol *sym = n->assign.lhs->symbol;
            
            if (need_value) {
                ir_emit(out, IR_DUP, NULL, 0);
            }
            
            if (sym && sym->is_local) {
                ir_emit_with_symbol(out, IR_STORE_LOCAL, NULL, sym->local_index, sym);
            } else {
                ir_emit_with_symbol(out, IR_STORE_GLOBAL, n->assign.lhs->id, 0, sym);
            }
        }
    }
}

static void gen_call(AST *n, IRList *out) {
    AST *arg = n->call.args;
    while (arg) {
        gen_expr(arg, out);
        arg = arg->next;
    }
    
    const char *func_name = NULL;
    Symbol *func_symbol = NULL;
    
    if (n->call.callee->kind == AST_ID) {
        func_name = n->call.callee->id;
        func_symbol = n->call.callee->symbol;
    }
    
    if (func_name) {
        ir_emit_with_symbol(out, IR_CALL, func_name, n->call.arg_count, func_symbol);
    }
}

static void gen_logical_or(AST *n, IRList *out) {
    char *end_label = gen_label();
    
    gen_expr(n->logical.left, out);
    ir_emit(out, IR_DUP, NULL, 0);
    
    // If left is non-zero, result is true - skip right operand
    ir_emit(out, IR_JUMP_IF_ZERO, end_label, 0);
    ir_emit(out, IR_POP, NULL, 0);
    ir_emit(out, IR_PUSH_INT, NULL, 1);
    char *skip = gen_label();
    ir_emit(out, IR_JUMP, skip, 0);
    
    ir_emit(out, IR_LABEL, end_label, 0);
    ir_emit(out, IR_POP, NULL, 0);
    gen_expr(n->logical.right, out);
    
    ir_emit(out, IR_LABEL, skip, 0);
}

static void gen_logical_and(AST *n, IRList *out) {
    char *end_label = gen_label();
    
    gen_expr(n->logical.left, out);
    ir_emit(out, IR_DUP, NULL, 0);
    
    // If left is zero, result is false - skip right operand
    ir_emit(out, IR_JUMP_IF_ZERO, end_label, 0);
    ir_emit(out, IR_POP, NULL, 0);
    gen_expr(n->logical.right, out);
    char *skip = gen_label();
    ir_emit(out, IR_JUMP, skip, 0);
    
    ir_emit(out, IR_LABEL, end_label, 0);
    ir_emit(out, IR_POP, NULL, 0);
    ir_emit(out, IR_PUSH_INT, NULL, 0);
    
    ir_emit(out, IR_LABEL, skip, 0);
}

static void gen_ternary(AST *n, IRList *out) {
    char *false_label = gen_label();
    char *end_label = gen_label();
    
    gen_expr(n->ternary.cond, out);
    ir_emit(out, IR_JUMP_IF_ZERO, false_label, 0);
    
    gen_expr(n->ternary.iftrue, out);
    ir_emit(out, IR_JUMP, end_label, 0);
    
    ir_emit(out, IR_LABEL, false_label, 0);
    gen_expr(n->ternary.iffalse, out);
    
    ir_emit(out, IR_LABEL, end_label, 0);
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
            
        case AST_ID: {
            Symbol *sym = n->symbol;

            if (sym && sym->is_local) {
                ir_emit_with_symbol(out, IR_LOAD_LOCAL, NULL, sym->local_index, sym);
            } else {
                ir_emit_with_symbol(out, IR_LOAD_GLOBAL, n->id, 0, sym);
            }
            break;
        }
            
        case AST_BINOP:
            gen_binop(n, out);
            break;
            
        case AST_ASSIGN:
            gen_assign(n, out, true);
            break;
            
        case AST_UNARY:
            gen_unary(n, out);
            break;
            
        case AST_ARRAY_ACCESS: {
            gen_expr(n->array.array, out);
            gen_expr(n->array.index, out);
            
            Symbol *array_sym = n->array.array->symbol;
            ir_emit_with_symbol(out, IR_ARRAY_LOAD, NULL, 0, array_sym);
            break;
        }
            
        case AST_FUNC_CALL:
            gen_call(n, out);
            break;
            
        case AST_LOGICAL_OR:
            gen_logical_or(n, out);
            break;
            
        case AST_LOGICAL_AND:
            gen_logical_and(n, out);
            break;
            
        case AST_TERNARY:
            gen_ternary(n, out);
            break;
            
        default:
            break;
    }
}

void gen_decl(AST *n, IRList *out) {
    if (!n || n->kind != AST_DECL) return;
    
    if (n->decl.decl_type && n->decl.decl_type->kind == TY_ARRAY) {
        Symbol *sym = n->symbol;
        
        if (sym && sym->is_local) {
            int array_size;
            if(n->decl.decl_type->array_size > 0){
                array_size = n->decl.decl_type->array_size;
            } else {
                fprintf(stderr, "Warning: Array size not specified, defaulting to 10\n");
                array_size = 10;
            }

            ir_emit(out, IR_PUSH_INT, NULL, array_size);
            ir_emit_with_symbol(out, IR_ALLOC_ARRAY, n->decl.name, 0, sym);
            ir_emit_with_symbol(out, IR_STORE_LOCAL, NULL, sym->local_index, sym);
        }
    }
    
    if (n->decl.init) {
        gen_expr(n->decl.init, out);
        
        Symbol *sym = n->symbol;
        if (sym && sym->is_local) {
            ir_emit_with_symbol(out, IR_STORE_LOCAL, NULL, sym->local_index, sym);
        } else {
            ir_emit_with_symbol(out, IR_STORE_GLOBAL, n->decl.name, 0, sym);
        }
    }
}

static void gen_if(AST *n, IRList *out) {
    char *else_label = gen_label();
    char *end_label = gen_label();
    
    gen_expr(n->if_stmt.cond, out);
    ir_emit(out, IR_JUMP_IF_ZERO, else_label, 0);
    
    gen_stmt(n->if_stmt.then_branch, out);
    ir_emit(out, IR_JUMP, end_label, 0);
    
    ir_emit(out, IR_LABEL, else_label, 0);
    if (n->if_stmt.else_branch) {
        gen_stmt(n->if_stmt.else_branch, out);
    }
    
    ir_emit(out, IR_LABEL, end_label, 0);
}

static void gen_while(AST *n, IRList *out) {
    char *start_label = gen_label();
    char *end_label = gen_label();
    
    push_loop(end_label, start_label);
    
    ir_emit(out, IR_LABEL, start_label, 0);
    gen_expr(n->while_stmt.cond, out);
    ir_emit(out, IR_JUMP_IF_ZERO, end_label, 0);
    
    gen_stmt(n->while_stmt.body, out);
    ir_emit(out, IR_JUMP, start_label, 0);
    
    ir_emit(out, IR_LABEL, end_label, 0);
    
    pop_loop();
}

static void gen_do_while(AST *n, IRList *out) {
    char *start_label = gen_label();
    char *cond_label = gen_label();
    char *end_label = gen_label();
    
    push_loop(end_label, cond_label);
    
    ir_emit(out, IR_LABEL, start_label, 0);
    gen_stmt(n->do_while.body, out);
    
    ir_emit(out, IR_LABEL, cond_label, 0);
    gen_expr(n->do_while.cond, out);
    ir_emit(out, IR_JUMP_IF_ZERO, end_label, 0);
    ir_emit(out, IR_JUMP, start_label, 0);
    
    ir_emit(out, IR_LABEL, end_label, 0);
    
    pop_loop();
}

static void gen_for(AST *n, IRList *out) {
    char *start_label = gen_label();
    char *post_label = gen_label();
    char *end_label = gen_label();
    
    // Init
    if (n->for_stmt.init) {
        gen_stmt(n->for_stmt.init, out);
    }
    
    push_loop(end_label, post_label);
    
    // Condition check
    ir_emit(out, IR_LABEL, start_label, 0);
    if (n->for_stmt.cond) {
        gen_expr(n->for_stmt.cond, out);
        ir_emit(out, IR_JUMP_IF_ZERO, end_label, 0);
    }
    
    // Body
    gen_stmt(n->for_stmt.body, out);
    
    // Post (continue jumps here)
    ir_emit(out, IR_LABEL, post_label, 0);
    if (n->for_stmt.post) {
        if (n->for_stmt.post->kind == AST_ASSIGN || 
            n->for_stmt.post->kind == AST_UNARY ||
            n->for_stmt.post->kind == AST_FUNC_CALL) {
            gen_stmt(n->for_stmt.post, out);
        } else {
            gen_expr(n->for_stmt.post, out);
            ir_emit(out, IR_POP, NULL, 0);
        }
    }
    
    ir_emit(out, IR_JUMP, start_label, 0);
    ir_emit(out, IR_LABEL, end_label, 0);
    
    pop_loop();
}

static void gen_stmt(AST *n, IRList *out) {
    if (!n) return;
    
    switch(n->kind) {
        case AST_DECL:
            gen_decl(n, out);
            break;
            
        case AST_RETURN:
            if (n->ret.expr) {
                gen_expr(n->ret.expr, out);
                ir_emit(out, IR_RETURN, NULL, 0);
            } else {
                ir_emit(out, IR_RETURN_VOID, NULL, 0);
            }
            break;
            
        case AST_BREAK: {
            char *break_label = get_break_label();
            if (break_label) {
                ir_emit(out, IR_JUMP, break_label, 0);
            }
            break;
        }
            
        case AST_CONTINUE: {
            char *continue_label = get_continue_label();
            if (continue_label) {
                ir_emit(out, IR_JUMP, continue_label, 0);
            }
            break;
        }
            
        case AST_IF:
            gen_if(n, out);
            break;
            
        case AST_WHILE:
            gen_while(n, out);
            break;
            
        case AST_DO_WHILE:
            gen_do_while(n, out);
            break;
            
        case AST_FOR:
            gen_for(n, out);
            break;
            
        case AST_BLOCK:
            for (int i = 0; i < n->block.count; i++) {
                gen_stmt(n->block.statements[i], out);
            }
            break;
            
        case AST_FUNC_CALL:
            gen_expr(n, out);
            if(n->type && n->type->kind != TY_VOID && n->func.return_type && n->func.return_type->kind != TY_VOID){
                ir_emit(out, IR_POP, NULL, 0);
            }
            break;
            
        case AST_ASSIGN:
            gen_assign(n, out, false);
            break;
            
        case AST_UNARY:
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
        if (ast->func.body) {
            gen_stmt(ast->func.body, out);
        }
    } else {
        gen_stmt(ast, out);
    }
}
