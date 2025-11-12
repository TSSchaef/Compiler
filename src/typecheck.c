#include "typecheck.h"
#include "ast.h"
#include "symtab.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static Type *current_function_return_type = NULL;

// Type constructors
Type *type_int() {
    return type_int_const(false);
}

Type *type_int_const(bool is_const) {
    Type *t = malloc(sizeof(Type));
    t->kind = TY_INT;
    t->is_const = is_const;
    t->return_type = NULL;
    t->array_of = NULL;
    t->params = NULL;
    t->param_count = 0;
    return t;
}

Type *type_char() {
    return type_char_const(false);
}

Type *type_char_const(bool is_const) {
    Type *t = malloc(sizeof(Type));
    t->kind = TY_CHAR;
    t->is_const = is_const;
    return t;
}

Type *type_float() {
    return type_float_const(false);
}

Type *type_float_const(bool is_const) {
    Type *t = malloc(sizeof(Type));
    t->kind = TY_FLT;
    t->is_const = is_const;
    return t;
}

Type *type_void() {
    return type_void_const(false);
}

Type *type_void_const(bool is_const) {
    Type *t = malloc(sizeof(Type));
    t->kind = TY_VOID;
    t->is_const = is_const;
    return t;
}

Type *set_const(Type *t){
    if(t){
        t->is_const = true;
    }
    return t;
}

Type *type_array(Type *elem_type) {
    Type *t = malloc(sizeof(Type));
    t->kind = TY_ARRAY;
    t->is_const = false;
    t->array_of = elem_type;
    return t;
}

Type *type_func(Type *ret, Type **params, int param_count) {
    Type *ft = malloc(sizeof(Type));
    ft->kind = TY_FUNC;
    ft->return_type = ret;
    ft->params = params;
    ft->param_count = param_count;
    return ft;
}

// Helper to convert type to string for output
static const char *type_to_string(Type *t) {
    static char buf[258];
    if (!t) return "unknown";
    
    const char *const_prefix = t->is_const ? "const " : "";
    
    switch (t->kind) {
        case TY_INT: 
            snprintf(buf, sizeof(buf), "%sint", const_prefix);
            return buf;
        case TY_CHAR:
            snprintf(buf, sizeof(buf), "%schar", const_prefix);
            return buf;
        case TY_FLT:
            snprintf(buf, sizeof(buf), "%sfloat", const_prefix);
            return buf;
        case TY_VOID: 
            return "void";
        case TY_ARRAY: {
            // Get the base element type string (without any const)
            Type *elem = t->array_of;
            if (!elem) return "[]";

            // Build the string with const prefix if element is const
            const char *elem_const = elem->is_const ? "const " : "";
            const char *base_type;

            switch (elem->kind) {
                case TY_INT: base_type = "int"; break;
                case TY_CHAR: base_type = "char"; break;
                case TY_FLT: base_type = "float"; break;
                case TY_VOID: base_type = "void"; break;
                case TY_ARRAY:
                              // Nested array - recursively get the type
                              base_type = type_to_string(elem);
                              snprintf(buf, sizeof(buf), "%s[]", base_type);
                              return buf;
                default: base_type = "unknown"; break;
            }

            snprintf(buf, sizeof(buf), "%s%s[]", elem_const, base_type);
            return buf;
        }
        case TY_FUNC:
            return "function";
        default:
            return "unknown";
    }
}
// Error helper
static void error(const char *msg, AST *node) {
    fprintf(stderr, "Type checking error in file %s line %d\n\t%s\n", 
            node->filename,
            ast_get_line_no(node), msg);
}

// Check if types are compatible
static bool types_equal(Type *t1, Type *t2) {
    if (!t1 || !t2) return false;
    if (t1->kind != t2->kind) return false;
    
    if (t1->kind == TY_ARRAY) {
        return types_equal(t1->array_of, t2->array_of);
    }
    
    return true;
}

// Check if type is numeric
static bool is_numeric(Type *t) {
    return t && (t->kind == TY_INT || t->kind == TY_FLT);
}

static bool is_char(Type *t) {
    return t && (t->kind == TY_CHAR);
}

