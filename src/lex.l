%option nodefault noinput nounput yylineno

%{
    // Definitions and includes
    #include <stdio.h>
    #include <libgen.h>

    // For token definitions, so they're only in one place
    #include "parse.tab.h"

    #include "global.h"


    #define STACK_SIZE 512 
    #define FILE_SIZE 256
    #define MAX_FILE_NAME_SIZE 256

    #define MAX_INT_LEN     48
    #define MAX_REAL_LEN    48
    #define MAX_IDENT_LEN   48
    #define MAX_STRING_LEN  1024 

    char outputFileName[MAX_FILE_NAME_SIZE];
    FILE *outputFile;

    typedef struct {
        char *filename;
        char *filepath;
        FILE *file;
        YY_BUFFER_STATE buffer;
        int lineNumber;
    } FileStack;

    FileStack fileStack[STACK_SIZE];
    int fileStackTop = 0;

    void error(const char *msg){
        fprintf(stderr, "Lexer error in file %s line %d at text %s\n\t%s\n",
            fileStack[fileStackTop - 1].filename, yylineno, yytext, msg);
        remove(outputFileName);
        exit(1);
    }

    void getOutputFileName(const char *inputFile, char *outputFile) {
        const char *dot = strrchr(inputFile, '.');
        if (dot) {
            size_t len = dot - inputFile;
            strncpy(outputFile, inputFile, len);
            outputFile[len] = '\0';
            strcat(outputFile, ".lexer");
        } else {
            snprintf(outputFile, MAX_FILE_NAME_SIZE, "%s.lexer", inputFile);
        }
    }

    int pushFile(const char *filename) {
        if (fileStackTop >= STACK_SIZE) {
            error("File stack overflow\n");
            return -1;
        }

        FILE *file = fopen(filename, "r");
        if (!file) {
            // TODO : write error message with standard format ^^
            // fprintf(stderr, "Could not open file %s\n", filename);
            return -1;
        }

        fileStack[fileStackTop].filepath = strdup(filename);
        //basename to get the file name without path
        fileStack[fileStackTop].filename = basename(fileStack[fileStackTop].filepath);



        // If first file (all output to one file)
        if(fileStackTop == 0) {
            getOutputFileName(fileStack[fileStackTop].filename, outputFileName);

            //FILE *output = stdout;
            outputFile = fopen(outputFileName, "w");
            if(!outputFile) {
                // TODO : write error message with standard format ^^
                // fprintf(stderr, "Could not open output file %s\n", fileStack[fileStackTop].outputFileName);
                return -1;
            }
        }



        fileStack[fileStackTop].file = file;
        if(fileStackTop > 0) {
            fileStack[fileStackTop - 1].lineNumber = yylineno;
            yylineno = 1;
        }
        fileStack[fileStackTop].buffer = yy_create_buffer(file, YY_BUF_SIZE);
        yy_switch_to_buffer(fileStack[fileStackTop].buffer);

        fileStackTop++;
        return 0;
    }

    int popFile() {
        if (fileStackTop <= 0) {
            return -1;
        }

        fileStackTop--;
        fclose(fileStack[fileStackTop].file);
        free(fileStack[fileStackTop].filepath);

        yy_delete_buffer(fileStack[fileStackTop].buffer);

        if(fileStackTop != 0) {
            yylineno = fileStack[fileStackTop - 1].lineNumber;
            yy_switch_to_buffer(fileStack[fileStackTop - 1].buffer);
        } else {
            // yyterminate();
            return 1;
        }

        return 0;
    }

    void getFile() {
        char *start = strchr(yytext, '"');
        char filename[FILE_SIZE]; 

        // TODO: handle relative paths
        if (start) {
            start++; // Move past the first quote
            const char *end = strchr(start, '"');
            if (end) {
                int len = end - start;
                strncpy(filename, start, len);
                filename[len] = '\0';
                // TODO: Standardize message
                // printf("Found include filename: %s\n", filename);
            }
        }
        
        if (pushFile(filename) == -1){ 
            printf("Failed to include file %s\n", filename);
        } 
    }
    
    void printHex(int token){
        unsigned int i = 0;
        if(sscanf(yytext, "%x", &i) != 1){
            error("Invalid hex number");
            return;
        }

        if(mode != 2){
            // Only to .lexer file in mode 2 (lexer mode)
            return;
        }

        fprintf(outputFile, "File %s Line %d Token %d Text %d\n", 
            fileStack[fileStackTop - 1].filename, yylineno, token, i);
    }

    void printToken(int token){
        if(mode != 2){
            // Only to .lexer file in mode 2 (lexer mode)
            return;
        }

        fprintf(outputFile, "File %s Line %d Token %d Text %s\n", 
            fileStack[fileStackTop - 1].filename, yylineno, token, yytext);
    }


%}

