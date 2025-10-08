%{
    /* C definitions */
    #include <stdio.h>

    int yylex(void);
    void yyerror(const char *s);
%}

    /* token definitions */
%token NUMBER

%%
/* rules */
expr:
    NUMBER          { $$ = $1; }
    ;

%%

/* user subroutines */

void yyerror(const char *s){
    fprintf(stderr, "Error: %s\n", s);
}