// Check if type is integral
static bool is_integral(Type *t) {
    return t && (t->kind == TY_INT || t->kind == TY_CHAR);
}

// Forward declaration
static void type_check_node(AST *node);

static bool is_expression_statement(AST *stmt) {
    if (!stmt) return false;
    
    switch (stmt->kind) {
        // These are NOT expression statements
        case AST_DECL:
        case AST_FUNC:
        case AST_IF:
        case AST_WHILE:
        case AST_FOR:
        case AST_DO_WHILE:
        case AST_BLOCK:
        case AST_RETURN:
        case AST_BREAK:
        case AST_CONTINUE:
        case AST_SWITCH:
        case AST_CASE:
            return false;
        
        // These ARE expression statements
        case AST_ASSIGN:
        case AST_BINOP:
        case AST_UNARY:
        case AST_FUNC_CALL:
        case AST_LOGICAL_OR:
        case AST_LOGICAL_AND:
        case AST_TERNARY:
        case AST_ID:
        case AST_ARRAY_ACCESS:
        case AST_INT_LITERAL:
        case AST_FLOAT_LITERAL:
        case AST_STRING_LITERAL:
        case AST_CHAR_LITERAL:
        case AST_BOOL_LITERAL:
            return true;
        
        default:
            return false;
    }
}

// Type check and write expression statement to output
static void check_expression_statement(AST *expr) {
    if (!expr) return;
    
    type_check_node(expr);
    
    if (outputFile && expr->type) {
        fprintf(outputFile, "File %s Line %d: expression has type %s\n",
                expr->filename,
                ast_get_line_no(expr),
                type_to_string(expr->type));
    }
}

