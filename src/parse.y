%define parse.error detailed

%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

char *yyfilename = NULL;

extern int yylineno;
extern char *yytext;

extern FILE *outputFile;

extern char *getOutputFileName();
extern char *getCurrentFileName();

int yylex(void);
void yyerror(const char *s);

void print_ident(const char *kind, char *name);

%}

%union {
    char *ident;
}

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
%token <ident> IDENT 306

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
%nonassoc LOWER_THAN_ELSE

/* additive / multiplicative */
%left '+' '-'
%left '*' '/' '%'

/* unary (use separate precedence name for unary minus) */
%right UMINUS

%%

C :  Var C
  | Struct_def C
  | Fun_def C
  | Fun_proto C
  |
  ;

type_with_struct : TYPE
                 | STRUCT IDENT 
                 ;

opt_const_type : CONST type_with_struct
               | type_with_struct CONST
               | type_with_struct
               ;

Var : opt_const_type IDENT {print_ident("global variable", $2);} opt_array opt_assignment opt_ident_list ';'
    ; 

opt_ident_list : 
           | ',' IDENT {print_ident("global variable", $2);} opt_array opt_assignment opt_ident_list;
           ;

Var_local : opt_const_type IDENT {print_ident("local variable", $2);} opt_array opt_assignment opt_ident_local_list ';'
    ; 

opt_ident_local_list : 
           | ',' IDENT {print_ident("local variable", $2);} opt_array opt_assignment opt_ident_local_list;
           ;



Struct_def : STRUCT IDENT {print_ident("global struct", $2);} '{' Struct_members '}' ';' 
           ;

Struct_local_def : STRUCT IDENT {print_ident("local struct", $2);} '{' Struct_members '}' ';' 
           ;

Struct_members : Struct_member Struct_members
               |
               ;

Struct_member : opt_const_type IDENT {print_ident("member", $2);} opt_array opt_member_list ';'
              ;

opt_member_list : 
           | ',' IDENT {print_ident("member", $2);} opt_array opt_member_list;
           ;

opt_array :
          | '[' INT ']' 
          ;

opt_assignment :
               | '=' expr 
               ;

Fun_dec : opt_const_type IDENT {print_ident("function", $2);} '(' opt_param_list ')' 
    ;

Fun_proto : Fun_dec ';' 
          ;

opt_empty_array :
          | '[' ']' 
          ;

opt_param_list :
                | opt_const_type IDENT {print_ident("parameter", $2);} opt_empty_array opt_param_list_tail
               ;

opt_param_list_tail :
                     | ',' opt_const_type IDENT {print_ident("parameter", $3);} opt_empty_array opt_param_list_tail
                    ;

Fun_def : Fun_dec '{' opt_fun_body '}' 
        ;

opt_fun_body :
             | Var_local opt_fun_body
             | Struct_local_def opt_fun_body
             | Stat opt_fun_body
             ;

Stat_block : '{' Stat_block_body '}'
           ;
           
Stat_block_body :
                | Stat Stat_block_body
                ;

Stat : matched_stmt
     | unmatched_stmt
     ;

unmatched_stmt : IF '(' expr ')' Stat
               | IF '(' expr ')' matched_stmt ELSE unmatched_stmt
               ;

matched_stmt : Stat_block
             | ';'
             | expr ';'
             | BREAK ';'
             | CONTINUE ';'
             | RETURN opt_expr ';' 
             | FOR '(' opt_expr ';' opt_expr ';' opt_expr ')' matched_stmt
             | WHILE '(' expr ')' matched_stmt
             | DO matched_stmt WHILE '(' expr ')' ';'
             | IF '(' expr ')' matched_stmt ELSE matched_stmt
             ;

opt_expr :
         | expr
         ;

expr : assignment_expression
     ;

//Assignment is right associative
assignment_expression : conditional_expression
    | lvalue '=' assignment_expression
    | lvalue PLUS_EQUAL assignment_expression
    | lvalue MINUS_EQUAL assignment_expression
    | lvalue TIMES_EQUAL assignment_expression
    | lvalue DIVIDE_EQUAL assignment_expression
    | lvalue MODULO_EQUAL assignment_expression
    ;

//Right associative ternary
conditional_expression : logical_or_expression
    | logical_or_expression '?' expr ':' conditional_expression
    ;

logical_or_expression : logical_and_expression
    | logical_or_expression OR_OR logical_and_expression
    ;

logical_and_expression : bitwise_or_expression
    | logical_and_expression AND_AND bitwise_or_expression
    ;

bitwise_or_expression : bitwise_xor_expression
    | bitwise_or_expression '|' bitwise_xor_expression
    ;

bitwise_xor_expression : bitwise_and_expression
    | bitwise_xor_expression '^' bitwise_and_expression
    ;

bitwise_and_expression
    : equality_expression
    | bitwise_and_expression '&' equality_expression
    ;

equality_expression : relational_expression
    | equality_expression EQUALITY relational_expression
    | equality_expression NOT_EQUAL relational_expression
    ;

relational_expression : additive_expression
    | relational_expression '<' additive_expression
    | relational_expression '>' additive_expression
    | relational_expression LT_EQUAL additive_expression
    | relational_expression GT_EQUAL additive_expression
    ;

additive_expression : multiplicative_expression
    | additive_expression '+' multiplicative_expression
    | additive_expression '-' multiplicative_expression
    ;

multiplicative_expression : unary_expression
    | multiplicative_expression '*' unary_expression
    | multiplicative_expression '/' unary_expression
    | multiplicative_expression '%' unary_expression
    ;

unary_expression : INCRDEC_PREFIX
    | '&' unary_expression
    | '*' unary_expression
    | '+' unary_expression
    | '-' unary_expression %prec UMINUS
    | '!' unary_expression
    | '~' unary_expression
    | '(' opt_const_type ')' unary_expression    // casting: (TYPE) expr  
    | postfix_expression
    ;

/* helper nonterminal for prefix ++/-- form */
INCRDEC_PREFIX
    : PLUS_PLUS lvalue
    | MINUS_MINUS lvalue
    ;

/* postfix_expression covers primary and function-call form IDENT(expr-list) and lvalue-postfix inc/dec */
postfix_expression
    : primary                      
    | primary '(' argument_expression_list_opt ')'   
    | lvalue_postfix PLUS_PLUS
    | lvalue_postfix MINUS_MINUS
    ;

primary
    : INT
    | FLOAT
    | STRING
    | CHAR
    | HEX
    | TRUE
    | FALSE
    | '(' expr ')'       /* parenthesized expression */
    | lvalue
    ;

lvalue : IDENT 
    | IDENT '[' expr ']' 
    | lvalue '.' IDENT
    | lvalue '.' IDENT '[' expr ']'
    ;

lvalue_postfix
    : lvalue
    ;

argument_expression_list_opt : 
    | argument_expression_list
    ;

argument_expression_list : expr
    | argument_expression_list ',' expr
    ;
%%

/* user C code */
void print_ident(const char *kind, char *name) {
    fprintf(outputFile, "File %s Line %d: %s %s\n", getCurrentFileName(), yylineno, kind, name);
    free(name);
}

void yyerror(const char *s) {
    fprintf(stderr, "Parser error in file %s line %d at text %s \n\t %s \n", getCurrentFileName(), yylineno, yytext, s);
    remove(getOutputFileName());
}
