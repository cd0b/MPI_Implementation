#ifndef _MPI_H
#define _MPI_H

extern unsigned short rank;
extern void* map;
extern volatile unsigned int finalize;

int MPI_Init(int*, char***);
int MPI_Finalize();
int MPI_Comm_size(int *);
int MPI_Comm_rank(int *);
int MPI_Recv(void* buf, int count, int datatype, int source, int tag);
int MPI_Send(const void* buf, int count, int datatype, int dest, int tag);

#endif