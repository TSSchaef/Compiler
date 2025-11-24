#include "jbcgen.h"
#include "symtab.h"
#include <stdio.h>
#include <string.h>

// Extract class name from filename (removes path and .j extension)
static char* get_classname_from_output(const char *filename) {
    const char *base = strrchr(filename, '/');
    if (!base) base = strrchr(filename, '\\');
    base = base ? base + 1 : filename;
    
    char *classname = strdup(base);
    char *dot = strrchr(classname, '.');
    if (dot) {
        *dot = '\0';
    }
    return classname;
}

void emit_class_header(FILE *out, const char *classname) {
    fprintf(out, ".class public %s\n", classname);
    fprintf(out, ".super java/lang/Object\n\n");
}

void emit_global_field(FILE *out, const char *name, Type *type) {
    const char *type_desc = "I"; // default to int
    
    if (type) {
        switch(type->kind) {
            case TY_INT: type_desc = "I"; break;
            case TY_FLT: type_desc = "F"; break;
            case TY_CHAR: type_desc = "C"; break;
            case TY_VOID: type_desc = "V"; break;
            case TY_ARRAY: type_desc = "[I"; break; // simplified - array of int
            case TY_FUNC: type_desc = "I"; break; // shouldn't happen for fields
            case TY_STRUCT: type_desc = "Ljava/lang/Object;"; break; // treat as object
            default: type_desc = "I"; break;
        }
    }
    
    fprintf(out, ".field public static %s %s\n", name, type_desc);
}

static const char* get_type_descriptor(Type *type) {
    if (!type) return "I";
    
    switch(type->kind) {
        case TY_VOID: return "V";
        case TY_INT: return "I";
        case TY_FLT: return "F";
        case TY_CHAR: return "C";
        case TY_ARRAY: 
            // For arrays, return the array descriptor
            if (type->array_of) {
                if (type->array_of->kind == TY_CHAR) return "[C";
                if (type->array_of->kind == TY_INT) return "[I";
                if (type->array_of->kind == TY_FLT) return "[F";
            }
            return "[I"; // default to int array
        case TY_FUNC: return "I"; // shouldn't happen as a descriptor
        case TY_STRUCT: return "Ljava/lang/Object;";
        default: return "I";
    }
}

static void emit_method_signature(FILE *out, const char *classname, const char *name, Type *return_type, AST *params) {
    fprintf(out, "\n.method public static %s(", name);
    
    // Emit parameter types
    AST *p = params;
    while (p) {
        if (p->kind == AST_DECL) {
            fprintf(out, "%s", get_type_descriptor(p->decl.decl_type));
        }
        p = p->next;
    }
    
    fprintf(out, ")%s\n", get_type_descriptor(return_type));
}

void emit_method_header(FILE *out, const char *classname, const char *name, Type *return_type, AST *params) {
    fprintf(out, "\n.method public static %s : (", name);
    
    // Emit parameter types
    AST *p = params;
    while (p) {
        if (p->kind == AST_DECL) {
            fprintf(out, "%s", get_type_descriptor(p->decl.decl_type));
        }
        p = p->next;
    }
    
    fprintf(out, ")%s\n", get_type_descriptor(return_type));
    fprintf(out, ".code stack 32 locals 32\n");
}

void emit_method_footer(FILE *out) {
    fprintf(out, ".end code\n");
    fprintf(out, ".end method\n\n");
}

static void emit_comparison(FILE *out, IRKind kind) {
    static int label_counter = 0;
    int true_label = label_counter++;
    int end_label = label_counter++;
    
    const char *cmp_instr;
    switch(kind) {
        case IR_EQ: cmp_instr = "if_icmpeq"; break;
        case IR_NEQ: cmp_instr = "if_icmpne"; break;
        case IR_LT: cmp_instr = "if_icmplt"; break;
        case IR_GT: cmp_instr = "if_icmpgt"; break;
        case IR_LE: cmp_instr = "if_icmple"; break;
        case IR_GE: cmp_instr = "if_icmpge"; break;
        default: return;
    }
    
    fprintf(out, "    %s L%d\n", cmp_instr, true_label);
    fprintf(out, "    ldc 0\n");
    fprintf(out, "    goto L%d\n", end_label);
    fprintf(out, "L%d:\n", true_label);
    fprintf(out, "    ldc 1\n");
    fprintf(out, "L%d:\n", end_label);
}

