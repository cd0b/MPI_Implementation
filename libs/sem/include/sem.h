#ifndef _SEM_H
#define _SEM_H

#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>


#define DEFAULT_SEMAPHORE_NAME_LENGTH 32

sem_t* csem(const char*, int); // create semaphore
sem_t* osem(const char*); // open semaphore
void dsem(sem_t*); // down semaphore
void usem(sem_t*); // up semaphore
void rsem(const char*); // remove semaphore
void rsem_silent(const char* name); // on error do nothing



/*
    string functions 
    gsn -> generate semaphore name
*/


/*
    generate semaphore name for page_table_x
*/
void gsn_pt(char*, unsigned short);

/*
    generate semaphore name for sender_x
*/
void gsn_send(char*, unsigned short);

/*
    generate semaphore name for receiver_x
*/
void gsn_receive(char*, unsigned short);



#endif
