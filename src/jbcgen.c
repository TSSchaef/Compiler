#include "jbcgen.h"
#include "symtab.h"
#include "ast.h"
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

void codegen_error(const char *msg, AST *node) {
    fprintf(stderr, "Code generation error in file %s line %d\n\t%s\n", 
            node->filename,
            ast_get_line_no(node), msg);
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
            case TY_ARRAY: 
                if (type->array_of) {
                    if (type->array_of->kind == TY_CHAR) type_desc = "[C";
                    else if (type->array_of->kind == TY_INT) type_desc = "[I";
                    else if (type->array_of->kind == TY_FLT) type_desc = "[F";
                    else type_desc = "[I";
                } else {
                    type_desc = "[I";
                }
                break;
            case TY_FUNC: type_desc = "I"; break;
            case TY_STRUCT: type_desc = "Ljava/lang/Object;"; break;
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
            if (type->array_of) {
                if (type->array_of->kind == TY_CHAR) return "[C";
                if (type->array_of->kind == TY_INT) return "[I";
                if (type->array_of->kind == TY_FLT) return "[F";
            }
            return "[I";
        case TY_FUNC: return "I";
        case TY_STRUCT: return "Ljava/lang/Object;";
        default: return "I";
    }
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
    fprintf(out, ".end method\n");
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
    fprintf(out, "    iconst_0\n");
    fprintf(out, "    goto L%d\n", end_label);
    fprintf(out, "L%d:\n", true_label);
    fprintf(out, "    iconst_1\n");
    fprintf(out, "L%d:\n", end_label);
}

static const char *array_load_opcode(Type *array_type) {
    if (!array_type || array_type->kind != TY_ARRAY) {
        return "iaload";
    }
    
    Type *elem = array_type->array_of;
    if (!elem) {
        return "iaload";
    }
    
    if (elem->kind == TY_INT)   return "iaload";
    if (elem->kind == TY_CHAR)  return "caload";
    if (elem->kind == TY_FLT)   return "faload";
    return "aaload";
}

static const char *array_store_opcode(Type *array_type) {
    if (!array_type || array_type->kind != TY_ARRAY) {
        return "iastore";
    }
    
    Type *elem = array_type->array_of;
    if (!elem) {
        return "iastore";
    }
    
    if (elem->kind == TY_INT)   return "iastore";
    if (elem->kind == TY_CHAR)  return "castore";
    if (elem->kind == TY_FLT)   return "fastore";
    return "aastore";
}

static const char *newarray_type(Type *elem_type) {
    if (!elem_type) return "int";
    
    switch (elem_type->kind) {
        case TY_INT: return "int";
        case TY_CHAR: return "char";
        case TY_FLT: return "float";
        default: return "int";
    }
}