void emit_java_from_ir(FILE *out, const char *classname, IRList *ir) {
    for (IRInstruction *p = ir->head; p; p = p->next) {
        switch(p->kind) {
            case IR_PUSH_INT:
                // Use iconst for small constants, ldc for larger ones
                if (p->i == -1) {
                    fprintf(out, "    iconst_m1\n");
                } else if (p->i >= 0 && p->i <= 5) {
                    fprintf(out, "    iconst_%d\n", p->i);
                } else if (p->i >= -128 && p->i <= 127) {
                    fprintf(out, "    bipush %d\n", p->i);
                } else if (p->i >= -32768 && p->i <= 32767) {
                    fprintf(out, "    sipush %d\n", p->i);
                } else {
                    fprintf(out, "    ldc %d\n", p->i);
                }
                break;
                
            case IR_PUSH_FLOAT:
                fprintf(out, "    ldc %f\n", p->f);
                break;
                
            case IR_PUSH_STRING:
                fprintf(out, "    ldc \"%s\"\n", p->s);
                // Convert Java string to C char array for lib440
                fprintf(out, "    invokestatic Method lib440 java2c (Ljava/lang/String;)[C\n");
                break;
                
            case IR_LOAD_GLOBAL:
                fprintf(out, "    getstatic Field %s %s I\n", classname, p->s);
                break;
                
            case IR_STORE_GLOBAL:
                fprintf(out, "    putstatic Field %s %s I\n", classname, p->s);
                break;
                
            case IR_LOAD_LOCAL:
                fprintf(out, "    iload %d\n", p->i);
                break;
                
            case IR_STORE_LOCAL:
                fprintf(out, "    istore %d\n", p->i);
                break;
                
            case IR_ADD:
                fprintf(out, "    iadd\n");
                break;
                
            case IR_SUB:
                fprintf(out, "    isub\n");
                break;
                
            case IR_MUL:
                fprintf(out, "    imul\n");
                break;
                
            case IR_DIV:
                fprintf(out, "    idiv\n");
                break;
                
            case IR_MOD:
                fprintf(out, "    irem\n");
                break;
                
            case IR_NEG:
                fprintf(out, "    ineg\n");
                break;
                
            case IR_BIT_AND:
                fprintf(out, "    iand\n");
                break;
                
            case IR_BIT_OR:
                fprintf(out, "    ior\n");
                break;
                
            case IR_BIT_XOR:
                fprintf(out, "    ixor\n");
                break;
                
            case IR_BIT_NOT:
                fprintf(out, "    ldc -1\n");
                fprintf(out, "    ixor\n");
                break;
                
            case IR_SHL:
                fprintf(out, "    ishl\n");
                break;
                
            case IR_SHR:
                fprintf(out, "    ishr\n");
                break;
                
            case IR_EQ:
            case IR_NEQ:
            case IR_LT:
            case IR_GT:
            case IR_LE:
            case IR_GE:
                emit_comparison(out, p->kind);
                break;
                
            case IR_CALL:
                // Generate function call
                // Check if it's a standard library function
                if (is_stdlib_function(p->s)) {
                    // Call from lib440 class
                    if (strcmp(p->s, "getchar") == 0) {
                        fprintf(out, "    invokestatic Method lib440 getchar ()I\n");
                    } else if (strcmp(p->s, "putchar") == 0) {
                        fprintf(out, "    invokestatic Method lib440 putchar (I)I\n");
                    } else if (strcmp(p->s, "getint") == 0) {
                        fprintf(out, "    invokestatic Method lib440 getint ()I\n");
                    } else if (strcmp(p->s, "putint") == 0) {
                        fprintf(out, "    invokestatic Method lib440 putint (I)V\n");
                    } else if (strcmp(p->s, "getfloat") == 0) {
                        fprintf(out, "    invokestatic Method lib440 getfloat ()F\n");
                    } else if (strcmp(p->s, "putfloat") == 0) {
                        fprintf(out, "    invokestatic Method lib440 putfloat (F)V\n");
                    } else if (strcmp(p->s, "putstring") == 0) {
                        fprintf(out, "    invokestatic Method lib440 putstring ([C)V\n");
                    }
                } else {
                    // User-defined function - call from current class
                    fprintf(out, "    invokestatic Method %s %s (", classname, p->s);
                    // Parameter types - simplified, assumes all int
                    for (int i = 0; i < p->i; i++) {
                        fprintf(out, "I");
                    }
                    fprintf(out, ")I\n");
                }
                break;
                
            case IR_RETURN:
                fprintf(out, "    ireturn\n");
                break;
                
            case IR_RETURN_VOID:
                fprintf(out, "    return\n");
                break;
                
            case IR_POP:
                fprintf(out, "    pop\n");
                break;
                
            case IR_DUP:
                fprintf(out, "    dup\n");
                break;
                
            case IR_CAST_I2F:
                fprintf(out, "    i2f\n");
                break;
                
            case IR_CAST_F2I:
                fprintf(out, "    f2i\n");
                break;
                
            case IR_CAST_I2D:
                fprintf(out, "    i2d\n");
                break;
                
            case IR_CAST_D2I:
                fprintf(out, "    d2i\n");
                break;
                
            case IR_CAST_F2D:
                fprintf(out, "    f2d\n");
                break;
                
            case IR_CAST_D2F:
                fprintf(out, "    d2f\n");
                break;
                
            default:
                fprintf(out, "    ; unhandled IR op %d\n", p->kind);
        }
    }
}

