###############################################
# Makefile for compiling the program skeleton
# 'make' build executable file 'PROJ'
# 'make all' build project
# 'make clean' removes all .o, executable
###############################################
PROJ = sokoban # the name of the project
CC = gcc # name of compiler
# define any compile-time flags
CFLAGS = -std=c99 -Wall -O3 -Wuninitialized -Wunreachable-code -pedantic # there is a space at the end of this
LFLAGS = -lm
###############################################
# You don't need to edit anything below this line
###############################################
# list of object files
# The following includes all of them!
C_FILES := $(wildcard *.c)
OBJS := $(patsubst %.c, %.o, $(C_FILES))
# To create the executable file we need the individual
# object files
$(PROJ): queue.o sokoban.c
	$(CC) $(CFLAGS) $(LFLAGS) -o $(PROJ) sokoban.c queue.o

# To create each individual object file we need to
# compile these files using the following general
# purpose macro
.c.o:
	$(CC) $(CFLAGS) -c $<
# there is a TAB for each identation.
# To make all (program + manual) "make all"

all :
	make

debug: queue.o sokoban.c
	$(CC) $(CFLAGS) -DDEBUG $(LFLAGS) -o $(PROJ) sokoban.c queue.o
	
queue.o: queue.c queue.h
	$(CC) $(LFLAGS) $(CFLAGS) -c queue.c

clean:
	rm -rf *.o sokoban
