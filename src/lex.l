%option nodefault noinput nounput yylineno

%{
    // Definitions and includes
    #include <stdio.h>
    #include <libgen.h>

    #define STACK_SIZE 256
    #define FILE_SIZE 256


    typedef struct {
        char *filename;
        char *filepath;
        FILE *file;
        YY_BUFFER_STATE buffer;
        int lineNumber;
    } FileStack;

    FileStack fileStack[STACK_SIZE];
    int fileStackTop = 0;

    int pushFile(const char *filename) {
        if (fileStackTop >= STACK_SIZE) {
            fprintf(stderr, "File stack overflow\n");
            return -1;
        }

        FILE *file = fopen(filename, "r");
        if (!file) {
            fprintf(stderr, "Could not open file %s\n", filename);
            return -1;
        }

        fileStack[fileStackTop].filepath = strdup(filename);
        //basename to get the file name without path
        fileStack[fileStackTop].filename = basename(fileStack[fileStackTop].filepath);
        fileStack[fileStackTop].file = file;
        fileStack[fileStackTop].lineNumber = yylineno;
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

        if (start) {
            start++; // Move past the first quote
            const char *end = strchr(start, '"');
            if (end) {
                int len = end - start;
                strncpy(filename, start, len);
                filename[len] = '\0';
                printf("Found include filename: %s\n", filename);
            }
        }
        
        if (pushFile(filename) == -1){ 
            printf("Failed to include file %s\n", filename);
        } 
    }


    void printToken(int token){
        printf("File %s Line %d Token %d Text %s\n", 
            fileStack[fileStackTop - 1].filename, yylineno, token, yytext);
    }

    void printError(const char *msg){
        printf("Lexer error in file %s line %d at text %s\n%s\n",
            fileStack[fileStackTop - 1].filename, yylineno, yytext, msg);
    }

%}

%x COMMENT

%%
[\!%&()\*\+,\-\./:;<=>\?[\]{\|}~]  { printToken(yytext[0]); }

==                          { printToken(351);} 
!=                          { printToken(352);} 
>=                          { printToken(353);} 
"<"=                        { printToken(354);} 
\+\+                        { printToken(355);} 
--                          { printToken(356);} 
\|\|                        { printToken(357);} 
&&                          { printToken(358);} 

\+=                         { printToken(361);} 
-=                          { printToken(362);} 
\*=                         { printToken(363);} 
\/=                         { printToken(364);} 
->                          { printToken(365);} 
%=                          { printToken(366);} 

const                       { printToken(401); }
struct                      { printToken(402); }
for                         { printToken(403); }
while                       { printToken(404); }
do                          { printToken(405); }
if                          { printToken(406); }
else                        { printToken(407); }
break                       { printToken(408); }
continue                    { printToken(409); }
return                      { printToken(410); }
switch                      { printToken(411); }
case                        { printToken(412); }
default                     { printToken(413); }
true                        { printToken(414); }
false                       { printToken(415); }
bool                        { printToken(416); }

#[ \t]*include[ \t]*\"[^\"]+\"  { getFile(); }

int|float|char|void         { printToken(301); }
'\''(\\[ntrb\\'\"]|[^\\'])'\''   { printToken(302); }    
[0-9]+                      { printToken(303); }
[0-9]*\.[0-9]+              { printToken(304); }
\".*\"                      { printToken(305); }
[a-zA-Z_][a-zA-Z0-9_]*      { printToken(306); }

"//".*[\n|$(?!.)]             { }

"/*"  { BEGIN COMMENT; }
<COMMENT>[^*/]+ { /* ignore anything that is not '*' or '/' */ }
<COMMENT>("*"+)"/" { BEGIN INITIAL; }
<COMMENT>[*/] { /* residual stuff */ }

[\n]                        { }
[ \t\r\v\f]+                { /* Ignore whitespace */ }
.                           { printError("Unrecognized character"); }
%%

// Additional C code section

int yywrap() {
    if (popFile() != 0) {
        return 1; // No more files to process
    }

    if (!yyin) {
        return 1; // No more files to process
    }     

    return 0;
}

