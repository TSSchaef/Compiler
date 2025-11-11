%define parse.error detailed

%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "global.h"
#include "ast.h"
#include "symtab.h"
#include "typecheck.h"

extern int yylineno;
extern char *yytext;

extern FILE *outputFile;

extern char *getOutputFileName();
extern char *getCurrentFileName();

int yylex(void);
void yyerror(const char *s);

void print_ident(const char *kind, char *name);

struct Type *type_int(void);
struct Type *type_char(void);
struct Type *type_float(void);
struct Type *type_void(void);

/* helper that maps the TYPE token string to a Type*. */
static struct Type *type_from_name(const char *name) {
    if (!name) return NULL;
    if (strcmp(name, "int") == 0) return type_int();
    if (strcmp(name, "char") == 0) return type_char();
    if (strcmp(name, "float") == 0) return type_float();
    if (strcmp(name, "void") == 0) return type_void();
    return NULL;
}

struct Type *curr_type;

%}

%union {
    struct AST *ast;

    int intval;
    float floatval;
    bool boolval;
    char charval;
    char *strval;

    char *ident;
    struct Type *type;
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

%token <boolval>    BOOL    416

%token <ident>      TYPE    301
%token <charval>    CHAR    302
%token <intval>     INT     303
%token <floatval>   FLOAT   304
%token <strval>     STRING  305
%token <ident>      IDENT   306

%token <intval>     HEX     307

%token BITWISE      308


%type <ast> Program C Var Var_local Struct_def Struct_local_def Struct_members Struct_member Fun_dec Fun_proto Fun_def Stat_block Stat_block_body Stat unmatched_stmt matched_stmt expr assignment_expression conditional_expression logical_or_expression logical_and_expression bitwise_or_expression bitwise_xor_expression bitwise_and_expression equality_expression relational_expression additive_expression multiplicative_expression unary_expression postfix_expression primary lvalue lvalue_postfix argument_expression_list_opt argument_expression_list opt_assignment opt_ident_list opt_ident_local_list opt_param_list opt_param_list_tail opt_fun_body opt_expr INCRDEC_PREFIX

%type <type> opt_const_type type_with_struct
%type <boolval> opt_array opt_empty_array

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

Program : C {
                root_ast = ast_block_from_list($1);
                $$ = root_ast;
            } 
        ;

C :  Var C          { $$ = ast_list_prepend($1, $2); }
    | Struct_def C  { $$ = ast_list_prepend($1, $2); }
    | Fun_def C     { $$ = ast_list_prepend($1, $2); }
    | Fun_proto C   { $$ = ast_list_prepend($1, $2); }
    |               {$$ = NULL;}
  ;

type_with_struct : TYPE             { $$ = type_from_name($1); free($1); }
                 | STRUCT IDENT 
                 ;


opt_const_type : CONST type_with_struct     { $$ = $2; }
               | type_with_struct CONST     { $$ = $1; }
               | type_with_struct           { $$ = $1; }
               ;


Var : opt_const_type IDENT  { print_ident("global variable", $2);} opt_array opt_assignment opt_ident_list ';' 

            {
                if($4){
                    $$ = ast_set_line_no(ast_decl($2, type_array($1), $5),
                                                    yylineno);
                    curr_type = $1;
                } else {
                    $$ = ast_set_line_no(ast_decl($2, $1, $5), yylineno);
                    curr_type = $1;
                }
            };


opt_ident_list :  {$$ = NULL; }
           | ',' IDENT  { print_ident("global variable", $2); } opt_array opt_assignment opt_ident_list
            {
                if($4){
                    $$ = ast_set_line_no(ast_decl($2, type_array(curr_type)
                                                    , $5), yylineno);
                } else {
                    $$ = ast_set_line_no(ast_decl($2, curr_type, $5), yylineno);
                }
            };



Var_local : opt_const_type IDENT { print_ident("local variable", $2); } opt_array opt_assignment opt_ident_local_list ';'
            {
                if($4){
                    $$ = ast_set_line_no(ast_decl($2, type_array($1), $5),
                                                    yylineno);
                    curr_type = $1;
                } else {
                    $$ = ast_set_line_no(ast_decl($2, $1, $5), yylineno);
                    curr_type = $1;
                }
            };



opt_ident_local_list :  {$$ = NULL; }
           | ',' IDENT  {
                            print_ident("local variable", $2);
                        } 
        opt_array opt_assignment opt_ident_local_list
            {
                if($4){
                    $$ = ast_set_line_no(ast_decl($2, type_array(curr_type)
                                                    , $5), yylineno);
                } else {
                    $$ = ast_set_line_no(ast_decl($2, curr_type, $5), yylineno);
                }
            };



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


opt_array : { $$ = false; }
          | '[' INT ']'  { $$ = true; }
          ;




opt_assignment :            { $$ = NULL; }
               | '=' expr   { $$ = $2; }
               ;



Fun_dec : opt_const_type IDENT {print_ident("function", $2);} '(' opt_param_list ')'    {
            $$ = ast_set_line_no(ast_func($2, $1, $5, NULL), yylineno);
         };

Fun_proto : Fun_dec ';' { $$ = $1; }
          ;

opt_empty_array :   { $$ = false; }
        | '[' ']'  { $$ = true; }
        ;


opt_param_list : {$$ = NULL;}
                | opt_const_type IDENT {print_ident("parameter", $2);} opt_empty_array opt_param_list_tail
            {
                if($4){
                    $$ = ast_set_line_no(ast_list_prepend(ast_decl($2, 
                                    type_array($1), NULL), $5), yylineno);
                } else {
                    $$ = ast_set_line_no(ast_list_prepend(ast_decl($2, $1, 
                                            NULL), $5), yylineno);
                }
            };

opt_param_list_tail : {$$ = NULL;}
                     | ',' opt_const_type IDENT {print_ident("parameter", $3);} opt_empty_array opt_param_list_tail
            {
                if($5){
                    $$ = ast_set_line_no(ast_list_prepend(ast_decl($3, 
                                    type_array($2), NULL), $6), yylineno);
                } else
                    $$ = ast_set_line_no(ast_list_prepend(ast_decl($3, $2, 
                                            NULL), $6), yylineno);
            };


Fun_def : Fun_dec '{' opt_fun_body '}'  { $1->func.body = ast_block_from_list($3); $$ = $1; }
        ;



opt_fun_body :                                 { $$ = NULL; }
            | Var_local opt_fun_body           { $$ = ast_list_prepend($1, $2); }
            | Struct_local_def opt_fun_body    { $$ = ast_list_prepend($1, $2); }
            | Stat opt_fun_body                { $$ = ast_list_prepend($1, $2); }
            ;



Stat_block : '{' Stat_block_body '}' { $$ = ast_block_from_list($2); }
           ;


Stat_block_body : {$$ = NULL; }
                | Stat Stat_block_body  { $$ = ast_list_prepend($1, $2); }
                ;


Stat : matched_stmt     { $$ = $1; }
     | unmatched_stmt   { $$ = $1; }
     ;


unmatched_stmt : IF '(' expr ')' Stat       { $$ = ast_set_line_no(ast_if($3, $5, NULL), yylineno); }
               | IF '(' expr ')' matched_stmt ELSE unmatched_stmt
                    { $$ = ast_set_line_no(ast_if($3, $5, $7), yylineno); }
               ;


matched_stmt : Stat_block   { $$ = $1; }
             | ';'          { $$ = NULL; }
             | expr ';'     { $$ = $1; }
             | BREAK ';'    { $$ = ast_set_line_no(ast_break(), yylineno); }
             | CONTINUE ';' { $$ = ast_set_line_no(ast_continue(), yylineno); }
             | RETURN opt_expr ';' { $$ = ast_set_line_no(ast_return($2), yylineno); }

             | FOR '(' opt_expr ';' opt_expr ';' opt_expr ')' matched_stmt
                    { $$ = ast_set_line_no(ast_for($3, $5, $7, $9), yylineno); }

             | WHILE '(' expr ')' matched_stmt
                    { $$ = ast_set_line_no(ast_while($3, $5), yylineno); }

             | DO matched_stmt WHILE '(' expr ')' ';'
                    { $$ = ast_set_line_no(ast_do_while($2, $5), yylineno); }

             | IF '(' expr ')' matched_stmt ELSE matched_stmt
                    { $$ = ast_set_line_no(ast_if($3, $5, $7), yylineno); }
             ;


opt_expr : { $$ = NULL; }
         | expr { $$ = $1; }
         ;


expr : assignment_expression { $$ = $1; }
     ;



//Assignment is right associative
assignment_expression : conditional_expression  { $$ = $1; }
    | lvalue '=' assignment_expression          
            { $$ = ast_set_line_no(ast_assign(AOP_ASSIGN, $1, $3), yylineno); }

    | lvalue PLUS_EQUAL assignment_expression
            { $$ = ast_set_line_no(ast_assign(AOP_ADD_ASSIGN, $1, $3), yylineno); }

    | lvalue MINUS_EQUAL assignment_expression
            { $$ = ast_set_line_no(ast_assign(AOP_SUB_ASSIGN, $1, $3), yylineno); }

    | lvalue TIMES_EQUAL assignment_expression
            { $$ = ast_set_line_no(ast_assign(AOP_MUL_ASSIGN, $1, $3), yylineno); }

    | lvalue DIVIDE_EQUAL assignment_expression
            { $$ = ast_set_line_no(ast_assign(AOP_DIV_ASSIGN, $1, $3), yylineno); }

    | lvalue MODULO_EQUAL assignment_expression
            { $$ = ast_set_line_no(ast_assign(AOP_MOD_ASSIGN, $1, $3), yylineno); }
    ;


//Right associative ternary
conditional_expression : logical_or_expression { $$ = $1; }
    | logical_or_expression '?' expr ':' conditional_expression
            { $$ = ast_set_line_no(ast_ternary($1, $3, $5), yylineno); }
    ;


logical_or_expression : logical_and_expression  { $$ = $1; }
    | logical_or_expression OR_OR logical_and_expression
        {$$ = ast_set_line_no(ast_logical_or($1, $3), yylineno); }
    ;


logical_and_expression : bitwise_or_expression  { $$ = $1; }
    | logical_and_expression AND_AND bitwise_or_expression
        {$$ = ast_set_line_no(ast_logical_and($1, $3), yylineno); }
    ;


bitwise_or_expression : bitwise_xor_expression { $$ = $1; }
    | bitwise_or_expression '|' bitwise_xor_expression
        {$$ = ast_set_line_no(ast_binop(OP_BIT_OR, $1, $3), yylineno); }
    ;


bitwise_xor_expression : bitwise_and_expression { $$ = $1; }
    | bitwise_xor_expression '^' bitwise_and_expression
        {$$ = ast_set_line_no(ast_binop(OP_BIT_XOR, $1, $3), yylineno); }
    ;


bitwise_and_expression
    : equality_expression   { $$ = $1; }
    | bitwise_and_expression '&' equality_expression
        {$$ = ast_set_line_no(ast_binop(OP_BIT_AND, $1, $3), yylineno); }
    ;


equality_expression : relational_expression { $$ = $1; }
    | equality_expression EQUALITY relational_expression
         {$$ = ast_set_line_no(ast_binop(OP_EQ, $1, $3), yylineno); }
    | equality_expression NOT_EQUAL relational_expression
         {$$ = ast_set_line_no(ast_binop(OP_NEQ, $1, $3), yylineno); }
    ;


relational_expression : additive_expression { $$ = $1; }
    | relational_expression '<' additive_expression
            {$$ = ast_set_line_no(ast_binop(OP_LT, $1, $3), yylineno); }
    | relational_expression '>' additive_expression
            {$$ = ast_set_line_no(ast_binop(OP_GT, $1, $3), yylineno); }
    | relational_expression LT_EQUAL additive_expression
            {$$ = ast_set_line_no(ast_binop(OP_LE, $1, $3), yylineno); }
    | relational_expression GT_EQUAL additive_expression
            {$$ = ast_set_line_no(ast_binop(OP_GE, $1, $3), yylineno); }
    ;


additive_expression : multiplicative_expression { $$ = $1; }
    | additive_expression '+' multiplicative_expression
            {$$ = ast_set_line_no(ast_binop(OP_ADD, $1, $3), yylineno); }
    | additive_expression '-' multiplicative_expression
            {$$ = ast_set_line_no(ast_binop(OP_SUB, $1, $3), yylineno); }
    ;


multiplicative_expression : unary_expression    { $$ = $1; }
    | multiplicative_expression '*' unary_expression
        {$$ = ast_set_line_no(ast_binop(OP_MUL, $1, $3), yylineno); }
    | multiplicative_expression '/' unary_expression
        {$$ = ast_set_line_no(ast_binop(OP_DIV, $1, $3), yylineno); }
    | multiplicative_expression '%' unary_expression
        {$$ = ast_set_line_no(ast_binop(OP_MOD, $1, $3), yylineno); }
    ;


unary_expression : INCRDEC_PREFIX   { $$ = $1; }
    | '&' unary_expression
            {$$ = ast_set_line_no(ast_unary(UOP_ADDR, $2), yylineno); }
    | '*' unary_expression
            {$$ = ast_set_line_no(ast_unary(UOP_DEREF, $2), yylineno); }
    | '+' unary_expression
            {$$ = ast_set_line_no(ast_unary(UOP_PLUS, $2), yylineno); }
    | '-' unary_expression %prec UMINUS
            {$$ = ast_set_line_no(ast_unary(UOP_NEG, $2), yylineno); }
    | '!' unary_expression
            {$$ = ast_set_line_no(ast_unary(UOP_LOGICAL_NOT, $2), yylineno); }
    | '~' unary_expression
            {$$ = ast_set_line_no(ast_unary(UOP_BITWISE_NOT, $2), yylineno); }
    | '(' opt_const_type ')' unary_expression    // casting: (TYPE) expr  
            {$$ = ast_set_line_no(ast_cast($2, $4), yylineno); }

    | postfix_expression    { $$ = $1; }
    ;


/* helper nonterminal for prefix ++/-- form */
INCRDEC_PREFIX
    : PLUS_PLUS lvalue      
        { $$ = ast_set_line_no(ast_unary(UOP_PRE_INC, $2), yylineno); }
    | MINUS_MINUS lvalue
        { $$ = ast_set_line_no(ast_unary(UOP_PRE_DEC, $2), yylineno); }
    ;


/* postfix_expression covers primary and function-call form IDENT(expr-list) and lvalue-postfix inc/dec */
postfix_expression
    : primary           { $$ = $1; }
    | primary '(' argument_expression_list_opt ')'   
        { $$ = ast_set_line_no(ast_func_call($1, $3), yylineno); }
    | lvalue_postfix PLUS_PLUS
        { $$ = ast_set_line_no(ast_unary(UOP_POST_INC, $1), yylineno); }
    | lvalue_postfix MINUS_MINUS
        { $$ = ast_set_line_no(ast_unary(UOP_POST_DEC, $1), yylineno); }
    ;


primary
    : INT           { $$ = ast_set_line_no(ast_int($1), yylineno);}
    | FLOAT         { $$ = ast_set_line_no(ast_float($1), yylineno); }
    | STRING        { $$ = ast_set_line_no(ast_string($1), yylineno); }
    | CHAR          { $$ = ast_set_line_no(ast_char($1), yylineno); }
    | HEX           { $$ = ast_set_line_no(ast_int($1), yylineno); }
    | BOOL          { $$ = ast_set_line_no(ast_bool($1), yylineno); }
    | TRUE          { $$ = ast_set_line_no(ast_bool(true), yylineno); }
    | FALSE         { $$ = ast_set_line_no(ast_bool(false), yylineno); }
    | '(' expr ')'  { $$ = $2; }
    | lvalue        { $$ = $1; }
    ;


lvalue : IDENT                      
        { $$ = ast_set_line_no(ast_id($1), yylineno); }

    | IDENT '[' expr ']'            
        { 
            $$ = ast_array_access(ast_set_line_no(ast_id($1), yylineno), $3);
        }

    | lvalue '.' IDENT              
        { $$ = ast_set_line_no(ast_id($3), yylineno); }

    | lvalue '.' IDENT '[' expr ']' 
        { 
            $$ = ast_array_access(ast_set_line_no(ast_id($3), yylineno), $5);
        }
    ;


lvalue_postfix
    : lvalue    { $$ = $1; }
    ;


argument_expression_list_opt : { $$ = NULL; }
    | argument_expression_list  { $$ = $1; }
    ;


argument_expression_list : expr { $$ = $1; }
    | argument_expression_list ',' expr { $$ = ast_set_line_no(ast_list_append($3, $1), yylineno); }
    ;

%%



/* user C code */
void print_ident(const char *kind, char *name) {
    // Only print parsing information in mode 3
    if(mode == 3){
        fprintf(outputFile, "File %s Line %d: %s %s\n", getCurrentFileName(), yylineno, kind, name);
    }
}

void yyerror(const char *s) {
    fprintf(stderr, "Parser error in file %s line %d at text %s \n\t %s \n", getCurrentFileName(), yylineno, yytext, s);
    remove(getOutputFileName());
}
