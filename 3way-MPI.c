#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#define MAX_SIZE 1000000
#define MAX_STRING 2001
char lines[MAX_SIZE][MAX_STRING];
char* local_lines;
float* global_averages;
float* local_averages;
int NUM_THREADS;
int BATCH_SIZE;

float find_avg(char* line, int nchars) 
{
   int i, j;
   float sum = 0;

   for ( i = 0; i < nchars; i++ ) 
   {
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

void calculate_averages(void *rank, int lastIdx)
{
    int i;
    int strLength;
    int myId = *((int*) rank);

    for (i = 0; i < BATCH_SIZE; i++)
    {
        local_averages[i] = 0.0f;
    }
    
    for (i = 0; i < BATCH_SIZE; i++)
    {
        strLength = strlen(local_lines + (i * MAX_STRING));
        local_averages[i] = find_avg(local_lines + (i * MAX_STRING), strLength);
    }
}

void calculate_averages_serial(int length)
{
    int i;
    int strLength;
    for ( i = 0; i < length; i++ )
    {
        strLength = strlen(lines[i]);
        global_averages[i] = find_avg(lines[i], strLength);
    }
}

int main(int argc, char* argv[])
{
   int display = 0;
   int maxlines = 1000000;
   int i, lineCount, err;
   int numtasks, rank;
   float charsum = 0.0; 
   int nchars = 0; 
   struct timeval t1, t2;
   double elapsedTime;
   FILE *fd;
   MPI_Status Status;
   gettimeofday(&t1, NULL);
   err = MPI_Init(&argc, &argv);
   if (err != MPI_SUCCESS) 
   {
       printf("Error starting MPI program. Terminating.\n");
       MPI_Abort(MPI_COMM_WORLD, err);
   }

   MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   
   NUM_THREADS = numtasks;
   BATCH_SIZE = MAX_SIZE / NUM_THREADS;

   fflush(stdout);
   int* localLineCount = malloc(sizeof(int));
   if (rank == 0)
   {
       global_averages = malloc(sizeof(float) * MAX_SIZE);
       fd = fopen("/homes/dan/625/wiki_dump.txt", "r");
       for (lineCount = 0; lineCount < MAX_SIZE; lineCount++)
       {
           err = fscanf(fd, "%[^\n]\n", lines[lineCount]);
           if (err == EOF)
           {
               break;
           }
       }
       fclose( fd );
       *localLineCount = lineCount;
   }

   if (NUM_THREADS > 1)
   {
       local_lines = malloc(sizeof(char) * BATCH_SIZE * MAX_STRING);

       local_averages = malloc(sizeof(float) * BATCH_SIZE);
       MPI_Scatter(lines, BATCH_SIZE * MAX_STRING, MPI_CHAR, local_lines, BATCH_SIZE * MAX_STRING, MPI_CHAR, 0, MPI_COMM_WORLD);
       MPI_Bcast(localLineCount, 1, MPI_INT, 0, MPI_COMM_WORLD);

       calculate_averages(&rank, lineCount);

       MPI_Gather(local_averages, BATCH_SIZE, MPI_FLOAT, global_averages, BATCH_SIZE, MPI_FLOAT, 0, MPI_COMM_WORLD);
   }
   else
   {
       calculate_averages_serial(lineCount);
   }


   if (rank == 0 && display != 0)
   {
       for (i = 0; i < lineCount; i++)
       {
           printf("%d: %.1f\n", i, global_averages[i]);
       }     
   }

   MPI_Finalize();

   if (rank == 0)
   {
       gettimeofday(&t2, NULL);

       elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
       elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
       printf("DATA, %d, %f\n", NUM_THREADS, elapsedTime);
   }
   return 0;
}

