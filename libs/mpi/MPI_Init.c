#include "mpi.h"
#include "mem.h"
#include "smem.h"
#include "sem.h"
#include <signal.h>

typedef void (*sighandler_t)(int);

unsigned short rank = -1;
void* map;

void interrupt_handler(int signo) {
	MPI_Finalize();
}

int MPI_Init(int *argc, char*** argv) {

    signal(SIGINT, &interrupt_handler);

    // remove mpirun's parameters.
    *argv = (*argv) + 2;
    *argc = *argc - 2;

    // open shared memory.
    map = open_smem(DEFAULT_SMEM_NAME, DEFAULT_SMEM_SIZE);

    // get a rank
    rank = alloc_proc_rank(map);

    return 0;

}
