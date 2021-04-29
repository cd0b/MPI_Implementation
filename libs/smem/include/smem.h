#ifndef _SMEM_H
#define _SMEM_H

#define DEFAULT_SMEM_SIZE 40000000
#define DEFAULT_SMEM_NAME	"MPI_SMEM" 

#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>



void* creat_smem(const char*, int);
void* open_smem(const char*, int);
int close_smem(const char*);
int delete_smem(void*, const char*, int);

#endif
