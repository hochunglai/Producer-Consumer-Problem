#ifndef  HEADER_H
#define  HEADER_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFFER_SIZE 128
#define NUM_BUFF 100

struct buffer {
	char string[BUFFER_SIZE];
	int count;

};


static int set_semvalue(int sem, int val)
{
	union semun sem_union;
	sem_union.val = val;
	if (semctl(sem, 0, SETVAL, sem_union) == -1) return(0);
	return(1);
}


static void del_semvalue(int sem)
{
	union semun sem_union;
	if (semctl(sem, 0, IPC_RMID, sem_union) == -1)
		fprintf(stderr, "Failed to delete semaphore\n");
}

//Wait
static int semaphore_p(int sem)
{

	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = -1; /* P() */
	sem_b.sem_flg = 0;
	if (semop(sem, &sem_b, 1) == -1) {
		fprintf(stderr, "semaphore_p failed\n");
		return(0);
	}
	return(1);
}


//Signal
static int semaphore_v(int sem)
{

	struct sembuf sem_b;
	sem_b.sem_num = 0;
	sem_b.sem_op = 1; /* V() */
	sem_b.sem_flg = 0;
	if (semop(sem, &sem_b, 1) == -1) {
		fprintf(stderr, "semaphore_v failed\n");
		return(0);
	}
	return(1);
}

#endif




