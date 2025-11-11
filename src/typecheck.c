#include "typecheck.h"
#include "ast.h"
#include "symtab.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Type constructors
Type *type_int() {
    static Type int_type = { .kind = TY_INT };
    return &int_type;
}

Type *type_char() {
    static Type char_type = { .kind = TY_CHAR };
    return &char_type;
}

Type *type_float() {
    static Type float_type = { .kind = TY_FLT };
    return &float_type;
}

Type *type_void() {
    static Type void_type = { .kind = TY_VOID };
    return &void_type;
}

Type *type_array(Type *elem_type) {
    Type *t = malloc(sizeof(Type));
    t->kind = TY_ARRAY;
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
    
    switch (t->kind) {
        case TY_INT: return "int";
        case TY_CHAR: return "char";
        case TY_FLT: return "float";
        case TY_VOID: return "void";
        case TY_ARRAY:
            const char *elem = type_to_string(t->array_of);
            size_t elem_len = strlen(elem);
            // Ensure we have space for elem + "[]" + null terminator
            if (elem_len + 3 <= sizeof(buf)) {
                snprintf(buf, sizeof(buf), "%s[]", elem);
            } else {
                strncpy(buf, "array", sizeof(buf) - 1);
                buf[sizeof(buf) - 1] = '\0';
            }
            return buf;        case TY_FUNC:
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
        
    case AST_STRING_LITERAL:
        node->type = type_array(type_char());
        break;
        
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
                error("Arithmetic operation requires numeric operands", node);
                node->type = NULL;
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
            node->type = type_int(); // Comparisons return int (boolean)
        }
        break;

    case AST_ASSIGN:
        type_check_node(node->assign.lhs);
        type_check_node(node->assign.rhs);
        
        if (!node->assign.lhs->type || !node->assign.rhs->type) {
            error("Invalid operands in assignment", node);
            node->type = NULL;
        } else {
            node->type = node->assign.lhs->type;
        }
        break;

    case AST_LOGICAL_OR:
    case AST_LOGICAL_AND:
        type_check_node(node->logical.left);
        type_check_node(node->logical.right);
        node->type = type_int(); // Logical operators return int (boolean)
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
        
        switch (node->unary.op) {
            case UOP_PLUS:
            case UOP_NEG:
                if (node->unary.operand->type && 
                    is_numeric(node->unary.operand->type)) {
                    node->type = node->unary.operand->type;
                } else {
                    error("Unary +/- requires numeric operand", node);
                    node->type = NULL;
                }
                break;
                
            case UOP_LOGICAL_NOT:
                node->type = type_int();
                break;
                
            case UOP_BITWISE_NOT:
                if (node->unary.operand->type && 
                    is_integral(node->unary.operand->type)) {
                    node->type = type_int();
                } else {
                    error("Bitwise NOT requires integral operand", node);
                    node->type = NULL;
                }
                break;
                
            case UOP_PRE_INC:
            case UOP_PRE_DEC:
            case UOP_POST_INC:
            case UOP_POST_DEC:
                if (node->unary.operand->type) {
                    node->type = node->unary.operand->type;
                } else {
                    node->type = NULL;
                }
                break;
                
            case UOP_CAST:
                node->type = node->unary.cast_type;
                break;
                
            default:
                node->type = node->unary.operand->type;
                break;
        }
        break;

    case AST_DECL:
        if (node->decl.init) {
            type_check_node(node->decl.init);
        }
        add_symbol(node->decl.name, node->decl.decl_type);
        node->type = node->decl.decl_type;
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
        add_symbol(node->func.name, ft);
        
        enter_scope();
        
        // Add parameters to scope
        for (AST *p = param; p; p = p->next) {
            add_symbol(p->decl.name, p->decl.decl_type);
        }
        
        type_check_node(node->func.body);
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
        
        if (node->call.callee->type && 
            node->call.callee->type->kind == TY_FUNC) {
            node->type = node->call.callee->type->return_type;
        } else {
            error("Called object is not a function", node);
            node->type = NULL;
        }
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
        } else {
            node->type = type_void();
        }
        break;

    case AST_BREAK:
    case AST_CONTINUE:
        node->type = type_void();
        break;

    default:
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
