#include "typecheck.h"
#include "global.h"
#include "ast.h"
#include "symtab.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static Type *current_function_return_type = NULL;
static bool in_function = false;

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
    t->struct_name = NULL;
    t->members = NULL;
    t->member_count = 0;
    return t;
}

Type *type_char() {
    return type_char_const(false);
}

Type *type_char_const(bool is_const) {
    Type *t = malloc(sizeof(Type));
    t->kind = TY_CHAR;
    t->is_const = is_const;
    t->struct_name = NULL;
    t->members = NULL;
    t->member_count = 0;
    return t;
}

Type *type_float() {
    return type_float_const(false);
}

Type *type_float_const(bool is_const) {
    Type *t = malloc(sizeof(Type));
    t->kind = TY_FLT;
    t->is_const = is_const;
    t->struct_name = NULL;
    t->members = NULL;
    t->member_count = 0;
    return t;
}

Type *type_void() {
    return type_void_const(false);
}

Type *type_void_const(bool is_const) {
    Type *t = malloc(sizeof(Type));
    t->kind = TY_VOID;
    t->is_const = is_const;
    t->struct_name = NULL;
    t->members = NULL;
    t->member_count = 0;
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
    t->struct_name = NULL;
    t->members = NULL;
    t->member_count = 0;
    return t;
}

Type *type_func(Type *ret, Type **params, int param_count) {
    Type *ft = malloc(sizeof(Type));
    ft->kind = TY_FUNC;
    ft->return_type = ret;
    ft->params = params;
    ft->param_count = param_count;
    ft->struct_name = NULL;
    ft->members = NULL;
    ft->member_count = 0;
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
        case TY_STRUCT:
            snprintf(buf, sizeof(buf), "%sstruct %s", const_prefix, 
                     t->struct_name ? t->struct_name : "<anonymous>");
            return buf;
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
                case TY_STRUCT:
                    snprintf(buf, sizeof(buf), "%sstruct %s[]", elem_const,
                             elem->struct_name ? elem->struct_name : "<anonymous>");
                    return buf;
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
    
    if (t1->kind == TY_STRUCT) {
        // Structs are equal if they have the same name
        if (!t1->struct_name || !t2->struct_name) return false;
        return strcmp(t1->struct_name, t2->struct_name) == 0;
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
        case AST_STRUCT_DEF:
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
        case AST_MEMBER_ACCESS:
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
        if(mode == 4){
            fprintf(outputFile, "File %s Line %d: expression has type %s\n",
                    expr->filename,
                    ast_get_line_no(expr),
                    type_to_string(expr->type));
        }
    }
}

static void link_struct_members(Type *t) {
    if (!t) return;
    
    if (t->kind == TY_STRUCT && t->struct_name) {
        // If this struct type doesn't have members, look them up
        if (!t->members || t->member_count == 0) {
            Type *struct_def = lookup_struct(t->struct_name);
            if (struct_def) {
                t->members = struct_def->members;
                t->member_count = struct_def->member_count;
            }
        }
    } else if (t->kind == TY_ARRAY) {
        // Recursively handle array element types
        link_struct_members(t->array_of);
    }
}

// Returns true if 'from' can be widened to 'to'
static bool can_widen_to(Type *from, Type *to) {
    if (!from || !to) return false;
    
    // Same type - no widening needed
    if (from->kind == to->kind) return true;
    
    // char -> int -> float (widening chain)
    if (from->kind == TY_CHAR && (to->kind == TY_INT || to->kind == TY_FLT)) {
        return true;
    }
    
    if (from->kind == TY_INT && to->kind == TY_FLT) {
        return true;
    }
    
    return false;
}


