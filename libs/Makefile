export CC
export INCLUDES
export LFLAGS

LIBNAME := mem
DIRS := smem mem sem mpi
OBJS := $(foreach DIR,$(DIRS), $(addprefix $(DIR)/,*.o))

.PHONY: $(DIRS)

lib$(LIBNAME).a: $(DIRS)
	$(AR) rcs $@ $(OBJS)

$(DIRS):
	$(MAKE) -C $@
