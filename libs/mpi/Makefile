SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
MAIN := mpi

CFLAGS := -c -O3 -fpic -Wall
LDFLAGS := -shared

lib$(MAIN).so: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)
	mv $@ ../$@

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
