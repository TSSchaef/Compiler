tschaef Compiler
===================

## Usage 
All source code is within the source folder. Calling "make" will use the Makefile to create the executable "mycc". This can be used with this format:

mycc -mode infile

mode: integer (1-5)

infile: filepath to code for compilation 

"make clean" can be used to remove all object, binary, and dependency files generated.

Modes 1 & 2 are currently implemented. Mode 1 does NOT require an infile. It will simply print the version information for the compiler.

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