void emit_java_from_ir(FILE *out, const char *classname, IRList *ir) {
    for (IRInstruction *p = ir->head; p; p = p->next) {
        switch(p->kind) {
            case IR_PUSH_INT:
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
                fprintf(out, "    ldc %s\n", p->s);
                fprintf(out, "    invokestatic Method lib440 java2c (Ljava/lang/String;)[C\n");
                break;
                
            case IR_LOAD_GLOBAL: {
                const char *type_desc = "I";
                if (p->symbol && p->symbol->type) {
                    type_desc = get_type_descriptor(p->symbol->type);
                }
                fprintf(out, "    getstatic Field %s %s %s\n", classname, p->s, type_desc);
                break;
            }
                
            case IR_STORE_GLOBAL: {
                const char *type_desc = "I";
                if (p->symbol && p->symbol->type) {
                    type_desc = get_type_descriptor(p->symbol->type);
                }
                fprintf(out, "    putstatic Field %s %s %s\n", classname, p->s, type_desc);
                break;
            }
                
            case IR_LOAD_LOCAL:
                fprintf(out, "    iload_%d\n", p->i);
                break;
                
            case IR_STORE_LOCAL:
                fprintf(out, "    istore_%d\n", p->i);
                break;

            case IR_ARRAY_LOAD: {
                Type *array_type = (p->symbol && p->symbol->type) ? p->symbol->type : NULL;
                fprintf(out, "    %s\n", array_load_opcode(array_type));
                break;
            }
                
            case IR_ARRAY_STORE: {
                Type *array_type = (p->symbol && p->symbol->type) ? p->symbol->type : NULL;
                fprintf(out, "    %s\n", array_store_opcode(array_type));
                break;
            }
            
            case IR_ALLOC_ARRAY: {
                Type *elem_type = NULL;
                if (p->symbol && p->symbol->type && p->symbol->type->kind == TY_ARRAY) {
                    elem_type = p->symbol->type->array_of;
                }
                fprintf(out, "    newarray %s\n", newarray_type(elem_type));
                break;
            }
                
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
                fprintf(out, "    iconst_m1\n");
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
                if (is_stdlib_function(p->s)) {
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
                    const char *return_desc = "I";
                    
                    if (p->symbol && p->symbol->type && p->symbol->type->kind == TY_FUNC) {
                        return_desc = get_type_descriptor(p->symbol->type->return_type);
                    }
                    
                    fprintf(out, "    invokestatic Method %s %s (", classname, p->s);
                    for (int i = 0; i < p->i; i++) {
                        fprintf(out, "I");
                    }
                    fprintf(out, ")%s\n", return_desc);
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
                
            case IR_DUP2:
                fprintf(out, "    dup2\n");
                break;
                
            case IR_DUP_X2:
                fprintf(out, "    dup_x2\n");
                break;
                
            case IR_CAST_I2F:
                fprintf(out, "    i2f\n");
                break;
                
            case IR_CAST_F2I:
                fprintf(out, "    f2i\n");
                break;
                
            default:
                break;
        }
    }
}

void emit_static_initializer(FILE *out, const char *classname, AST *program) {
    bool has_arrays = false;
    
    for (AST *n = program; n != NULL; n = n->next) {
        if (n->kind == AST_DECL && n->decl.decl_type && n->decl.decl_type->kind == TY_ARRAY) {
            has_arrays = true;
            break;
        } else if (n->kind == AST_BLOCK) {
            for (int i = 0; i < n->block.count; i++) {
                AST *stmt = n->block.statements[i];
                if (stmt->kind == AST_DECL && stmt->decl.decl_type && stmt->decl.decl_type->kind == TY_ARRAY) {
                    has_arrays = true;
                    break;
                }
            }
            if (has_arrays) break;
        }
    }
    
    if (!has_arrays) return;
    
fprintf(out, "\n.method static <clinit> : ()V\n");
    fprintf(out, ".code stack 10 locals 0\n");
    
    for (AST *n = program; n != NULL; n = n->next) {
        if (n->kind == AST_DECL && n->decl.decl_type && n->decl.decl_type->kind == TY_ARRAY) {
            // CHANGE THIS: Use actual array size
            int array_size = n->decl.decl_type->array_size > 0 ? 
                            n->decl.decl_type->array_size : 10;
            
            if (array_size <= 127) {
                fprintf(out, "    bipush %d\n", array_size);
            } else if (array_size <= 32767) {
                fprintf(out, "    sipush %d\n", array_size);
            } else {
                fprintf(out, "    ldc %d\n", array_size);
            }
            
            Type *elem = n->decl.decl_type->array_of;
            if (elem && elem->kind == TY_INT) {
                fprintf(out, "    newarray int\n");
            } else if (elem && elem->kind == TY_CHAR) {
                fprintf(out, "    newarray char\n");
            } else if (elem && elem->kind == TY_FLT) {
                fprintf(out, "    newarray float\n");
            } else {
                fprintf(out, "    newarray int\n");
            }
            
            fprintf(out, "    putstatic Field %s %s %s\n", 
                    classname, n->decl.name, get_type_descriptor(n->decl.decl_type));
        } else if (n->kind == AST_BLOCK) {
            for (int i = 0; i < n->block.count; i++) {
                AST *stmt = n->block.statements[i];
                if (stmt->kind == AST_DECL && stmt->decl.decl_type && stmt->decl.decl_type->kind == TY_ARRAY) {
                    // SAME CHANGES as above for block statements
                    int array_size = stmt->decl.decl_type->array_size > 0 ? 
                                    stmt->decl.decl_type->array_size : 10;
                    
                    if (array_size <= 127) {
                        fprintf(out, "    bipush %d\n", array_size);
                    } else if (array_size <= 32767) {
                        fprintf(out, "    sipush %d\n", array_size);
                    } else {
                        fprintf(out, "    ldc %d\n", array_size);
                    }
                    
                    Type *elem = stmt->decl.decl_type->array_of;
                    if (elem && elem->kind == TY_INT) {
                        fprintf(out, "    newarray int\n");
                    } else if (elem && elem->kind == TY_CHAR) {
                        fprintf(out, "    newarray char\n");
                    } else if (elem && elem->kind == TY_FLT) {
                        fprintf(out, "    newarray float\n");
                    } else {
                        fprintf(out, "    newarray int\n");
                    }
                    
                    fprintf(out, "    putstatic Field %s %s %s\n", 
                            classname, stmt->decl.name, get_type_descriptor(stmt->decl.decl_type));
                }
            }
        }
    }
    
    fprintf(out, "    return\n");
    fprintf(out, ".end code\n");
    fprintf(out, ".end method\n");
}

void emit_init_method(FILE *out, const char *classname) {
    fprintf(out, "\n.method <init> : ()V\n");
    fprintf(out, ".code stack 1 locals 1\n");
    fprintf(out, "    aload_0\n");
    fprintf(out, "    invokespecial Method java/lang/Object <init> ()V\n");
    fprintf(out, "    return\n");
    fprintf(out, ".end code\n");
    fprintf(out, ".end method\n");
}

void emit_java_main(FILE *out, const char *classname) {
    fprintf(out, "\n.method public static main : ([Ljava/lang/String;)V\n");
    fprintf(out, ".code stack 1 locals 1\n");
    fprintf(out, "    invokestatic Method %s main ()I\n", classname);
    fprintf(out, "    invokestatic Method java/lang/System exit (I)V\n");
    fprintf(out, "    return\n");
    fprintf(out, ".end code\n");
    fprintf(out, ".end method\n");
}

static void generate_function(FILE *out, AST *func, const char *classname) {
    if (!func || func->kind != AST_FUNC) return;

    IRList ir;
    generate_ir_from_ast(func, &ir);

    ir_print(&ir, stdout); // For debugging

    emit_method_header(out, classname, func->func.name, 
                      func->func.return_type, func->func.params);
    
    emit_java_from_ir(out, classname, &ir);
    
    if (func->func.return_type && func->func.return_type->kind == TY_VOID) {
        if (!ir.tail || ir.tail->kind != IR_RETURN_VOID) {
            fprintf(out, "    return\n");
        }
    }
    
    emit_method_footer(out);
}

static void emit_functions_from_ast(FILE *out, AST *node, const char *classname) {
    if (!node) return;

    for (AST *n = node; n != NULL; n = n->next) {
        if (n->kind == AST_FUNC) {
            generate_function(out, n, classname);
        } else if (n->kind == AST_BLOCK) {
            for (int i = 0; i < n->block.count; i++) {
                emit_functions_from_ast(out, n->block.statements[i], classname);
            }
        }
    }
}

static void emit_globals_from_ast(FILE *out, AST *node) {
    if (!node) return;
    
    for (AST *n = node; n != NULL; n = n->next) {
        if (n->kind == AST_DECL) {
            emit_global_field(out, n->decl.name, n->decl.decl_type);
        } else if (n->kind == AST_BLOCK) {
            for (int i = 0; i < n->block.count; i++) {
                emit_globals_from_ast(out, n->block.statements[i]);
            }
        }
    }
}

void generate_code(AST *program) {
    if (!program) {
        fprintf(stderr, "Code generation error: NULL program AST\n");
        return;
    }
    
    if (!outputFile) {
        fprintf(stderr, "Code generation error: outputFile is NULL\n");
        return;
    }
    
    char *output_filename = getOutputFileName();
    char *classname = get_classname_from_output(output_filename);
    
    emit_class_header(outputFile, classname);
    emit_globals_from_ast(outputFile, program);
    emit_static_initializer(outputFile, classname, program);
    emit_functions_from_ast(outputFile, program, classname);
    emit_init_method(outputFile, classname);
    emit_java_main(outputFile, classname);
    
    free(classname);
}
