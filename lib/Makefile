CC = gcc
RM = rm -f

# defining any directories containing header files other than /usr/include
INCLUDES = -I../include

BUILDDIR = 
CFLAGS = $(INCLUDES) $(BUILDDIR) -Wall
AR = ar

SRC = err.c std.c parse.c transmission.c authentication.c sync.c notification.c
# Automatically create the Object file list
OBJS = ${SRC:.c=.o}

LIB = ./libsupport.a
.PHONY: clean

# $? : only the files that have changed
$(LIB) : $(OBJS)
	${AR} rvs ${LIB} $?

clean :
	$(RM) *.o $(LIB)

# Building Library dependencies
depend: $(SRCS)
	makedepend -- $(CFLAGS) -- $^

# DO NOT DELETE THIS LINE -- make depend needs it
