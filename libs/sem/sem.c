#include "sem.h"

/*
	csem -> create semaphore
	with flags O_CREAT and O_EXCL
	if there is semaphore with this name
	exit(-1)
*/
sem_t* csem(const char* name, int val) {

	sem_t* semptr = sem_open(name, O_CREAT | O_EXCL, 0666, val);

	if(semptr == SEM_FAILED) {
		switch(errno) {
			case EACCES:
			case EEXIST:
				perror("Semaphore is exist!\n");
				break;
			case EINVAL:
				perror("Invalid name or semaphore max value reached!\n");
				break;
			case EMFILE:
				perror("Semaphore can not created, fd max count reached!\n");
				break;
			case ENAMETOOLONG:
				perror("Long name for semaphore!\n");
				break;
			case ENFILE:
				perror("Semaphore can not created, Linux reached max open file limit!\n");
				break;
			case ENOENT:
				perror("Bad formatted semaphore name!\n");
				break;
			case ENOMEM:
				perror("Insufficient memory to create semaphore!\n");
		}
		exit(-1);
	}

	return semptr;

}


/*
	osem -> open semaphore
	with flag O_RDWR
	if error occurs exit(-1)
*/
sem_t* osem(const char* name) {

	sem_t* semptr = sem_open(name, O_RDWR, 0666, 1);

	if(semptr == SEM_FAILED) {
		switch(errno) {
			case EACCES:
				perror("Semaphore is exist but can not open in this process!\n");
				break;
			case EINVAL:
				perror("Invalid name or semaphore max value reached!\n");
				break;
			case EMFILE:
				perror("Semaphore can not created, fd max count reached!\n");
				break;
			case ENAMETOOLONG:
				perror("Long name for semaphore!\n");
				break;
			case ENFILE:
				perror("Semaphore can not created, Linux reached max open file limit!\n");
				break;
			case ENOENT:
				perror("No semaphore found in this name!\n");
				break;
			case ENOMEM:
				perror("Insufficient memory to create semaphore!\n");
		}
		exit(-1);
	}

	return semptr;

}


/*
	downsem
	if error exit(-1)
*/
void dsem(sem_t* semptr) {
	int ret = sem_wait(semptr);

	if(ret != 0) {
		switch(errno) {
			case EINTR:
				perror("The semaphore wait call is interrupted!\n");	
				break;
			case EINVAL:
				perror("Semaphore is not valid!\n");
				break;
		}
		exit(-1);
	}
}



/*
	upsem
	if error exit(-1)
*/
void usem(sem_t* semptr) {
	int ret = sem_post(semptr);

	if(ret != 0) {
		switch(errno) {
			case EINVAL:
				perror("Semapgore is not valid!\n");
				break;
			case EOVERFLOW:
				perror("The max value for semaphore would be exceeded!\n");
				break;
		}
		exit(-1);
	}
}



/*
	remove semaphore
	if error exit(-1)
*/
void rsem(const char* name) {

	int ret = sem_unlink(name);

	if(ret != 0) {
		switch (errno) {
			case EACCES:
				perror("This process can not unlink this semaphore!\n");
				break;
			case ENAMETOOLONG:
				perror("Semaphore name is too long!\n");
				break;
			case ENOENT:
				perror("No semaphore with this name!\n");
				break;
		}
		exit(-1);
	}

} 


void rsem_silent(const char* name) {
	sem_unlink(name);
}




/*
	gsn_pt
	generate_sem_name_for_page_table
*/
void gsn_pt(char* buf, unsigned short rank) {
	sprintf(buf, "page_table_%hu", rank);
}


/*
	gsn_send
	generate_sem_name_for_sender_process
*/
void gsn_send(char* buf, unsigned short rank) {
	sprintf(buf, "sender_%hu", rank);
}

/*
	gsn_receive
	generate_sem_name_for_receiver_process
*/
void gsn_receive(char* buf, unsigned short rank) {
	sprintf(buf, "receiver_%hu", rank);
}