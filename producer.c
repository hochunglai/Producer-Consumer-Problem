#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/sem.h>
#include <sys/shm.h>
#include <inttypes.h>
#include <math.h>

#include "semun.h"
#include "header.h"


static int set_semvalue(int sem, int val);
static void del_semvalue(int sem);
static int semaphore_p(int sem);
static int semaphore_v(int sem);
void append(char* v, int btyes);


static int sem_s;
static int sem_n;
static int sem_e;

static int sum;

struct buffer *shared_stuff;
struct buffer *in;

int main()
{	void *shared_memory = (void *)0;
	char input_buffer[BUFSIZ+1];
	char test[128];

	int shmid;
	int cycles;
	int btyes;
	
	//Open the file to be read from
	int filedesc = open("input_file.txt", O_RDONLY);
	if(filedesc == -1){
		printf("Error Opening File");
		exit(EXIT_FAILURE);
	}
	
	//Allocating shared memory
	shmid = shmget((key_t)8911, sizeof(struct buffer)*NUM_BUFF, 0666 | IPC_CREAT);
	
	if(shmid ==-1){
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}

	//Initializing Semaphore S, N, E
	sem_s = semget((key_t)1234, 1, 0666 | IPC_CREAT);
	sem_n = semget((key_t)4321, 1, 0666 | IPC_CREAT);
	sem_e = semget((key_t)9999, 1, 0666 | IPC_CREAT);
	
	if(!set_semvalue(sem_n, 0)){
		fprintf(stderr, "Failed to initialize sem_n\n");
		exit(EXIT_FAILURE);
	}

	if(!set_semvalue(sem_s, 1)){
		fprintf(stderr, "Failed to initialize sem_s\n");
		exit(EXIT_FAILURE);
	}

	if(!set_semvalue(sem_e, NUM_BUFF)){
		fprintf(stderr, "Failed to initialize sem_e\n");
		exit(EXIT_FAILURE);
	}

	shared_memory = shmat(shmid, (void *)0, 0);
	if(shared_memory == (void *)-1){
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}
	
	printf("Memory attached at %lX\n", (uintptr_t)shared_memory);

	shared_stuff = (struct buffer *)shared_memory;
	in = (struct buffer *)shared_memory;


	//Reading from input file to input buffer
	int nread = read(filedesc, &input_buffer, BUFSIZ);
	if(nread == -1){
		printf("Error Reading");
		exit(EXIT_FAILURE);
	}
	
	printf("Wrtting to buffer starts\n");
	printf("====================================================================================\n\n\n");
	
	//Dividing text stored in input buffer to smaller segments
	while(nread != 0){
		if(nread%128 ==0){
			cycles = nread/128;
		}else{
			cycles = (nread/128)+1;
		}
		for(int i=0; i < cycles; i++){
			memset(test, 0, sizeof test);
			if(i!= (nread/128) || nread/128 == 64){
				for(int j=0; j<=127; j++){
					test[j] = input_buffer[(i*(128))+j];
					printf("%c", test[j]);	
				}
				btyes = 128;
			}else{
				for(int j=0; j<= 127; j++){ //(nread - (nread/128)*128)
					test[j] = input_buffer[(i*(128))+j];
					printf("%c", test[j]);	
				}
				btyes = (nread - ((nread/128))*128);
			}
			
			if(!semaphore_p(sem_e)) exit(EXIT_FAILURE);
	 		if(!semaphore_p(sem_s)) exit(EXIT_FAILURE);
	 		append(test, btyes);
	 		if(!semaphore_v(sem_s)) exit(EXIT_FAILURE);
	 		if(!semaphore_v(sem_n)) exit(EXIT_FAILURE);	
	 		memset(test, 0, sizeof test);
		}
		memset(input_buffer, 0, sizeof input_buffer);
		nread = read(filedesc, &input_buffer, BUFSIZ);
		if(nread == -1){
			printf("Error Reading");
			exit(EXIT_FAILURE);
		}
	}
	
	printf("\n\n\n====================================================================================\n");
	printf("Wrtting to buffer ends\n");
	
	//Print total btyes written
	printf("Total Btyes Written - %d\n", sum);
	
}


void append(char *v, int btyes)
{
	static int count=0;
	memcpy(in->string, v, 128);
	in->count = btyes;
	
	//Sum up btypes written
	sum = sum + btyes;
	
	//Increment to next element in buffer
	in++;
	count++;
	
	//Loop around the buffer
	if(count==100){
		count =0;
		in = shared_stuff;
	}
}