static void type_check_node(AST *node) {
    if (!node) return;

    //fprintf(stderr, "Type checking node kind=%d, line=%d\n", node->kind, node->line_no);

    switch (node->kind) {
    case AST_INT_LITERAL:
        node->type = type_int();
        break;
        
    case AST_FLOAT_LITERAL:
        node->type = type_float();
        break;
        
    case AST_CHAR_LITERAL:
        node->type = type_char();
        break;
        
    case AST_STRING_LITERAL: {
        Type *char_type = malloc(sizeof(Type));
        char_type->kind = TY_CHAR;
        char_type->is_const = true;  // String literals are const
        node->type = type_array(char_type);
        break;
    }
                              

    case AST_BOOL_LITERAL:
        node->type = type_int(); // Treat bool as int
        break;

    case AST_ID: {
        Symbol *s = lookup_symbol(node->id);
        if (!s) {
            error("Undeclared identifier", node);
            node->type = NULL;
        } else {
            node->type = s->type;
        }
        break;
    }

    case AST_ARRAY_ACCESS: {
       type_check_node(node->array.array);
       type_check_node(node->array.index);

       if (!node->array.array->type) {
           error("Array expression has no type", node);
           node->type = NULL;
           break;
       }

       if (node->array.array->type->kind != TY_ARRAY) {
           error("Tried to access a variable that isn't an array", node);
           node->type = NULL;
           break;
       }

       if (!node->array.index->type || !is_integral(node->array.index->type)) {
           error("Array subscript is not an integer", node);
           node->type = NULL;
           break;
       }

       // The type of array[index] is the element type, preserving const
       node->type = node->array.array->type->array_of;
       break;
   }

    case AST_BINOP:
        type_check_node(node->binop.left);
        type_check_node(node->binop.right);
        
        if (!node->binop.left->type || !node->binop.right->type) {
            error("Invalid operands in binary operation", node);
            node->type = NULL;
            break;
        }
        
        // Arithmetic operators
        if (node->binop.op == OP_ADD || node->binop.op == OP_SUB ||
            node->binop.op == OP_MUL || node->binop.op == OP_DIV) {
            if (!is_numeric(node->binop.left->type) || 
                !is_numeric(node->binop.right->type)) {
                
                if(!is_char(node->binop.left->type) || 
                        !is_char(node->binop.right->type)) {
                    error("Arithmetic operation requires numeric \
                            or char operands", node);
                    node->type = NULL;
                } else {
                    node->type = type_char();
                }

            } else {
                // Float if either operand is float
                if (node->binop.left->type->kind == TY_FLT || 
                    node->binop.right->type->kind == TY_FLT) {
                    node->type = type_float();
                } else {
                    node->type = type_int();
                }
            }
        }
        // Modulo and bitwise operators
        else if (node->binop.op == OP_MOD || node->binop.op == OP_BIT_AND ||
                 node->binop.op == OP_BIT_OR || node->binop.op == OP_BIT_XOR ||
                 node->binop.op == OP_SHL || node->binop.op == OP_SHR) {
            if (!is_integral(node->binop.left->type) || 
                !is_integral(node->binop.right->type)) {
                error("Bitwise/modulo operation requires integral operands", node);
                node->type = NULL;
            } else {
                node->type = type_int();
            }
        }
        // Comparison operators
        else {
            node->type = type_char(); // Comparisons return char (boolean)
        }

        if (node->type) {
            // All binops discard const
            node->type->is_const = false;
        }

        break;

case AST_ASSIGN:
        type_check_node(node->assign.lhs);
        type_check_node(node->assign.rhs);
        
        if (!node->assign.lhs->type || !node->assign.rhs->type) {
            error("Invalid operands in assignment", node);
            node->type = NULL;
            break;
        }
        
        // Check basic type compatibility
        if (node->assign.lhs->type->kind == TY_VOID || 
            node->assign.rhs->type->kind == TY_VOID) {
            error("Cannot assign void type", node);
            node->type = NULL;
            break;
        }

        // Check if trying to assign to const
        if (node->assign.lhs->type->is_const) {
            error("Cannot assign to const variable", node);
            node->type = NULL;
            break;
        }
        
        // For compound assignments (+=, -=, *=, /=, %=), check arithmetic compatibility
        if (node->assign.op != AOP_ASSIGN) {
            // LHS must be numeric for arithmetic compound assignments
            if (node->assign.op == AOP_ADD_ASSIGN || 
                node->assign.op == AOP_SUB_ASSIGN ||
                node->assign.op == AOP_MUL_ASSIGN || 
                node->assign.op == AOP_DIV_ASSIGN) {
                
                // For array types, += and -= are valid (pointer arithmetic)
                if (node->assign.lhs->type->kind == TY_ARRAY) {
                    if (node->assign.op == AOP_MUL_ASSIGN || 
                        node->assign.op == AOP_DIV_ASSIGN) {
                        error("Cannot use *= or /= with array/pointer type", node);
                        node->type = NULL;
                        break;
                    }
                    // For += and -=, RHS must be integral
                    if (!is_integral(node->assign.rhs->type)) {
                        error("Pointer arithmetic requires integer operand", node);
                        node->type = NULL;
                        break;
                    }
                } else {
                    // For non-array types, both sides must be numeric
                    if (!is_numeric(node->assign.lhs->type) || 
                        !is_numeric(node->assign.rhs->type)) {
                        error("Compound assignment requires arithmetic types", node);
                        node->type = NULL;
                        break;
                    }
                }
            } else if (node->assign.op == AOP_MOD_ASSIGN) {
                // Modulo requires integral types on both sides
                if (!is_integral(node->assign.lhs->type) || 
                    !is_integral(node->assign.rhs->type)) {
                    error("Modulo assignment requires integer types", node);
                    node->type = NULL;
                    break;
                }
            } else {
                // Bitwise compound assignments require integral types
                if (!is_integral(node->assign.lhs->type) || 
                    !is_integral(node->assign.rhs->type)) {
                    error("Bitwise compound assignment requires integer types", node);
                    node->type = NULL;
                    break;
                }
            }
        } else {
            // Simple assignment - check type compatibility
            // Arrays/pointers can be assigned to each other
            if (node->assign.lhs->type->kind == TY_ARRAY) {
                // Assigning to an array - RHS should also be array/pointer
                if (node->assign.rhs->type->kind != TY_ARRAY) {
                    error("Type mismatch: cannot assign non-array to array", node);
                    node->type = NULL;
                    break;
                }
                // Check that element types match
                if (!types_equal(node->assign.lhs->type->array_of, 
                                node->assign.rhs->type->array_of)) {
                    error("Type mismatch: array element types do not match", node);
                    node->type = NULL;
                    break;
                }
            } else if (node->assign.rhs->type->kind == TY_ARRAY) {
                // Assigning array to non-array is not allowed
                error("Type mismatch: cannot assign array to non-array", node);
                node->type = NULL;
                break;
            } else {
                // Both are non-arrays - allow implicit conversions between arithmetic types
                // but don't allow completely incompatible types
                if ((is_numeric(node->assign.lhs->type) && 
                     !is_numeric(node->assign.rhs->type)) ||
                    (!is_numeric(node->assign.lhs->type) && 
                     is_numeric(node->assign.rhs->type))) {
                    error("Type mismatch in assignment", node);
                    node->type = NULL;
                    break;
                }
            }
        }
        
        node->type = node->assign.lhs->type;
        break;


    case AST_LOGICAL_OR:
    case AST_LOGICAL_AND:
        type_check_node(node->logical.left);
        type_check_node(node->logical.right);
        node->type = type_char(); // Logical operators return char (boolean)
        break;

    case AST_TERNARY:
        type_check_node(node->ternary.cond);
        type_check_node(node->ternary.iftrue);
        type_check_node(node->ternary.iffalse);
        
        if (node->ternary.iftrue->type && node->ternary.iffalse->type) {
            // Result type is from the true branch (simplified)
            node->type = node->ternary.iftrue->type;
        } else {
            node->type = NULL;
        }
        break;

    case AST_UNARY:
        type_check_node(node->unary.operand);

        if(node->unary.operand->type){
            if(node->unary.operand->type->kind == TY_VOID){
                error("Operand of unary operator cannot be void", node);
                node->type = NULL;
                break;
            }
        }
        
        switch (node->unary.op) {
            case UOP_PLUS:
            case UOP_NEG:
                if (node->unary.operand->type && 
                    is_numeric(node->unary.operand->type) || 
                    is_char(node->unary.operand->type)) {
                    node->type = node->unary.operand->type;
                } else {
                    error("Unary +/- requires numeric or char operand", node);
                    node->type = NULL;
                }
                if (node->type) {
                    node->type->is_const = false;
                }
                break;
                
            case UOP_LOGICAL_NOT:
                node->type = type_char();
                if (node->type) {
                    node->type->is_const = false;
                }
                break;
                
            case UOP_BITWISE_NOT:
                if (node->unary.operand->type && 
                    is_integral(node->unary.operand->type) ||
                    is_char(node->unary.operand->type)) {
                    node->type = node->unary.operand->type;
                } else {
                    error("Bitwise NOT requires integral operand", node);
                    node->type = NULL;
                }
                if (node->type) {
                    node->type->is_const = false;
                }
                break;
                
            case UOP_PRE_INC:
            case UOP_PRE_DEC:
            case UOP_POST_INC:
            case UOP_POST_DEC:
                if (node->unary.operand->type) {
                    if (node->unary.operand->type->is_const) {
                        error("Cannot increment/decrement const variable", node);
                        node->type = NULL;
                        break;
                    }
                    node->type = node->unary.operand->type;
                } else {
                    node->type = NULL;
                }
                break;
                
            case UOP_CAST:
                node->type = node->unary.cast_type;
                if (node->type) {
                    node->type->is_const = false;
                }
                break;
                
            default:
                node->type = node->unary.operand->type;
                if (node->type) {
                    node->type->is_const = false;
                }
                break;
        }
        break;

    case AST_DECL:
        //fprintf(stderr, "  DECL name=%s, next=%p\n", node->decl.name, (void*)node->next);

        if (node->decl.init) {
            type_check_node(node->decl.init);
        }
        if(!add_symbol(node->decl.name, node->decl.decl_type)){
            error("Redeclaration of variable", node);
        }
        if(node->decl.decl_type->kind == TY_VOID){
            error("Variable cannot be of type void", node);
        }
        node->type = node->decl.decl_type;
        // DON'T process node->next here - the block will handle it
        break;

    case AST_FUNC: {
                       // Build function type from parameters
                       AST *param = node->func.params;
                       int param_count = 0;
                       Type **param_types = NULL;

                       // Count parameters
                       for (AST *p = param; p; p = p->next) {
                           param_count++;
                       }

                       if (param_count > 0) {
                           param_types = malloc(sizeof(Type*) * param_count);
                           int i = 0;
                           for (AST *p = param; p; p = p->next) {
                               param_types[i++] = p->decl.decl_type;
                           }
                       }

                       Type *ft = type_func(node->func.return_type, param_types, param_count);

                       if(!add_symbol(node->func.name, ft)){
                           error("Redeclaration of function", node);
                       }

                       enter_scope();

                       // Add parameters to scope
                       for (AST *p = param; p; p = p->next) {
                           if(!add_symbol(p->decl.name, p->decl.decl_type)){
                               error("Redeclaration of parameter", node);
                           }

                           if(p->decl.decl_type->kind == TY_VOID){
                               error("Function parameter cannot be of type void", node);
                           }
                       }

                       // Set current function return type for return statement checking
                       Type *prev_return_type = current_function_return_type;
                       current_function_return_type = node->func.return_type;

                       type_check_node(node->func.body);

                       // Restore previous return type (for nested functions if you support them)
                       current_function_return_type = prev_return_type;

                       exit_scope();

                       node->type = ft;
                       break;
                   }

 case AST_FUNC_CALL: {
        type_check_node(node->call.callee);
        
        // Type check arguments
        for (AST *arg = node->call.args; arg; arg = arg->next) {
            type_check_node(arg);
        }
        
        if (!node->call.callee->type) {
            error("Function callee has no type", node);
            node->type = NULL;
            break;
        }
        
        if (node->call.callee->type->kind != TY_FUNC) {
            error("Called object is not a function", node);
            node->type = NULL;
            break;
        }
        
        // Check argument count
        Type *func_type = node->call.callee->type;
        int actual_arg_count = 0;
        for (AST *arg = node->call.args; arg; arg = arg->next) {
            actual_arg_count++;
        }
        
        if (actual_arg_count != func_type->param_count) {
            char buf[256];
            snprintf(buf, sizeof(buf), 
                    "Function called with %d arguments but expects %d",
                    actual_arg_count, func_type->param_count);
            error(buf, node);
            node->type = NULL;
            break;
        }
        
        // Check argument types match parameter types
        AST *arg = node->call.args;
        for (int i = 0; i < func_type->param_count; i++) {
            if (!arg || !arg->type) {
                error("Argument has no type", node);
                node->type = NULL;
                break;
            }
            
            Type *param_type = func_type->params[i];
            Type *arg_type = arg->type;
            
            // Check if types are compatible
            if (param_type->kind == TY_ARRAY) {
                // Parameter expects an array/pointer
                if (arg_type->kind != TY_ARRAY) {
                    char buf[256];
                    snprintf(buf, sizeof(buf),
                            "Argument %d: expected array type but got %s",
                            i + 1, type_to_string(arg_type));
                    error(buf, node);
                    node->type = NULL;
                    break;
                }
                // Check element types match
                if (!types_equal(param_type->array_of, arg_type->array_of)) {
                    char buf[256];
                    snprintf(buf, sizeof(buf),
                            "Argument %d: array element types do not match (expected %s[], got %s[])",
                            i + 1, 
                            type_to_string(param_type->array_of),
                            type_to_string(arg_type->array_of));
                    error(buf, node);
                    node->type = NULL;
                    break;
                }
            } else if (arg_type->kind == TY_ARRAY) {
                // Argument is array but parameter is not
                char buf[256];
                snprintf(buf, sizeof(buf),
                        "Argument %d: cannot pass array to non-array parameter (expected %s, got %s)",
                        i + 1,
                        type_to_string(param_type),
                        type_to_string(arg_type));
                error(buf, node);
                node->type = NULL;
                break;
            } else {
                // Both are non-arrays - check basic type compatibility
                // Allow implicit conversions between numeric types
                bool param_numeric = is_numeric(param_type);
                bool arg_numeric = is_numeric(arg_type);
                
                if (param_numeric != arg_numeric) {
                    // One is numeric, other is not - incompatible
                    char buf[256];
                    snprintf(buf, sizeof(buf),
                            "Argument %d: type mismatch (expected %s, got %s)",
                            i + 1,
                            type_to_string(param_type),
                            type_to_string(arg_type));
                    error(buf, node);
                    node->type = NULL;
                    break;
                }
                
                // For non-numeric types, they must match exactly
                if (!param_numeric && !types_equal(param_type, arg_type)) {
                    char buf[256];
                    snprintf(buf, sizeof(buf),
                            "Argument %d: type mismatch (expected %s, got %s)",
                            i + 1,
                            type_to_string(param_type),
                            type_to_string(arg_type));
                    error(buf, node);
                    node->type = NULL;
                    break;
                }
            }
            
            arg = arg->next;
        }
        
        node->type = func_type->return_type;
        break;
    }

    case AST_BLOCK:
        enter_scope();
        for (int i = 0; i < node->block.count; i++) {
            AST *stmt = node->block.statements[i];
            
            if (is_expression_statement(stmt)) {
                check_expression_statement(stmt);
            } else {
                type_check_node(stmt);
            }
        }
        exit_scope();
        node->type = type_void();
        break;

    case AST_IF:
        type_check_node(node->if_stmt.cond);
        type_check_node(node->if_stmt.then_branch);
        if (node->if_stmt.else_branch) {
            type_check_node(node->if_stmt.else_branch);
        }
        node->type = type_void();
        break;

    case AST_WHILE:
        type_check_node(node->while_stmt.cond);
        type_check_node(node->while_stmt.body);
        node->type = type_void();
        break;

    case AST_DO_WHILE:
        type_check_node(node->do_while.body);
        type_check_node(node->do_while.cond);
        node->type = type_void();
        break;

    case AST_FOR:
        enter_scope();
        if (node->for_stmt.init) {
            type_check_node(node->for_stmt.init);
        }
        if (node->for_stmt.cond) {
            type_check_node(node->for_stmt.cond);
        }
        if (node->for_stmt.post) {
            type_check_node(node->for_stmt.post);
        }
        type_check_node(node->for_stmt.body);
        exit_scope();
        node->type = type_void();
        break;

    case AST_RETURN:
        if (node->ret.expr) {
            type_check_node(node->ret.expr);
            node->type = node->ret.expr->type;

            // Check if return type matches function return type
            if (current_function_return_type) {
                if (current_function_return_type->kind == TY_VOID) {
                    error("Cannot return a value from void function", node);
                } else if (!node->ret.expr->type) {
                    error("Return expression has no type", node);
                } else if (!types_equal(current_function_return_type, node->ret.expr->type)) {
                    // Allow implicit conversions between numeric types
                    bool ret_numeric = is_numeric(current_function_return_type);
                    bool expr_numeric = is_numeric(node->ret.expr->type);

                    if (!(ret_numeric && expr_numeric)) {
                        char buf[256];
                        snprintf(buf, sizeof(buf),
                                "Return type mismatch: expected %s, got %s",
                                type_to_string(current_function_return_type),
                                type_to_string(node->ret.expr->type));
                        error(buf, node);
                    }
                }
            }
        } else {
            node->type = type_void();

            // Check if function expects a return value
            if (current_function_return_type && 
                    current_function_return_type->kind != TY_VOID) {
                char buf[256];
                snprintf(buf, sizeof(buf),
                        "Function expects return type %s but return statement has no value",
                        type_to_string(current_function_return_type));
                error(buf, node);
            }
        }
        break;

    case AST_BREAK:
    case AST_CONTINUE:
        node->type = type_void();
        break;

    default:
        error("Unrecognized AST node kind in type checker", node);
        node->type = NULL;
        break;
    }
}

// Main entry point for type checking
void type_check(AST *node) {
    type_check_node(node);
}


// Type check a program (top-level statements)
void type_check_program(AST *root) {
    if (!root) return;
    
    // If root is a block, process each statement
    if (root->kind == AST_BLOCK) {
        for (int i = 0; i < root->block.count; i++) {
            AST *stmt = root->block.statements[i];
            
            // Check if this is an expression statement (followed by semicolon)
            // For expression statements, output the type
            if (is_expression_statement(stmt)) {
                check_expression_statement(stmt);
            } else {
                type_check_node(stmt);
            }
        }
    } else {
        type_check_node(root);
    }
}
