INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O3 $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= multiple_threads.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	multiple_threads

clean:
	-rm -f *.o *.d
	-rm -f perfmon multiple_threads

distclean:
	-rm -f *.o *.d
	-rm -f multiple_threads

multiple_threads: multiple_threads.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
