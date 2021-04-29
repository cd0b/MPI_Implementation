SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
MAIN := mem

CFLAGS := -c -O3 -Wall

.PHONY: all

all: $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
