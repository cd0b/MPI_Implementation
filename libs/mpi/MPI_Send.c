#include "mpi.h"
#include "mem.h"
#include "sem.h"
#include <stdio.h>


int MPI_Send(const void* buf, int count, int datatype, int dest, int tag) {

    int ret = memwrite(map, (unsigned short)dest, (unsigned short)rank, buf, count*datatype, tag);
    if(ret != 0) return -1;

    // wake up recv to read data
    char sem_name[DEFAULT_SEMAPHORE_NAME_LENGTH];
    gsn_receive(sem_name, dest);
    sem_t* sem_recv = osem(sem_name);
    usem(sem_recv);

    // wait for recv to finish
    gsn_send(sem_name, rank);
    sem_t* sem_send = osem(sem_name);
    dsem(sem_send);

    return 0;

}