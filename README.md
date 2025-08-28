tschaef Compiler
===================

## Usage 
All source code is within the source folder. Calling "make" will use the Makefile to create the executable "mycc". This can be used with this format:

mycc -mode infile

mode: integer (1-5)
infile: filepath to code for compilation 

Only mode 1 is currently implemented which does NOT require an infile. Mode 1 will simply print the version information for the compiler.

"make clean" can be used to remove all object, binary, and dependency files generated.