static void type_check_node(AST *node) {
    if (!node) return;

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
            char buf[256];
            snprintf(buf, sizeof(buf), "Undeclared identifier '%s'", node->id);
            error(buf, node);
            node->type = NULL;
        } else {
            node->type = s->type;
            node->symbol = copy_symbol(s);
        }
        break;
    }

    case AST_MEMBER_ACCESS: {
        type_check_node(node->member.object);
        
        if (!node->member.object->type) {
            error("Member access on expression with no type", node);
            node->type = NULL;
            break;
        }
        
        Type *obj_type = node->member.object->type;
        
        // Check if the object is a struct type
        if (obj_type->kind != TY_STRUCT) {
            char buf[256];
            snprintf(buf, sizeof(buf), 
                    "Member access on non-struct type %s",
                    type_to_string(obj_type));
            error(buf, node);
            node->type = NULL;
            break;
        }
        
        // Find the member in the struct
        StructMember *member = struct_member_find(obj_type, node->member.member_name);
        if (!member) {
            char buf[256];
            snprintf(buf, sizeof(buf), 
                    "Struct %s has no member named '%s'",
                    obj_type->struct_name, node->member.member_name);
            error(buf, node);
            node->type = NULL;
            break;
        }
        
        // The type of s.m is the type of member m
        node->type = member->type;
        
        // If the struct variable is const, all members become const
        if (obj_type->is_const && node->type) {
            // Create a copy of the member type with const set
            Type *const_type = malloc(sizeof(Type));
            memcpy(const_type, node->type, sizeof(Type));
            const_type->is_const = true;
            node->type = const_type;
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
           char buf[256];
           const char *array_name = (node->array.array->kind == AST_ID) ? 
               node->array.array->id : "<expression>";
           snprintf(buf, sizeof(buf), 
                   "Subscripted value '%s' is not an array (has type %s)", 
                   array_name, type_to_string(node->array.array->type));
           error(buf, node);
           node->type = NULL;
           break;
       }

       if (!node->array.index->type || !is_integral(node->array.index->type)) {
           char buf[256];
           snprintf(buf, sizeof(buf), 
                   "Array subscript has non-integer type %s", 
                   node->array.index->type ? type_to_string(node->array.index->type) : "unknown");
           error(buf, node);
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
            char buf[256];
            snprintf(buf, sizeof(buf), 
                    "Invalid operands to binary %s (have %s and %s)",
                    binop_to_string(node->binop.op),
                    node->binop.left->type ? type_to_string(node->binop.left->type) : "unknown",
                    node->binop.right->type ? type_to_string(node->binop.right->type) : "unknown");
            error(buf, node);
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
                    char buf[256];
                    snprintf(buf, sizeof(buf), 
                            "Invalid operands to binary %s (have %s and %s)",
                            binop_to_string(node->binop.op),
                            type_to_string(node->binop.left->type),
                            type_to_string(node->binop.right->type));
                    error(buf, node);
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
                char buf[256];
                snprintf(buf, sizeof(buf), 
                        "Invalid operands to binary %s (have %s and %s)",
                        binop_to_string(node->binop.op),
                        type_to_string(node->binop.left->type),
                        type_to_string(node->binop.right->type));
                error(buf, node);
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
            char buf[256];
            snprintf(buf, sizeof(buf), 
                    "Invalid operands in assignment (have %s and %s)",
                    node->assign.lhs->type ? type_to_string(node->assign.lhs->type) : "unknown",
                    node->assign.rhs->type ? type_to_string(node->assign.rhs->type) : "unknown");
            error(buf, node);
            node->type = NULL;
            break;
        }
        
        // Check basic type compatibility
        if (node->assign.lhs->type->kind == TY_VOID || 
            node->assign.rhs->type->kind == TY_VOID) {
            error("Cannot assign to or from void type", node);
            node->type = NULL;
            break;
        }

        // Check if trying to assign to const
        if (node->assign.lhs->type->is_const) {
             char buf[256];
             const char *var_name = (node->assign.lhs->kind == AST_ID) ? 
                 node->assign.lhs->id : "<expression>";
             snprintf(buf, sizeof(buf), 
                     "Assignment to read-only variable '%s' (type %s)",
                     var_name, type_to_string(node->assign.lhs->type));
             error(buf, node);
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
                    // For += and -=, RHS must be integral (allow widening from char to int)
                    if (!is_integral(node->assign.rhs->type) && 
                            node->assign.rhs->type->kind != TY_CHAR) {
                        error("Pointer arithmetic requires integer operand", node);
                        node->type = NULL;
                        break;
                    }
                } else {
                    // For non-array types, both sides must be numeric
                    // Allow widening: RHS can be widened to match LHS type
                    if (!is_numeric(node->assign.lhs->type) && 
                            node->assign.lhs->type->kind != TY_CHAR) {
                        error("Compound assignment requires arithmetic types", node);
                        node->type = NULL;
                        break;
                    }

                    // Check if RHS can be widened to LHS type
                    if (!can_widen_to(node->assign.rhs->type, node->assign.lhs->type)) {
                        char buf[256];
                        snprintf(buf, sizeof(buf),
                                "Compound assignment type mismatch: cannot assign %s to %s",
                                type_to_string(node->assign.rhs->type),
                                type_to_string(node->assign.lhs->type));
                        error(buf, node);
                        node->type = NULL;
                        break;
                    }
                }
            } else if (node->assign.op == AOP_MOD_ASSIGN) {
                // Modulo requires integral types on both sides
                // Allow char to be widened to int
                Type *lhs_t = node->assign.lhs->type;
                Type *rhs_t = node->assign.rhs->type;

                bool lhs_ok = (lhs_t->kind == TY_INT || lhs_t->kind == TY_CHAR);
                bool rhs_ok = (rhs_t->kind == TY_INT || rhs_t->kind == TY_CHAR);

                if (!lhs_ok || !rhs_ok) {
                    error("Modulo assignment requires integer types", node);
                    node->type = NULL;
                    break;
                }
            } else {
                // Bitwise compound assignments require integral types
                // Allow char to be widened to int
                Type *lhs_t = node->assign.lhs->type;
                Type *rhs_t = node->assign.rhs->type;

                bool lhs_ok = (lhs_t->kind == TY_INT || lhs_t->kind == TY_CHAR);
                bool rhs_ok = (rhs_t->kind == TY_INT || rhs_t->kind == TY_CHAR);

                if (!lhs_ok || !rhs_ok) {
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
            } else if (node->assign.lhs->type->kind == TY_STRUCT || 
                       node->assign.rhs->type->kind == TY_STRUCT) {
                // Struct assignments must match types exactly
                if (!types_equal(node->assign.lhs->type, node->assign.rhs->type)) {
                    error("Type mismatch: cannot assign different struct types", node);
                    node->type = NULL;
                    break;
                }
            } else {
                // Both are non-arrays - allow implicit conversions between arithmetic types
                if (!can_widen_to(node->assign.rhs->type,
                            node->assign.lhs->type)) {
                    char buf[256];
                    snprintf(buf, sizeof(buf),
                            "Type mismatch in assignment: cannot assign %s to %s",
                            type_to_string(node->assign.rhs->type),
                            type_to_string(node->assign.lhs->type));
                    error(buf, node);
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
                        char buf[256];
                        const char *var_name = (node->unary.operand->kind == AST_ID) ? 
                            node->unary.operand->id : "<expression>";
                        snprintf(buf, sizeof(buf), 
                                "Cannot %s read-only variable '%s' (type %s)",
                                (node->unary.op == UOP_PRE_INC || node->unary.op == UOP_POST_INC) ? 
                                "increment" : "decrement",
                                var_name, type_to_string(node->unary.operand->type));
                        error(buf, node);
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
        if (node->decl.init) {
            type_check_node(node->decl.init);
        }

        // Ensure struct types have their members properly linked
        link_struct_members(node->decl.decl_type);

        if (node->decl.decl_type && node->decl.decl_type->kind == TY_ARRAY) {
            if (node->decl.init) {
                // If init is a string literal, capture its length
                if (node->decl.init->kind == AST_STRING_LITERAL) {
                    node->decl.decl_type->array_size = strlen(node->decl.init->strval) + 1;
                }
                // If init is an array literal, capture its size (would need AST support)
                // For now, default to a reasonable size if not specified
                else if (node->decl.decl_type->array_size == 0) {
                    node->decl.decl_type->array_size = 10;
                }
            } else if (node->decl.decl_type->array_size == 0) {
                node->decl.decl_type->array_size = 10;
            }
        }

        // Check if it's a struct type declaration
        if (node->decl.decl_type && node->decl.decl_type->kind == TY_STRUCT) {
            // ... existing struct code ...
        }

        if(!add_symbol(node->decl.name, node->decl.decl_type)){
            char buf[256];
            snprintf(buf, sizeof(buf), 
                    "Redeclaration of variable '%s'", node->decl.name);
            error(buf, node);
        }

        if(node->decl.decl_type->kind == TY_VOID){
            char buf[256];
            snprintf(buf, sizeof(buf), 
                    "Variable '%s' declared void", node->decl.name);
            error(buf, node);
        }

        node->type = node->decl.decl_type;
        node->symbol = copy_symbol(lookup_symbol(node->decl.name));
        break;

    case AST_STRUCT_DEF: {
         // Process struct definition
         const char *struct_name = node->struct_def.name;

         // Check if struct is already defined in current scope
         Type *existing = lookup_struct_current(struct_name);
         if (existing) {
             char buf[256];
             snprintf(buf, sizeof(buf), 
                     "Redefinition of struct '%s'", struct_name);
             error(buf, node);
             node->type = NULL;
             break;
         }

         // Build member list
         StructMember *members = NULL;
         StructMember *last_member = NULL;
         int member_count = 0;

         for (AST *member_node = node->struct_def.members; member_node; 
                 member_node = member_node->next) {

             if (member_node->kind != AST_DECL) continue;

             // Make sure struct member types have their members linked
             link_struct_members(member_node->decl.decl_type);

             StructMember *m = struct_member_create(member_node->decl.name, 
                     member_node->decl.decl_type);

             if (!members) {
                 members = m;
             } else {
                 last_member->next = m;
             }
             last_member = m;
             member_count++;
         }

         // Create struct type and add to symbol table
         Type *struct_type = type_struct(struct_name, members, member_count);

         if (!add_struct(struct_name, struct_type)) {
             char buf[256];
             snprintf(buf, sizeof(buf), 
                     "Failed to add struct '%s' to symbol table", struct_name);
             error(buf, node);
         }

         node->type = struct_type;
         break;
     }


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
               char buf[256];
               Symbol *existing = lookup_symbol_current(node->func.name);
               snprintf(buf, sizeof(buf), 
                       "Redeclaration of function '%s'", node->func.name);
               error(buf, node);
           }

           enter_scope();  // Enter function scope

           // Add parameters to function scope (they are local variables)
           int param_index = 0;
           for (AST *p = param; p; p = p->next) {
               // Check if parameter is a struct type
               if (p->decl.decl_type && p->decl.decl_type->kind == TY_STRUCT) {
                   Type *struct_def = lookup_struct(p->decl.decl_type->struct_name);
                   if (!struct_def) {
                       char buf[256];
                       snprintf(buf, sizeof(buf), 
                               "Parameter '%s' in function '%s' declared with undefined struct type '%s'",
                               p->decl.name, node->func.name, p->decl.decl_type->struct_name);
                       error(buf, node);
                   }
               }

               if(!add_symbol(p->decl.name, p->decl.decl_type)){
                   char buf[256];
                   snprintf(buf, sizeof(buf), 
                           "Redeclaration of parameter '%s' in function '%s'",
                           p->decl.name, node->func.name);
                   error(buf, node);
               } else {
                   Symbol *param_sym = lookup_symbol_current(p->decl.name);
                   if (param_sym) {
                       param_sym->is_local = true;
                       param_sym->local_index = param_index;
                   }
                   // Store symbol in AST node for later IR generation
                   param_index++;
               }

               if(p->decl.decl_type->kind == TY_VOID){
                   char buf[256];
                   snprintf(buf, sizeof(buf), 
                           "Parameter '%s' in function '%s' declared void",
                           p->decl.name, node->func.name);
                   error(buf, node);
               }
           }

           set_local_count(param_count);
           //printf("Function '%s' has %d parameters\n", node->func.name, param_index);
           
           Type *prev_return_type = current_function_return_type;
           current_function_return_type = node->func.return_type;
            
            // Indicate we are inside a function and don't enter_scope for the body again
           in_function = true;

           type_check_node(node->func.body);

           current_function_return_type = prev_return_type;

           exit_scope();  // Exit function scope

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
            } else if (param_type->kind == TY_STRUCT || arg_type->kind == TY_STRUCT) {
                // Struct types must match exactly
                if (!types_equal(param_type, arg_type)) {
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
            } else {
                // Both are non-arrays - check basic type compatibility
                // Allow implicit conversions between numeric types
                if (!can_widen_to(arg_type, param_type)) {
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

     case AST_BLOCK: {
        bool should_skip_scope = in_function;
        in_function = false;

        if(!should_skip_scope) enter_scope();

        for (int i = 0; i < node->block.count; i++) {
            AST *stmt = node->block.statements[i];
            
            if (is_expression_statement(stmt)) {
                check_expression_statement(stmt);
            } else {
                type_check_node(stmt);
            }
        }
        if(!should_skip_scope) exit_scope();
        node->type = type_void();
        break;
    }

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
                } else if (!can_widen_to(node->ret.expr->type, current_function_return_type)) {
                    // Type doesn't match and can't be widened
                    char buf[256];
                    snprintf(buf, sizeof(buf),
                            "Return type mismatch: expected %s, got %s",
                            type_to_string(current_function_return_type),
                            type_to_string(node->ret.expr->type));
                    error(buf, node);
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
