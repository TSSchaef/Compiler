%{
    /* C code section */
    // Definitions and includes
    #include <stdio.h>

    int lineNumber = 1;
    char *filename;

    void printToken(int token){
        printf("File %s Line %d Token %d Text %s\n", 
            filename, lineNumber, token, yytext);
    }

    void printError(const char *msg){
        printf("Lexer error in file %s line %d at text %s\n%s\n",
            filename, lineNumber, yytext, msg);
    }

%}

%%
[a-zA-Z_][a-zA-Z0-9_]*  { printToken(306); }

[\n]     { lineNumber++; }
[ \t\r\v\f]+  { /* Ignore whitespace */ }
.           { printError("Unrecognized character"); }
%%

// Additional C code section


