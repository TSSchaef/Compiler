#include "lexer.h"

bool isDelimiter(char c) {
    return c == ' ' || c == '+' || c == '-' || c == '*' || c == '/' ||
           c == ',' || c == ';' || c == '>' || c == '<' || c == '=' ||
           c == '(' || c == ')' || c == '[' || c == ']' || c == '{' ||
           c == '}' || c == '\n' || c == '\t';
}

bool isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' ||
           c == '>' || c == '<' || c == '=';
}




int lexer(char *filename){
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        fprintf(stderr, "Could not open file %s\n", filename);
        return -1;
    }

    return 0;
}
