#include "mpi.h"
#include "sem.h"
#include "mem.h"

int MPI_Finalize() {
    
    free_proc(map, rank);

    // clear semaphores
    char sem_name[DEFAULT_SEMAPHORE_NAME_LENGTH];
    gsn_pt(sem_name, rank);
    rsem(sem_name);
    gsn_receive(sem_name, rank);
    rsem(sem_name);
    gsn_send(sem_name, rank);
    rsem(sem_name);

    return 0;
}