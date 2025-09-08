#include "lexer.h"

bool isWhitespace(char c) {
    return c == ' ' || c == '\n' || c == '\r'||
        c == '\t' || c == '\v' || c == '\f';
}

bool isDelimiter(char c) {
    return c == '!' || c == '%' || c == '&' || c == '(' || c == ')' ||
        c == '*' || c == '+' || c == ',' || c == '-' || c == '.' ||
        c == '/' || c == ':' || c == ';' || c == '<' || c == '=' ||
        c == '>' || c == '?' || c == '[' || c == ']' || c == '{' ||
        c == '|' || c == '}' || c == '~' ||
        isWhitespace(c);
}

int isTwoCharOperator(char *c) {
    if(c == NULL || c[0] == '\0' || c[1] == '\0'){
        logBadInput("isTwoCharOperator");
        return -1;
    }

    if( c[0] == '=' && c[1] == '=') return 351;
    if( c[0] == '!' && c[1] == '=') return 352;
    if( c[0] == '>' && c[1] == '=') return 353;
    if( c[0] == '<' && c[1] == '=') return 354;
    if( c[0] == '+' && c[1] == '+') return 355;
    if( c[0] == '-' && c[1] == '-') return 356;
    if( c[0] == '|' && c[1] == '|') return 357;
    if( c[0] == '&' && c[1] == '&') return 358;

    if( c[0] == '+' && c[1] == '=') return 361;
    if( c[0] == '-' && c[1] == '=') return 362;
    if( c[0] == '*' && c[1] == '=') return 363;
    if( c[0] == '/' && c[1] == '=') return 364;

    if( c[0] == '-' && c[1] == '>') return 365;

    if( c[0] == '%' && c[1] == '=') return 366;

    return 0;
}

bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool isValidIdentifier(char* str) {
    if (str == NULL || str[0] == '\0' 
            || isDelimiter(str[0]) || isDigit(str[0])){

        return false;
    }

    for (int i = 1; str[i] != '\0'; i++) {
        if (isDelimiter(str[i]) || isDigit(str[i])){
            return false;
        }
    }

    return true;
}

bool isKeyword(char* str) {
    const char* keywords[] = {
        "const", "struct", "for", "while", "do", "if", "else", 
        "break", "continue", "return", "switch", "case", "default",
        "true", "false", "bool"
    };

    int numKeywords = sizeof(keywords) / sizeof(keywords[0]);

    for (int i = 0; i < numKeywords; i++) {
        if (strcmp(str, keywords[i]) == 0) {
            return true;
        }
    }

    return false;
}



int lexer(char *filename){
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        fprintf(stderr, "Could not open file %s\n", filename);
        return -1;
    }

    return 0;
}
