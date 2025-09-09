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
[\!%&()\*\+,\-\./:;<=>\?[\]{\|}~]  { printToken(yytext[0]); }

==                          { printToken(351);} 
!=                          { printToken(352);} 
>=                          { printToken(353);} 
[<]=                        { printToken(354);} 
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

int|float|char|void         { printToken(301); }
'.'                         { printToken(302); }    
[0-9]+                      { printToken(303); }
[0-9]*\.[0-9]+              { printToken(304); }
\".*\"                        { printToken(305); }
[a-zA-Z_][a-zA-Z0-9_]*      { printToken(306); }

[\n]                        { lineNumber++; }
\\.*[\n|$(?!.)]             { lineNumber++; }
\/\*[.|\n]*\*\/                  { /* Ignore comments */ }

[ \t\r\v\f]+                { /* Ignore whitespace */ }
.                           { printError("Unrecognized character"); }
%%

// Additional C code section


