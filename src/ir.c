#include "ir.h"
#include "symtab.h"

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
    n->symbol = NULL;  // Add this

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
    n->symbol = sym;  // Store the symbol

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
    n->symbol = NULL;  // Add this

    if (!l->head)
        l->head = l->tail = n;
    else {
        l->tail->next = n;
        l->tail = n;
    }
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
            default:
                fprintf(out, "UNKNOWN(%d)\n", p->kind);
                break;
        }
    }
    fprintf(out, "=== End IR ===\n\n");
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
                // FIX: Get symbol from the operand
                Symbol *sym = n->unary.operand->symbol;
                if (n->unary.op == UOP_POST_INC) {
                    ir_emit(out, IR_DUP, NULL, 0);
                }
                ir_emit(out, IR_PUSH_INT, NULL, 1);
                ir_emit(out, IR_ADD, NULL, 0);
                
                // Use local or global store based on symbol
                if (sym && sym->is_local) {
                    ir_emit(out, IR_STORE_LOCAL, NULL, sym->local_index);
                } else {
                    ir_emit(out, IR_STORE_GLOBAL, n->unary.operand->id, 0);
                }
                
                if (n->unary.op == UOP_PRE_INC) {
                    // Load the new value back for pre-increment
                    if (sym && sym->is_local) {
                        ir_emit(out, IR_LOAD_LOCAL, NULL, sym->local_index);
                    } else {
                        ir_emit(out, IR_LOAD_GLOBAL, n->unary.operand->id, 0);
                    }
                }
            }
            break;
        case UOP_PRE_DEC:
        case UOP_POST_DEC:
            if (n->unary.operand->kind == AST_ID) {
                // FIX: Get symbol from the operand
                Symbol *sym = n->unary.operand->symbol;
                if (n->unary.op == UOP_POST_DEC) {
                    ir_emit(out, IR_DUP, NULL, 0);
                }
                ir_emit(out, IR_PUSH_INT, NULL, 1);
                ir_emit(out, IR_SUB, NULL, 0);
                
                // Use local or global store based on symbol
                if (sym && sym->is_local) {
                    ir_emit(out, IR_STORE_LOCAL, NULL, sym->local_index);
                } else {
                    ir_emit(out, IR_STORE_GLOBAL, n->unary.operand->id, 0);
                }
                
                if (n->unary.op == UOP_PRE_DEC) {
                    // Load the new value back for pre-decrement
                    if (sym && sym->is_local) {
                        ir_emit(out, IR_LOAD_LOCAL, NULL, sym->local_index);
                    } else {
                        ir_emit(out, IR_LOAD_GLOBAL, n->unary.operand->id, 0);
                    }
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
            Symbol *sym = n->assign.lhs->symbol;
            
            // Load current value
            if (sym && sym->is_local) {
                ir_emit(out, IR_LOAD_LOCAL, NULL, sym->local_index);
            } else {
                ir_emit(out, IR_LOAD_GLOBAL, n->assign.lhs->id, 0);
            }
            
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
            
            // Store result
            if (sym && sym->is_local) {
                ir_emit(out, IR_STORE_LOCAL, NULL, sym->local_index);
                ir_emit(out, IR_LOAD_LOCAL, NULL, sym->local_index);
            } else {
<<<<<<< HEAD
                ir_emit_with_symbol(out, IR_STORE_GLOBAL, n->assign.lhs->id, 0, sym);
                if(need_value){
                    ir_emit_with_symbol(out, IR_LOAD_GLOBAL, n->assign.lhs->id, 0, sym);
                }
=======
                ir_emit(out, IR_STORE_GLOBAL, n->assign.lhs->id, 0);
                ir_emit(out, IR_LOAD_GLOBAL, n->assign.lhs->id, 0);
>>>>>>> parent of 98a2c5f (Fixed statement vs expression assignments for pushing onto stack, began working on array access currently not working)
            }
        }
    } else {
        // Simple assignment
<<<<<<< HEAD
        if (n->assign.lhs->kind == AST_ARRAY_ACCESS) {
            // For array[index] = value:
            // Stack needs to be: array, index, value (in that order for iastore)
            
            gen_expr(n->assign.lhs->array.array, out);   // array ref
            gen_expr(n->assign.lhs->array.index, out);   // index
            gen_expr(n->assign.rhs, out);                // value
            
            // If we need the value (for chained assignment), we need to duplicate it
            // BEFORE the store, but the stack order for iastore is: array, index, value
            // So we need: array, index, value, value (with dup_x2 to get value below array/index)
            if (need_value) {
                // Stack: array, index, value
                // We want: value, array, index, value (so after iastore, value remains)
                // Use dup_x2: duplicates top value and inserts it 2 slots down
                // This gives us: array, value, index, value
                // That's not quite right... we need a different approach
                
                // Better approach: dup, then swap to get the right order
                // Actually, for iastore we need: arrayref, index, value
                // To keep value after store, we do: arrayref, index, value, value
                // Then rotate: arrayref, value, index, value
                // No, simpler: just dup BEFORE generating the whole sequence
                
                // Actually the right way: evaluate RHS first when chained
                // This is getting complex - let's restructure
            }

            Symbol *array_sym = n->assign.lhs->array.array->symbol;
            ir_emit_with_symbol(out, IR_ARRAY_STORE, NULL, 0, array_sym);
            
            if (need_value) {
                // After store, reload the value we just stored
                gen_expr(n->assign.lhs->array.array, out);
                gen_expr(n->assign.lhs->array.index, out);
                ir_emit_with_symbol(out, IR_ARRAY_LOAD, NULL, 0, array_sym);
            }

        } else if (n->assign.lhs->kind == AST_ID) {
            gen_expr(n->assign.rhs, out);

=======
        gen_expr(n->assign.rhs, out);
        
        if (n->assign.lhs->kind == AST_ID) {
            // FIX: Get symbol from the LHS
>>>>>>> parent of 98a2c5f (Fixed statement vs expression assignments for pushing onto stack, began working on array access currently not working)
            Symbol *sym = n->assign.lhs->symbol;
            ir_emit(out, IR_DUP, NULL, 0);
            
            // Store to local or global
            if (sym && sym->is_local) {
                ir_emit(out, IR_STORE_LOCAL, NULL, sym->local_index);
            } else {
                ir_emit_with_symbol(out, IR_STORE_GLOBAL, n->assign.lhs->id, 0, sym);
            }
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
    
    // Get function name and symbol
    const char *func_name = NULL;
    Symbol *func_symbol = NULL;
    
    if (n->call.callee->kind == AST_ID) {
        func_name = n->call.callee->id;
        func_symbol = n->call.callee->symbol;  // Get the symbol from the callee
    }
    
    if (func_name) {
        ir_emit_with_symbol(out, IR_CALL, func_name, n->call.arg_count, func_symbol);
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
            
        case AST_ID: {
            Symbol *sym = n->symbol;

            if (sym && sym->is_local) {
                ir_emit(out, IR_LOAD_LOCAL, NULL, sym->local_index);
            } else {
                // Pass symbol for global variables so we can get the correct type descriptor
                ir_emit_with_symbol(out, IR_LOAD_GLOBAL, n->id, 0, sym);
            }
            break;
        }
            
        case AST_BINOP:
            gen_binop(n, out);
            break;
            
        case AST_ASSIGN:
            gen_assign(n, out);
            break;
            
        case AST_UNARY:
            gen_unary(n, out);
            break;
<<<<<<< HEAD

        case AST_ARRAY_ACCESS: {
            gen_expr(n->array.array, out);   // Load array reference
            gen_expr(n->array.index, out);   // Load index

            // FIX: Pass the array's symbol so jbcgen can get the element type
            Symbol *array_sym = n->array.array->symbol;
            ir_emit_with_symbol(out, IR_ARRAY_LOAD, NULL, 0, array_sym);
            break;
        }
=======
>>>>>>> parent of 98a2c5f (Fixed statement vs expression assignments for pushing onto stack, began working on array access currently not working)
            
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
            if(n->type && n->type->kind != TY_VOID && n->func.return_type && n->func.return_type->kind != TY_VOID){
                ir_emit(out, IR_POP, NULL, 0);
            }
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
