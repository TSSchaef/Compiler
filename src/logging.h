#ifndef LOGGING
#define LOGGING

#include <stdio.h>

int handleInputs(char *argv[], int argc);

void logUsage();
void logCompilerInfo();
void logNotSupported();
void logImproperInput();

#endif
