#ifndef STACK_H
#define STACK_H

#include <stdio.h>

extern char *outputFileName;
extern FILE *outputFile;

typedef struct FileStack FileStack;

extern FileStack *fileStack;
extern int fileStackTop;

int pushFile(const char *filename);

#endif