%x COMMENT

%%
[\!%&()\*\+,\-\./:;<=>\?[\]{\|}~]  { printToken(yytext[0]); return yytext[0]; }

==                          { printToken(EQUALITY); return EQUALITY; }
!=                          { printToken(NOT_EQUAL); return NOT_EQUAL; }
>=                          { printToken(GT_EQUAL); return GT_EQUAL; }
"<"=                        { printToken(LT_EQUAL); return LT_EQUAL; }
\+\+                        { printToken(PLUS_PLUS); return PLUS_PLUS; }
--                          { printToken(MINUS_MINUS); return MINUS_MINUS; }
\|\|                        { printToken(OR_OR); return OR_OR; }
&&                          { printToken(AND_AND); return AND_AND; }

\+=                         { printToken(PLUS_EQUAL); return PLUS_EQUAL; }
-=                          { printToken(MINUS_EQUAL); return MINUS_EQUAL; }
\*=                         { printToken(TIMES_EQUAL); return TIMES_EQUAL; }
\/=                         { printToken(DIVIDE_EQUAL); return DIVIDE_EQUAL; }
->                          { printToken(ARROW); return ARROW; }
%=                          { printToken(MODULO_EQUAL); return MODULO_EQUAL;} 

const                       { printToken(CONST); return CONST; }
struct                      { printToken(STRUCT); return STRUCT; }
for                         { printToken(FOR); return FOR; }
while                       { printToken(WHILE); return WHILE; }
do                          { printToken(DO); return DO; }
if                          { printToken(IF); return IF; }
else                        { printToken(ELSE); return ELSE; }
break                       { printToken(BREAK); return BREAK; }
continue                    { printToken(CONTINUE); return CONTINUE; }
return                      { printToken(RETURN); return RETURN; }
switch                      { printToken(SWITCH); return SWITCH; }
case                        { printToken(CASE); return CASE; }
default                     { printToken(DEFAULT); return DEFAULT; }
true                        { printToken(TRUE); return TRUE; }
false                       { printToken(FALSE); return FALSE; }
bool                        { printToken(BOOL); return BOOL; }

#[ \t]*include[ \t]*\"[^\"]+\"  { getFile(); }

int|float|char|void         { printToken(TYPE); return TYPE; }
\'(\\[antrb\\'\"]|[^\\'])\' { printToken(CHAR); return CHAR; }
\'(\\[antrb\\'\"]|[^\\'])[^']+ { error("Unclosed character literal"); }

[0-9]+                      { if(yyleng > MAX_INT_LEN) {
                                error("Integer literal excedes max length");
                                return 0;
                            }
                            printToken(INT); 
                            return INT;
                            }

[0-9]*\.[0-9]+(e[+\-]?[0-9]+)?  {  if(yyleng > MAX_REAL_LEN) {
                                    error("Real number excedes max length");
                                    return 0;
                                }
                                printToken(FLOAT);
                                return FLOAT;
                                }

[0-9]*e[+\-]?[0-9]+         {  if(yyleng > MAX_REAL_LEN) {
                                    error("Real number excedes max length");
                                    return 0;
                                }
                                printToken(FLOAT);
                                return FLOAT;
                            }

\".*\"                      {  if(yyleng > MAX_STRING_LEN) {
                                    error("String length excedes max length");
                                    return 0;
                                }
                                printToken(STRING);
                                return STRING;
                            }

[a-zA-Z_][a-zA-Z0-9_]*      {  if(yyleng > MAX_IDENT_LEN) {
                                    error("Identifier length excedes max length");
                                    return 0;
                                }
                                printToken(IDENT);
                                return IDENT;
                            }

0[xX][0-9abcdefABCDEF]+     { printHex(HEX); return HEX; }
\^=?                        { printToken(BITWISE); return BITWISE; }


"//".*[\n|$(?!.)]           { /* Ignore single line comments */}

"/*"                        { BEGIN COMMENT; }
<COMMENT>[^*/]+             { /* ignore anything that is not '*' or '/' */ }
<COMMENT>("*"+)"/"          { BEGIN INITIAL; }
<COMMENT>[*/]               { /* residual stuff */ }

[ \n\t\r\v\f]+              { /* Ignore whitespace */ }
.                           { error("Unrecognized character"); }
%%

// Additional C code section

int yywrap() {
    if (popFile() != 0) {
        fclose(outputFile);
        return 1; // No more files to process
    }

    if (!yyin) {
        fclose(outputFile);
        return 1; // No more files to process
    }     

    return 0;
}

