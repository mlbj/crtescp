INCLUDE_DIRS = 
LIB_DIRS = 
CC = gcc

CDEFS = -D_GNU_SOURCE  # add the GNU source definition
CFLAGS = -O3 $(INCLUDE_DIRS) $(CDEFS)
LIBS = -lpthread       # ensure the pthread library is explicitly linked

HFILES = 
CFILES = sched_fifo.c

SRCS = ${HFILES} ${CFILES}
OBJS = ${CFILES:.c=.o}

all: sched_fifo

clean:
	-rm -f *.o *.d
	-rm -f perfmon sched_fifo

distclean: clean

sched_fifo: sched_fifo.o
	$(CC) $(CFLAGS) -o $@ sched_fifo.o $(LIBS)  # Link with -lpthread

depend:

.c.o:
	$(CC) $(CFLAGS) -c $<
