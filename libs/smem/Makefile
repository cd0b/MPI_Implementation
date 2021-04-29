SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
MAIN := smem

CFLAGS := -c -O3 -Wall

.PHONY: all

all: $(OBJS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
