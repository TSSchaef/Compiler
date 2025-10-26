%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *yyfilename = NULL;

extern int yylineno;
extern char *yytext;

//extern char *outputFileName;
char *tempOutputFile = "outputFile.parse";

int yylex(void);
void yyerror(const char *s);

void print_parser(const char *kind, const char *ident);

%}

/* --- Token numbers to match the lexer --- */
%token EQUALITY     351
%token NOT_EQUAL    352
%token GT_EQUAL     353
%token LT_EQUAL     354
%token PLUS_PLUS    355
%token MINUS_MINUS  356
%token OR_OR        357
%token AND_AND      358

%token PLUS_EQUAL   361
%token MINUS_EQUAL  362
%token TIMES_EQUAL  363
%token DIVIDE_EQUAL 364
%token ARROW        365
%token MODULO_EQUAL 366

%token CONST        401
%token STRUCT       402
%token FOR          403
%token WHILE        404
%token DO           405
%token IF           406
%token ELSE         407
%token BREAK        408
%token CONTINUE     409
%token RETURN       410
%token SWITCH       411
%token CASE         412
%token DEFAULT      413
%token TRUE         414
%token FALSE        415
%token BOOL         416

%token TYPE         301
%token CHAR         302
%token INT          303
%token FLOAT        304
%token STRING       305
%token IDENT        306

%token HEX          307
%token BITWISE      308


%%

C :  Var C
  | Fun_def C
  |
  ;

Var : TYPE IDENT opt_array opt_ident_list ';' {print_parser("global variable", "ident");}
    ; 

opt_ident_list : 
           | ',' IDENT opt_array opt_ident_list;
           ;

opt_array :
          | '[' INT ']' 
          ;

Fun_dec : TYPE IDENT '(' opt_param_list ')' {print_parser("function ", "ident");}
    ;

opt_empty_array :
          | '[' ']' 
          ;

opt_param_list :
                | TYPE IDENT opt_empty_array opt_param_list_tail
               ;

opt_param_list_tail :
                     | ',' TYPE IDENT opt_empty_array opt_param_list_tail
                    ;

Fun_def : Fun_dec '{' opt_fun_body '}'
        ;

opt_fun_body :
            | Var opt_fun_body
            | Stat opt_fun_body
             ;

Stat : ';'
     | RETURN opt_expr ';' 
     //| IF '(' expr ')' Stat opt_else
     | WHILE '(' expr ')' Stat
     | DO Stat WHILE '(' expr ')' ';'
     | FOR '(' opt_expr ';' opt_expr ';' opt_expr ')' Stat
     ;

opt_else :
          | ELSE Stat
          ;

opt_expr :
          | expr
          ;

expr : IDENT
     | INT
     | FLOAT
     | STRING
     | TRUE
     | FALSE
     ;

%%

/* user C code */
void print_parser(const char *kind, const char *ident) {
    printf("File %s Line %d: %s %s\n", tempOutputFile, yylineno, kind, ident);
}

void yyerror(const char *s) {
    fprintf(stderr, "Parser error in file %s line %d at text %s\n", s, yylineno, yytext);
}