void emit_init_method(FILE *out, const char *classname) {
    fprintf(out, ".method <init> : ()V\n");
    fprintf(out, ".code stack 1 locals 1\n");
    fprintf(out, "    aload_0\n");
    fprintf(out, "    invokespecial Method java/lang/Object <init> ()V\n");
    fprintf(out, "    return\n");
    fprintf(out, ".end code\n");
    fprintf(out, ".end method\n\n");
}

void emit_java_main(FILE *out, const char *classname) {
    fprintf(out, ".method public static main : ([Ljava/lang/String;)V\n");
    fprintf(out, ".code stack 1 locals 1\n");
    fprintf(out, "    invokestatic Method %s main ()I\n", classname);
    fprintf(out, "    invokestatic Method java/lang/System exit (I)V\n");
    fprintf(out, "    return\n");
    fprintf(out, ".end code\n");
    fprintf(out, ".end method\n");
}

// Collect all global variables from AST
static void collect_globals(AST *node, const char *classname) {
    if (!node) return;
    
    if (node->kind == AST_DECL && node->decl.name) {
        // This is a global variable declaration
        emit_global_field(outputFile, node->decl.name, node->decl.decl_type);
    }
    
    // Continue to next declaration
    if (node->next) {
        collect_globals(node->next, classname);
    }
}

// Generate code for a single function
static void generate_function(AST *func, const char *classname) {
    if (!func || func->kind != AST_FUNC) return;
    
    // Generate IR for this function
    IRList ir;
    generate_ir_from_ast(func, &ir);
    
    // Emit method header
    emit_method_header(outputFile, classname, func->func.name, 
                      func->func.return_type, func->func.params);
    
    // Emit bytecode from IR
    emit_java_from_ir(outputFile, classname, &ir);
    
    // Add default return if needed
    if (func->func.return_type && func->func.return_type->kind == TY_VOID) {
        fprintf(outputFile, "    return\n");
    } else if (!ir.tail || (ir.tail->kind != IR_RETURN && ir.tail->kind != IR_RETURN_VOID)) {
        fprintf(outputFile, "    ldc 0\n");
        fprintf(outputFile, "    ireturn\n");
    }
    
    // Emit method footer
    emit_method_footer(outputFile);
}

// Main code generation entry point
void generate_code(AST *program) {
    if (!program) {
        fprintf(stderr, "Code generation error: NULL program AST\n");
        return;
    }
    
    if (!outputFile) {
        fprintf(stderr, "Code generation error: outputFile is NULL\n");
        return;
    }
    
    // Get classname from output filename
    char *output_filename = getOutputFileName();
    char *classname = get_classname_from_output(output_filename);
    
    // Emit class header
    emit_class_header(outputFile, classname);
    
    // First pass: collect and emit global variables
    AST *node = program;
    while (node) {
        if (node->kind == AST_DECL) {
            collect_globals(node, classname);
        }
        node = node->next;
    }
    
    fprintf(outputFile, "\n");
    
    // Second pass: emit user-defined functions
    node = program;
    bool has_main = false;
    while (node) {
        if (node->kind == AST_FUNC) {
            if (strcmp(node->func.name, "main") == 0) {
                has_main = true;
            }
            generate_function(node, classname);
        }
        node = node->next;
    }
    
    // Emit special methods
    emit_init_method(outputFile, classname);
    
    // Emit Java main if C main exists
    if (has_main) {
        emit_java_main(outputFile, classname);
    }
    
    free(classname);
}
