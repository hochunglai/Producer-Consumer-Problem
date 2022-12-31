#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/sem.h>
#include <sys/shm.h>
#include <inttypes.h>

#include "semun.h"
#include "header.h"

static int set_semvalue(int sem, int val);
static void del_semvalue(int sem);
static int semaphore_p(int sem);
static int semaphore_v(int sem);
char *take();

static int sem_s;
static int sem_n;
static int sem_e;

static char w[128];

static int btyes_sum=0;
int btyes;
int start=1;

struct buffer *shared_stuff;
struct buffer *out;


int main()
{	void *shared_memory = (void *)0;
	int shmid;
	
	
	//Opening the output file
	int filedesc = open("output_file.txt", O_WRONLY | O_APPEND);
	if(filedesc == -1){
		printf("Error Opening File");
		exit(EXIT_FAILURE);
	}

	srand((unsigned int)getpid());
	
	//Obtaining the shared memory
	shmid = shmget((key_t)8911, sizeof(struct buffer)*NUM_BUFF, 0666 | IPC_CREAT);
	
	
	if(shmid ==-1){
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}

	//Obtaining Semaphore S, N, E
	sem_s = semget((key_t)1234, 1, 0666 | IPC_CREAT);
	sem_n = semget((key_t)4321, 1, 0666 | IPC_CREAT);
	sem_e = semget((key_t)9999, 1, 0666 | IPC_CREAT);
	
	shared_memory = shmat(shmid, (void *)0, 0);
	if(shared_memory == (void *)-1){
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}
	
	printf("Memory attached at %lX\n", (uintptr_t)shared_memory);


	shared_stuff = (struct buffer *)shared_memory;
	out = (struct buffer *)shared_memory;


	printf("Reading from buffer starts\n");
	printf("====================================================================================\n\n\n");

	//Consume
	while(start){
		if(!semaphore_p(sem_n))exit(EXIT_FAILURE);
		if(!semaphore_p(sem_s)) exit(EXIT_FAILURE);
		char *wr = take();
		if(!semaphore_v(sem_s)) exit(EXIT_FAILURE);
		if(!semaphore_v(sem_e)) exit(EXIT_FAILURE);
		
		//Writing to output file
		write(filedesc, wr, btyes);
		
		printf("%s", w);
	}
	
	printf("\n\n\n====================================================================================\n");
	printf("Reading from buffer ends\n");
	
	
	//Print out total btyes read
	printf("Total Btyes Read - %d\n", btyes_sum);


	//Delete and Detach Shared Memory
	if (shmdt(shared_memory) == -1) {
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}
	
	if (shmctl(shmid, IPC_RMID, 0) == -1) {
		fprintf(stderr, "shmctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}

	//Delete all semaphores
	del_semvalue(sem_s);
	del_semvalue(sem_n);
	del_semvalue(sem_e);
	

	exit(EXIT_SUCCESS);
	
}

char *take()
{
	static int count=0;
	memset(w, 0, 128);
	
	//Copy string from buffer
	memcpy(w, out->string, sizeof(out->string));
	
	//Sum up btyes read
	btyes_sum = btyes_sum + out->count;
	
	btyes = out->count;

	//If btyes is < 128, finish reading
	if(btyes<128){
		start=0;
	}
	
	//Increment to next element in buffer
	out++;
	count++;
	
	//Loop around the buffer
	if(count==100){
		out = shared_stuff;
		count =0;
	}
	return w;
}

