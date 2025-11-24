#include <stdio.h>
#include <stdlib.h>

#include "logging.h"
#include "symtab.h"
#include "ast.h"
#include "typecheck.h"
#include "stack.h"
#include "jbcgen.h"

extern FILE *outputFile;

// For lex and bison
int yylex(void);
int yyparse(void);

int mode;
AST *root_ast;

int main(int argc, char *argv[]){
    switch(mode = handleInputs(argv, argc)){
        case 1:
            logCompilerInfo();
            break;

        case 2:
            if(pushFile(argv[2]) != 0){
               return -1;
            }

            while( yylex() != 0);
            break;

        case 3:
            if(pushFile(argv[2]) != 0){
               return -1;
            }

            yyparse();

            if(outputFile){
                fclose(outputFile);
            }

            break;

        case 4:
            if(pushFile(argv[2]) != 0){
               return -1;
            }

            init_symtab();
            yyparse();
            //ast_print(root_ast);
            type_check(root_ast);

            ast_free(root_ast);

            if(outputFile){
                fclose(outputFile);
            }

            break;

        case 5:
            if(pushFile(argv[2]) != 0){
               return -1;
            }

            init_symtab();
            yyparse();

            ast_print(root_ast);
            
            type_check(root_ast);
            generate_code(root_ast);

            ast_free(root_ast);

            if(outputFile){
                fclose(outputFile);
            }

            break;
        default:
            logImproperInput();
            break;
    }

    return 0;
}
