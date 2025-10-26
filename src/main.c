#include <stdio.h>
#include <stdlib.h>

#include "logging.h"
#include "stack.h"

// For lex and bison
int yylex(void);
int yyparse(void);

int mode;

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

            while(yyparse() != 0);
            break;

        case 4:
        case 5:
            logNotSupported();
            break;
        default:
            logImproperInput();
            break;
    }

    return 0;
}
