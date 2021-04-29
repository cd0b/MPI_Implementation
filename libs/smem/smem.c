#include "smem.h"


int delete_smem(void* map, const char* name, int size) {
	int res = munmap(map, size);
	if(res == -1) {
		switch(errno) {
			case EINVAL:
				perror("Invalid addr!\n");
				break;
		}
		exit(-1);
	}

	return close_smem(name);
}


int close_smem(const char* name) {
	int smem_fd = shm_unlink(name);
	if(smem_fd == -1) {
		switch(errno) {
			case EACCES:
				perror("Permission to shm_unlink() the shared memory object was denied!\n");
				break;
			case ENOENT:
				perror("An  attempt was to made to shm_unlink() a name that does not exist!\n");
				break;
		}
		exit(-1);
	}
	return smem_fd;
}


void* open_smem(const char* name, int size) {

	int smem_fd = shm_open(name,
												O_RDWR, 
												0666);

	// errors.
	if(smem_fd == -1) {
		switch(errno) {
			case EACCES:
				perror("Process is not authorized to access this shared memory!\n");
				break;
			case EINVAL:
				perror("Shared memory name is invalid!\n");
				break;
			case ENAMETOOLONG:
				perror("The name of shared memory too long!\n");
				break;
			case ENFILE:
				perror("The system-wide limit on the total number of open files has been reached.\n");
				break;
		}
		exit(-1);
	}


	void* map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, smem_fd, 0);
	if(map == MAP_FAILED) {
		// error
		switch(errno) {
			case EACCES:
				perror("Access error on mmap!\n");
				break;
			case EAGAIN:
				perror("The file has been locked!\n");
				break;
			case EBADF:
				perror("The file descriptor is not valid!\n");
				break;
			case EINVAL:
				perror("The length of mmap is not valid!\n");
				break;
			case ENFILE:
				perror("The system-wide limit on the total number of open files has been reached!\n");
				break;
			case ENODEV:
				perror("The file system is not support mmap!\n");
				break;
			case ENOMEM:
				perror("No memory available!\n");
				break;
		}
		exit(-1);
	}

	return map;

}



void* creat_smem(const char* name, int size) {
	
	// create a shared memory region.
	int smem_fd = shm_open(name, 
											O_CREAT | O_RDWR, 
											0666);

	// errors.
	if(smem_fd == -1) {
		switch(errno) {
			case EACCES:
				perror("Process is not authorized to access this shared memory!\n");
				break;
			case EINVAL:
				perror("Shared memory name is invalid!\n");
				break;
			case ENAMETOOLONG:
				perror("The name of shared memory too long!\n");
				break;
			case ENFILE:
				perror("The system-wide limit on the total number of open files has been reached.\n");
				break;
		}
		exit(-1);
	}

	
	int ret = ftruncate(smem_fd, size);
	if(ret == -1) {
		switch (errno) {
			case EBADF:
				perror("Invalid file descriptor on truncate!\n");
				break;
			case EINVAL:
				perror("File descriptor is not open for write!\n");
				break;
		}
	}

	void* map = mmap(NULL, size, PROT_WRITE, MAP_SHARED, smem_fd, 0);
	if(map == MAP_FAILED) {
		// error
		switch(errno) {
			case EACCES:
				perror("Access error on mmap!\n");
				break;
			case EAGAIN:
				perror("The file has been locked!\n");
				break;
			case EBADF:
				perror("The file descriptor is not valid!\n");
				break;
			case EINVAL:
				perror("The length of mmap is not valid!\n");
				break;
			case ENFILE:
				perror("The system-wide limit on the total number of open files has been reached!\n");
				break;
			case ENODEV:
				perror("The file system is not support mmap!\n");
				break;
			case ENOMEM:
				perror("No memory available!\n");
				break;
		}
		exit(-1);
	}

	// return shared memory.
	return map;

}
