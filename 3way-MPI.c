#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BATCH_SIZE 1000000
#define MAX_STRING 2001
char lines[BATCH_SIZE];
float global_averages[BATCH_SIZE];
float local_averages[BATCH_SIZE];
int NUM_THREADS;

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

void calculate_averages(void *rank, int lastIdx)
{
    int i;
    int strLength;
    int myId = *((int*) rank);
    int startPos = ((long)myId) * (BATCH_SIZE / NUM_THREADS);
    int endPos = startPos + (BATCH_SIZE / NUM_THREADS);
    if (endPos > lastIdx)
    {
        endPos = lastIdx;
    }

    printf("myID = %d startPos = %d endPos = %d \n", myId, startPos, endPos); fflush(stdout);

    for (i = 0; i < lastIdx; i++)
    {
        local_averages[i] = 0.0;
    }

    for (i = startPos; i < endPos; i++)
    {
        strLength = strlen(lines[i]);
        local_averages[i] = find_avg(lines[i], strLength);
    }
}

int main(int argc, char* argv[])
{
   int maxlines = 1000000;
   int i, lineCount, err;
   int numtasks, rank;
   float  charsum=0.0; 
   int nchars = 0; 
   FILE *fd;
   MPI_Status Status;

   err = MPI_Init(&argc, &argv);
   if (err != MPI_SUCCESS) 
   {
       printf("Error starting MPI program. Terminating.\n");
       MPI_Abort(MPI_COMM_WORLD, err);
   }

   MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   
   NUM_THREADS = numtasks;
   printf("size = %d rank = %d\n", numtasks, rank);
   fflush(stdout);

   if (rank == 0)
   {
       fd = fopen("/homes/dan/625/wiki_dump.txt", "r");
       for (lineCount = 0; lineCount < BATCH_SIZE; lineCount++)
       {
           lines[i] = (char*)malloc(MAX_STRING);
           err = fscanf(fd, "%[^\n]\n", lines[lineCount]);
           if (err == EOF)
           {
               break;
           }
       }
       fclose( fd );
   }

   MPI_Bcast(lines, BATCH_SIZE * MAX_STRING, MPI_CHAR, 0, MPI_COMM_WORLD);

   calculate_averages(&rank, lineCount);

   MPI_Reduce(local_averages, global_averages, BATCH_SIZE, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

   if (rank == 0)
   {
       for (i = 0; i < lineCount; i++)
       {
           printf("%d: %.1f\n", i, global_averages[i]);
       }       
   }

   MPI_Finalize();
   return 0;
}

