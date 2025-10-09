%option nodefault noinput nounput yylineno

%{
    // Definitions and includes
    #include <stdio.h>
    #include <libgen.h>

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

        fprintf(outputFile, "File %s Line %d Token %d Text %d\n", 
            fileStack[fileStackTop - 1].filename, yylineno, token, i);
    }

    void printToken(int token){
        fprintf(outputFile, "File %s Line %d Token %d Text %s\n", 
            fileStack[fileStackTop - 1].filename, yylineno, token, yytext);
    }


%}

%x COMMENT

%%
[\!%&()\*\+,\-\./:;<=>\?[\]{\|}~]  { printToken(yytext[0]); return yytext[0]; }

==                          { printToken(351); return 351; }
!=                          { printToken(352); return 352; }
>=                          { printToken(353); return 353; }
"<"=                        { printToken(354); return 354; }
\+\+                        { printToken(355); return 355; }
--                          { printToken(356); return 356; }
\|\|                        { printToken(357); return 357; }
&&                          { printToken(358); return 358; }

\+=                         { printToken(361); return 361; }
-=                          { printToken(362); return 362; }
\*=                         { printToken(363); return 363; }
\/=                         { printToken(364); return 364; }
->                          { printToken(365); return 365; }
%=                          { printToken(366); return 366;} 

const                       { printToken(401); return 401; }
struct                      { printToken(402); return 402; }
for                         { printToken(403); return 403; }
while                       { printToken(404); return 404; }
do                          { printToken(405); return 405; }
if                          { printToken(406); return 406; }
else                        { printToken(407); return 407; }
break                       { printToken(408); return 408; }
continue                    { printToken(409); return 409; }
return                      { printToken(410); return 410; }
switch                      { printToken(411); return 411; }
case                        { printToken(412); return 412; }
default                     { printToken(413); return 413; }
true                        { printToken(414); return 414; }
false                       { printToken(415); return 415; }
bool                        { printToken(416); return 416; }

#[ \t]*include[ \t]*\"[^\"]+\"  { getFile(); }

int|float|char|void         { printToken(301); return 301; }
\'(\\[antrb\\'\"]|[^\\'])\' { printToken(302); return 302; }
\'(\\[antrb\\'\"]|[^\\'])[^']+ { error("Unclosed character literal"); }

[0-9]+                      { if(yyleng > MAX_INT_LEN) {
                                error("Integer literal excedes max length");
                                return 0;
                            }
                            printToken(303); 
                            return 303;
                            }

[0-9]*\.[0-9]+(e[+\-]?[0-9]+)?  {  if(yyleng > MAX_REAL_LEN) {
                                    error("Real number excedes max length");
                                    return 0;
                                }
                                printToken(304);
                                return 304;
                                }

[0-9]*e[+\-]?[0-9]+         {  if(yyleng > MAX_REAL_LEN) {
                                    error("Real number excedes max length");
                                    return 0;
                                }
                                printToken(304);
                                return 304;
                            }

\".*\"                      {  if(yyleng > MAX_STRING_LEN) {
                                    error("String length excedes max length");
                                    return 0;
                                }
                                printToken(305);
                                return 305;
                            }

[a-zA-Z_][a-zA-Z0-9_]*      {  if(yyleng > MAX_IDENT_LEN) {
                                    error("Identifier length excedes max length");
                                    return 0;
                                }
                                printToken(306);
                                return 306;
                            }

0[xX][0-9abcdefABCDEF]+     { printHex(307); return 307; }
\^=?                        { printToken(308); return 308; }


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

