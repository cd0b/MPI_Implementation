#include "mpi.h"
#include "mem.h"


int MPI_Comm_size(int* size) {

    *size = get_proc_count(map);
    return 0;

}