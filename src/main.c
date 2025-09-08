#include "logging.h"
#include "lexer.h"

int main(int argc, char *argv[]){
    switch(handleInputs(argv, argc)){
        case 1:
            logCompilerInfo();
            break;

        case 2:
            lexer(argv[2]);
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
