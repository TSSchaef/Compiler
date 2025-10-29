tschaef Compiler
===================

## Dependencies

- Flex is used to for the lexer to convert the regular expressions into a DFA in C. It can be installed on UNIX with:

    `sudo apt-get install flex`

- Bison is used for the parser to convert the context free grammar into a Look-Ahead LR(1) parser. It can be installed on UNIX with:
    
    `sudo apt-get install bison`


## Usage 
All source code is within the source folder. Calling 
    
    `make` 

will use the Makefile to create the executable "mycc". This can be used with this format:

    `./mycc -mode infile`

mode: integer (1-5)
infile: filepath to code for compilation 


To remove all object, binary, and dependency files generated use: 

    `make clean`


## Modes

Modes 1-3 are currently implemented. Mode 1 does NOT require an infile. It will simply print the version information for the compiler.

Mode 2 requires an infile and will perform lexical analysis on it, outputting the token stream to a .lexer file. Supported features for the lexer include:

 * Keywords, types, identifiers
 * Integer, real, string, and character literals 
    * (with escape characters)
 * Comments
 * Symbols
 * Real literals with exponents
 * Hexadecimal numbers
 * Boolean operations (^, ^=)
 * Errors for very long lexemes
 * Errors for invalid characters
 * .lexer file removed on error
 * Recursive #include operations

Mode 3 requires an infile and will perform lexical analysis and parsing on it, outputting the global/local variables, function declarations, and struct declarations to a .parser file. Supported features for the parser include:

 * Global variables
 * Function declarations / parameter lists
 * Function local variables and body
 * For, while, do loops
 * if then else
 * break / continue / return / expression statements
 * Expressions with unary/binary/ternary operators
 * Assignment operators; increment and decrement
 * Identifiers and arrays
 * Function calls and parameters
 * Function prototypes 
 * Variable initializations 
 * Constants 
 * User-defined structs 
 * Struct member selection 
