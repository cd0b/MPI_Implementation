LIBS := -lmem -lrt -lpthread
SRCS := $(wildcard *.c)
MAIN := mpirun
CFLAGS := -O3 -Wall

.PHONY: all

all: $(MAIN)

$(MAIN):
	$(CC) $(SRCS) -o $@ $(LFLAGS) $(INCLUDES) $(CFLAGS) $(LIBS)
