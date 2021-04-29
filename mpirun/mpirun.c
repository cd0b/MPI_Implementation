#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "smem.h"
#include "mem.h"
#include "sem.h"
#include <signal.h>

typedef void (*sighandler_t)(int);

static unsigned short process_count = 0;



void clean_semaphores_at_exit(void) {

	printf("%d\n", process_count);
	// clean
	// delete semaphores

	rsem("zero_region");
	rsem("pd_region");
	rsem("ipt_region");

	for(unsigned short rank = 0; rank < process_count; ++rank) {
		char sem_name[DEFAULT_SEMAPHORE_NAME_LENGTH];
		gsn_pt(sem_name, rank);
		rsem_silent(sem_name);

		gsn_receive(sem_name, rank);
		rsem_silent(sem_name);

		gsn_send(sem_name, rank);
		rsem_silent(sem_name);
	}

}

void int_handler(int signo) {
	clean_semaphores_at_exit();
}



int main(int argc, char** argv) {

	signal(SIGINT, &int_handler);
	atexit(&clean_semaphores_at_exit);

	csem("zero_region", 1);
	csem("ipt_region", 1); // invalid page table region
												// end of the memory
	csem("pd_region", 1); // page directory region

	// argv[1] -> how many process to create.
	// argv[2] -> which program will execute in processes created.
	
	process_count = atoi(argv[1]);
	
	for(unsigned short rank = 0; rank < process_count; ++rank) {
		// create semaphores
		char sem_name[DEFAULT_SEMAPHORE_NAME_LENGTH];
		gsn_pt(sem_name, rank);
		csem(sem_name, 1); // when accessing page table of this process

		gsn_receive(sem_name, rank);
		csem(sem_name, 0);             // after a send process, wait receive.

		gsn_send(sem_name, rank);
		csem(sem_name, 0);             // before a recv, wait send.
	}
	
	void* map = creat_smem(DEFAULT_SMEM_NAME, DEFAULT_SMEM_SIZE);
	init_mem(map, process_count);
	if(process_count <= 0) {
		// no process to create
		printf("mpirun successfully terminated!\n");
		return 0;
	}
	
	int i = process_count;
	while(i > 0) {
		pid_t parent_pid = fork();
		if(!parent_pid) {
			// child process.
			execv(argv[2], argv);
		}
		--i;
	}

	i = process_count;
	while(i > 0) {
		int status;
		int child_pid = wait(&status);
		char child_term_message[100];
		sprintf(child_term_message, "child with pid %d is terminated! Exit status: %d\n", child_pid, status);
		perror(child_term_message);
		--i;
	}

	// clean
	delete_smem(map, DEFAULT_SMEM_NAME, DEFAULT_SMEM_SIZE);

	printf("\n\nmpirun successfully terminated!\n");
	return 0;

}
