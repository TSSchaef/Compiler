#include <stdio.h>
#include <stdlib.h>

#include "logging.h"

// For lex
extern FILE *yyin;
extern char *filename;
int yylex(void);

int main(int argc, char *argv[]){
    switch(handleInputs(argv, argc)){
        case 1:
            logCompilerInfo();
            break;

        case 2:
            FILE *file = fopen(argv[2], "r");
            if(file == NULL){
                fprintf(stderr, "Could not open file %s\n", argv[2]);
                return -1;
            }

            filename = argv[2];
            yyin = file;

            yylex();

            break;
        case 3:
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
