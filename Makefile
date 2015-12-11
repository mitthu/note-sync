# This file was created by 'mitthu'
RM = rm -f
CC = gcc
OPTIMIZATION = 
CFLAGS = $(OPTIMIZATION) $(INCLUDES)
EXE = prg

# Make library
TEMP := $(shell cd lib; make)

# defining any directories containing header files other than /usr/include
INCLUDES = -I./include

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS = -L./lib

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname 
#   option, something like (this will link in libmylib.so and libm.so:
LIBS = -lsupport -lpthread

all:
	$(CC) $(CFLAGS) server.c -o server $(LFLAGS) $(LIBS)
	$(CC) $(CFLAGS) client.c -o client $(LFLAGS) $(LIBS)

.PHONY: clean
clean :
	$(RM) *.o $(EXE) *.gch lib/*.gch server client watch
	cd lib && $(MAKE) clean
	