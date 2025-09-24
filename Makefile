#Makefile
BINARY=mycc
DEV-BINARY=mycc-dev
CODEDIRS=src
INCDIRS=src ext 

CC=gcc
LEX=flex

#use DEV options for development
#PROD for production
DEV-OPT=-O0
PROD-OPT=-O3
LIBFLAGS= -lfl
#-lm -lpthread
DEPFLAGS=-MP -MD
DEV-CFLAGS=-Wall -Werror -g $(foreach D, $(INCDIRS), -I$(D)) $(DEV-OPT) $(DEPFLAGS)
PROD-CFLAGS=$(foreach D, $(INCDIRS), -I$(D)) $(PROD-OPT) $(DEPFLAGS)

CFILES=	$(foreach D,$(CODEDIRS), $(wildcard $(D)/*.c))
OBJECTS= $(patsubst %.c, %.o, $(CFILES)) 

LEXFILES= $(foreach D,$(CODEDIRS), $(wildcard $(D)/*.l))
LEX_OUTPUT = $(CODEDIRS)/lex.yy.o

DEV-OBJECTS= $(patsubst %.c, %.dev.o, $(CFILES)) 
DEPFILES= $(patsubst %.c, %.d, $(CFILES)) 
DEV-DEPFILES= $(patsubst %.c, %.dev.d, $(CFILES)) 

# Default used for production
all: $(BINARY)

$(BINARY): $(LEX_OUTPUT) $(OBJECTS) 
	$(CC) -o $@ $^ $(LIBFLAGS)

$(LEX_OUTPUT): $(LEXFILES)
	$(LEX) -o $(CODEDIRS)/lex.yy.c $<
	$(CC) $(PROD-CFLAGS) -c -o $@ $(CODEDIRS)/lex.yy.c

%.o: %.c
	$(CC) $(PROD-CFLAGS) -c -o $@ $<



# Used for development builds, has debugging enabled
dev: $(DEV-BINARY)

$(DEV-BINARY): $(DEV-OBJECTS)
	$(CC) -o $@ $^ $(LIBFLAGS)

%.dev.o: %.c
	$(CC) $(DEV-CFLAGS) -c -o $@ $<




.PHONY: clean

clean:
	@rm -f $(LEX_OUTPUT) $(OBJECTS) $(DEPFILES) $(BINARY) $(DEV-OBJECTS) $(DEV-DEPFILES) $(DEV-BINARY) perf.data* *.lexer

diff:
	$(info The status of the repository, and the volume of per-file changes:)
	@git status
	@git diff --stat

-include $(DEPFILES)
