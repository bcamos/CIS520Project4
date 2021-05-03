#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_SIZE 1000000
#define MAX_STRING 2001
char* lines[MAX_SIZE];
char** local_lines;
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
    int startPos = ((long)myId) * (BATCH_SIZE / NUM_THREADS);
    int endPos = startPos + (BATCH_SIZE / NUM_THREADS);
    if (endPos - startPos < BATCH_SIZE)
    {
        endPos -= startPos;
    }
    else
    {
        endPos = BATCH_SIZE;
    }

    printf("myID = %d startPos = %d endPos = %d \n", myId, startPos, endPos); fflush(stdout);

    for (i = 0; i < BATCH_SIZE; i++)
    {
        local_averages[i] = 0.0f;
    }

    printf("myID = %d Init success \n", myId); fflush(stdout);

    if (myId == 0)
    {
        for (i = 0; i < endPos; i++)
        {
            strLength = strlen(local_lines[i]);
            local_averages[i] = find_avg(local_lines[i], strLength);
        }
    }    
    printf("myID = %d Done! \n", myId); fflush(stdout);
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
   BATCH_SIZE = MAX_SIZE / NUM_THREADS;
   printf("size = %d rank = %d\n", numtasks, rank);
   fflush(stdout);
   int* localLineCount = malloc(sizeof(int));
   if (rank == 0)
   {
       global_averages = malloc(sizeof(float) * MAX_SIZE);
       fd = fopen("/homes/dan/625/wiki_dump.txt", "r");
       for (lineCount = 0; lineCount < BATCH_SIZE; lineCount++)
       {
           lines[lineCount] = (char*)malloc(sizeof(char) * MAX_STRING);
           err = fscanf(fd, "%[^\n]\n", lines[lineCount]);
           if (err == EOF)
           {
               break;
           }
       }
       fclose( fd );
       *localLineCount = lineCount;
   }

   local_lines = malloc(sizeof(char**) * BATCH_SIZE);
   for (i = 0; i < BATCH_SIZE; i++)
   {
       local_lines[i] = malloc(sizeof(char) * MAX_STRING);
   }

   local_averages = malloc(sizeof(float) * BATCH_SIZE);
   MPI_Scatter(lines, BATCH_SIZE * MAX_STRING, MPI_CHAR, local_lines, BATCH_SIZE * MAX_STRING, MPI_CHAR, 0, MPI_COMM_WORLD);   
   MPI_Bcast(localLineCount, 1, MPI_INT, 0, MPI_COMM_WORLD);

   calculate_averages(&rank, lineCount);

   MPI_Gather(local_averages, BATCH_SIZE, MPI_FLOAT, global_averages, BATCH_SIZE, MPI_FLOAT, 0, MPI_COMM_WORLD);

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

