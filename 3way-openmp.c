#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

int main() {
    FILE *filePointer;
    char ch;
    double average;
    int sum, count=0;
    omp_set_num_threads(8);
    filePointer = fopen("wiki_dump.txt", "r");
    if (filePointer == NULL){
        printf("File is not available \n");
    }
    else{
    	#pragma omp parallel private(count, sum, average)
        while ((ch = fgetc(filePointer)) != EOF){
            if(ch != '\n'){
            	//	Display each charater
            	// printf("%c", ch);
            	count++;
            	sum += ch;
            	average = sum / count;
            }
            else{
            	//	Display sum of each line along with number of characters and average
            	printf("Sum = %d, Count = %d, Average = %0.2f\n", sum, count, average);
            	sum = 0;
            	count = 0;
            }
        }
    }
    fclose(filePointer);

    return 0;
}