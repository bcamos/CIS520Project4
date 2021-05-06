#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define ARRAY_SIZE 1000000
#define STRING_SIZE 2001
#define NUM_THREADS 32
pthread_mutex_t mutexsum;
float averages[ARRAY_SIZE];




char char_array[ARRAY_SIZE][STRING_SIZE];


float find_avg(char* line, int nchars) 
{
   int i, j;
   float sum = 0;

   for ( i = 0; i < nchars; i++ ) {
      sum += ((int) line[i]);
   }

   if (nchars > 0)
   {
       return sum / (float)nchars;
   }	
   else
   {
       return 0.0;
   }	
}

void *count_array(void *myId)
{
	char aChar;
    int i, j, localChar;
	float local_char_count[ARRAY_SIZE];
    int startPos = ((int)myId) * (ARRAY_SIZE / NUM_THREADS);
    int endPos = startPos + (ARRAY_SIZE / NUM_THREADS);
    

    printf("myID = %d startPos = %d endPos = %d \n", (int)myId, startPos, endPos);

	for (i = 0; i < ARRAY_SIZE; i++)
    {
        local_char_count[i] = 0.0;
    }

    for (i = startPos; i < endPos; i++)
    {
        local_char_count[i] = find_avg(char_array[i],strlen(char_array[i]));
    }

	pthread_mutex_lock(&mutexsum);
	
	for(i = 0; i < ARRAY_SIZE; i++){
		averages[i] += local_char_count[i];
	}
	
	pthread_mutex_unlock(&mutexsum);
	
	pthread_exit(NULL);
}




void init_arrays(){
	int i, j, err;
	
	FILE *fd;
	
	pthread_mutex_init(&mutexsum, NULL);
	
	fd = fopen("/homes/dan/625/wiki_dump.txt", "r");
	
	for (i = 0; i < ARRAY_SIZE; i++){
		err = fscanf(fd, "%[^\n]\n", char_array[i]);
           if (err == EOF)
           {
               break;
           }
	}
	
	for (i = 0; i < ARRAY_SIZE; i++){
		averages[i] = 0;
	}
}

void print_results(float every_line_avg[]){
	int i,j, total = 0;
	
	for( i = 0; i < ARRAY_SIZE; i++){
		printf("%d: %.1f\n", i, every_line_avg[i]);
	}
	
}



main(){
	
   
   int i, lineCount, rc;
   
   
	
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;
	void *status;


	/* Initialize and set thread detached attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	init_arrays();
	
	struct timeval v1, v2;

	double timeElapsed;
	
	gettimeofday(&v1, NULL);
	
	
	for(i = 0; i < NUM_THREADS; i++){
		rc = pthread_create(&threads[i], &attr, count_array, (void *)i);
		if(rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}
	
	pthread_attr_destroy(&attr);
	for(i = 0; i < NUM_THREADS; i++){
		rc = pthread_join(threads[i], &status);
		if(rc){
			printf("ERROR; return code from pthread_join() is %d\n", rc);
			exit(-1);
		}
	}
	
	print_results(averages);
	
	gettimeofday(&v2, NULL);
	
	timeElapsed = (v2.tv_sec - v1.tv_sec) * 1000.0; 
	timeElapsed += (v2.tv_usec - v1.tv_usec) / 1000.0;
	printf("DATA, pthread, %s, %s, %d, %f\n", getenv("SLURM_NTASKS"), getenv("SLURM_NNODES"), NUM_THREADS, timeElapsed);
	
	
	pthread_mutex_destroy(&mutexsum);
	pthread_exit(NULL);
}













