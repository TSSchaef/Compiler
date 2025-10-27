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

void print_ident(const char *kind);

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



/* Precedence/associativity (list from lowest precedence to highest) */
%left ','

/* assignment operators are right-associative */
%right '=' PLUS_EQUAL MINUS_EQUAL TIMES_EQUAL DIVIDE_EQUAL MODULO_EQUAL

/* logical / bitwise */
%left OR_OR
%left AND_AND
%left '|'
%left '^'
%left '&'

/* equality / relational */
%nonassoc EQUALITY NOT_EQUAL
%nonassoc '<' '>' LT_EQUAL GT_EQUAL

/* additive / multiplicative */
%left '+' '-'
%left '*' '/' '%'

/* unary (use separate precedence name for unary minus) */
%right UMINUS

%%

C :  Var C
  | Fun_def C
  |
  ;

Var : TYPE IDENT {print_ident("global variable");} opt_array opt_ident_list ';'
    ; 

opt_ident_list : 
           | ',' IDENT {print_ident("global variable");} opt_array opt_ident_list;
           ;

opt_array :
          | '[' INT ']' 
          ;

Fun_dec : TYPE IDENT {print_ident("function");} '(' opt_param_list ')' 
    ;

opt_empty_array :
          | '[' ']' 
          ;

opt_param_list :
                | TYPE IDENT {print_ident("parameter");}opt_empty_array opt_param_list_tail
               ;

opt_param_list_tail :
                     | ',' TYPE IDENT {print_ident("parameter");} opt_empty_array opt_param_list_tail
                    ;

Fun_def : Fun_dec '{' opt_fun_body '}' 
        ;

opt_fun_body :
            | Var opt_fun_body
            | Stat opt_fun_body
             ;

Stat : ';'
     | expr ';'
     | BREAK ';'
     | CONTINUE ';'
     | RETURN opt_expr ';' 
     | IF '(' expr ')' Stat_or_stat_block //opt_else
     | FOR '(' opt_expr ';' opt_expr ';' opt_expr ')' Stat_or_stat_block 
     | WHILE '(' expr ')' Stat_or_stat_block
     | DO Stat_or_stat_block WHILE '(' expr ')' ';'
     ;

Stat_or_stat_block : Stat
                 | Stat_block
                 ;

Stat_block : '{' Stat_block_body '}'
           ;
           
Stat_block_body :
            | Stat Stat_block_body
            ;

opt_else :
         | ELSE Stat_or_stat_block
         ;

opt_expr :
         | expr
         ;

opt_expr_list: 
         | expr
         | expr ',' expr_list
         ;

expr_list : expr
          | expr ',' expr_list
          ;

expr : assignment_expression
     ;

/* Assignment: includes all allowed assignment operators, right-associative */
assignment_expression
    : conditional_expression
    | lvalue '=' assignment_expression
    | lvalue PLUS_EQUAL assignment_expression
    | lvalue MINUS_EQUAL assignment_expression
    | lvalue TIMES_EQUAL assignment_expression
    | lvalue DIVIDE_EQUAL assignment_expression
    | lvalue MODULO_EQUAL assignment_expression
    ;

/* Conditional (ternary) - right associative in the grammar by using conditional_expression on the RHS */
conditional_expression
    : logical_or_expression
    | logical_or_expression '?' expr ':' conditional_expression
    ;

/* Logical / bitwise precedence chain */
logical_or_expression
    : logical_and_expression
    | logical_or_expression OR_OR logical_and_expression
    ;

logical_and_expression
    : bitwise_or_expression
    | logical_and_expression AND_AND bitwise_or_expression
    ;

bitwise_or_expression
    : bitwise_xor_expression
    | bitwise_or_expression '|' bitwise_xor_expression
    ;

bitwise_xor_expression
    : bitwise_and_expression
    | bitwise_xor_expression '^' bitwise_and_expression
    ;

bitwise_and_expression
    : equality_expression
    | bitwise_and_expression '&' equality_expression
    ;

/* equality / relational */
equality_expression
    : relational_expression
    | equality_expression EQUALITY relational_expression
    | equality_expression NOT_EQUAL relational_expression
    ;

relational_expression
    : additive_expression
    | relational_expression '<' additive_expression
    | relational_expression '>' additive_expression
    | relational_expression LT_EQUAL additive_expression
    | relational_expression GT_EQUAL additive_expression
    ;

/* additive / multiplicative */
additive_expression
    : multiplicative_expression
    | additive_expression '+' multiplicative_expression
    | additive_expression '-' multiplicative_expression
    ;

multiplicative_expression
    : unary_expression
    | multiplicative_expression '*' unary_expression
    | multiplicative_expression '/' unary_expression
    | multiplicative_expression '%' unary_expression
    ;

/* Unary expressions: prefix ++/-- on l-value handled here as pre-increment, also unary ops and casts.
   Note: postfix ++/-- are handled via lvalue_postfix below (so we can match "lvalue inc_dec_op" and "inc_dec_op lvalue"). */
unary_expression
    : INCRDEC_PREFIX
    | '&' unary_expression
    | '*' unary_expression
    | '+' unary_expression
    | '-' unary_expression %prec UMINUS
    | '!' unary_expression
    | '~' unary_expression
    | '(' TYPE ')' unary_expression    /* cast: (TYPE) expr  (uses TYPE token distinct from IDENT) */
    | postfix_expression
    ;

/* helper nonterminal for prefix ++/-- form that must be followed by an lvalue */
INCRDEC_PREFIX
    : PLUS_PLUS lvalue
    | MINUS_MINUS lvalue
    ;

/* postfix_expression covers primary and function-call form IDENT(expr-list) and lvalue-postfix inc/dec */
postfix_expression
    : primary                      /* includes function-call form when primary is IDENT followed by (...) handled below */
    | primary '(' argument_expression_list_opt ')'   /* IDENT(...) permitted here because primary can be IDENT */
    | lvalue_postfix PLUS_PLUS
    | lvalue_postfix MINUS_MINUS
    ;

/* primary expressions: literal, IDENT, parenthesized expr */
primary
    : INT
    | FLOAT
    | STRING
    | TRUE
    | FALSE
    | '(' expr ')'       /* parenthesized expression */
    | IDENT              /* note: IDENT alone is a valid primary as well as the start of a call or lvalue */
    ;

/* lvalue: exactly as specified — an identifier optionally followed by one or more bracketed expressions.
   This allows IDENT and IDENT[expr] and IDENT[expr][expr] ... */
lvalue
    : IDENT {print_ident("local variable");}
    | lvalue {print_ident("local variable");} '[' expr ']' 
    ;

/* lvalue_postfix is used to allow postfix ++/-- on an lvalue and to be used where a postfix lvalue is needed */
lvalue_postfix
    : lvalue
    ;

/* argument list for function calls: zero or more expressions separated by commas */
argument_expression_list_opt
    : /* empty */
    | argument_expression_list
    ;

argument_expression_list
    : expr
    | argument_expression_list ',' expr
    ;
%%

/* user C code */
void print_ident(const char *kind) {
    printf("File %s Line %d: %s %s\n", tempOutputFile, yylineno, kind, yytext);
}

void yyerror(const char *s) {
    fprintf(stderr, "Parser error in file %s line %d at text %s\n", s, yylineno, yytext);
}
