#include "mpi.h"
#include "mem.h"
#include "sem.h"
#include <stdio.h>


int MPI_Recv(void* buf, int count, int datatype, int source, int tag) {

    // wait for send
    char sem_name[DEFAULT_SEMAPHORE_NAME_LENGTH];
    gsn_receive(sem_name, rank);
    sem_t* sem_recv = osem(sem_name);
    dsem(sem_recv);

    int ret = memread(map, (unsigned short)rank, (unsigned short)source, buf, count*datatype, tag);
    if(ret != 0) return -1;

    // wake up sender to finish send
    gsn_send(sem_name, source);
    sem_t* sem_send = osem(sem_name);
    usem(sem_send);

    return 0;

}