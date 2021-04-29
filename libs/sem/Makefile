SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
MAIN := sem

CFLAGS := -c -O3 -Wall

.PHONY: all

all: $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
