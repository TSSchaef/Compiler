%{
    /* C definitions */
    #include <stdio.h>

    int yylex(void);
    void yyerror(const char *s);
%}

/* token definitions */

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
/* rules */
expr:
    INT          { $$ = $1; }
    ;

%%

/* user subroutines */

void yyerror(const char *s){
    fprintf(stderr, "Error: %s\n", s);
}
