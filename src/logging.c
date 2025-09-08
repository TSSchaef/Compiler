#include "logging.h"
#include <stdio.h>

void logUsage(){
    fprintf(stderr, "\n\n Usage: \n mycc -mode infile \n \nmode: integer 1-5 \ninfile: path to file to compile (Not used for mode 1)\n");
}

void logCompilerInfo(){
    fprintf(stderr, "My very own C compiler written for COM S 4400, Fall 2025. \n Written by Tyler Schaefer (tschaef@iastate.edu) \n Version 0.0.1-SNAPSHOT, released 12 September, 2025\n");
}

void logNotSupported(){
    fprintf(stderr, "Those features aren't supported yet, see documentation for supported features.\n");
}

void logImproperInput(){
    fprintf(stderr, "Improper input format.");
    logUsage();
}

int handleInputs(char *argv[], int argc){
    //No flags/arguments
    int mode;
    if(argc <= 1){
        fprintf(stderr, "No mode specified.\n");
        return -1;
    } 

    sscanf(argv[1], "-%d", &mode);

    if(argc == 2){
        //Check mode is 1 else error
        if(mode == 1){
            return 1;
        } else {
            fprintf(stderr, "Mode %d requires an infile.\n", mode);
            return -1;
        }
    }

    return mode;
}

