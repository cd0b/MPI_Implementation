export LIBRARIES := -lmpi -lmem -lrt -lpthread

CURABSPATH := $(shell pwd)

export CC := gcc
export AR := ar
export INCLUDES := -I$(CURABSPATH)/include -I$(CURABSPATH)/libs/smem/include -I$(CURABSPATH)/mpirun/include -I$(CURABSPATH)/libs/mem/include -I$(CURABSPATH)/libs/sem/include -I$(CURABSPATH)/libs/mpi/include
export LFLAGS := -L$(CURABSPATH)/libs

DIRS := libs mpirun

.PHONY: dirs $(DIRS)


dirs: $(DIRS)

$(DIRS):
	$(MAKE) -C $@

mpirun: libs




TYPES_TO_CLEAN := *.a *.o mpirun hello test *.so *.swp *.s
DIRS_TO_CLEAN := libs/mpi libs/smem libs/mem libs/sem mpirun libs
FILES_TO_CLEAN := $(foreach DIR,$(DIRS_TO_CLEAN),$(addprefix $(DIR)/,$(TYPES_TO_CLEAN)))


.PHONY: clean

clean:
	rm -f $(FILES_TO_CLEAN)

