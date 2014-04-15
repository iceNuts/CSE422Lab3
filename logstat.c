/*
*   File created by Li Zeng @ April 15 3:39AM 2014 WUSTL 13 FALL
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <regex.h>


/*
*   print usage
*/

void printUsage()
{
    printf("logstat [-f <filename>] -r");
}


void printFile(char* filename)
{
    int c;
    FILE *file;
    file = fopen(filename, "r");
    if (file) {
        while ((c = getc(file)) != EOF)
            putchar(c);
        fclose(file);
    }
}

void printFileSize(char *filename) 
{
    struct stat st; 

    if (stat(filename, &st) == 0)
        printf("FILE SIZE : %lld\n", st.st_size);
    else
        printf("file size read error\n");
}

char *substring(char *string, int position, int length) 
{
   char *pointer;
   int c;
 
   pointer = malloc(length+1);
 
   if (pointer == NULL)
   {
      printf("Unable to allocate memory.\n");
      exit(EXIT_FAILURE);
   }
 
   for (c = 0 ; c < position -1 ; c++) 
      string++; 
 
   for (c = 0 ; c < length ; c++)
   {
      *(pointer+c) = *string;      
      string++;   
   }
 
   *(pointer+c) = '\0';
 
   return pointer;
}

void printMaxThreadNumber(char *filename) 
{
    FILE *fp;
    ssize_t read;
    char * line = NULL;
    size_t len = 0;
    fp = fopen(filename, "r");
    int max = -1;
    while ((read = getline(&line, &len, fp)) != -1) 
    {
        char* pre = strstr(line, "concurrent process number ");
        
        if (pre == NULL)  continue;
        
        char* end = strstr(line, ", memory mapped");
        if (end == NULL) continue;

        pre = pre + strlen("concurrent process number ");

        char offset[strlen(pre) - strlen(end) + 1];
        int i = 0;
        for (i = 0; i < (strlen(pre) - strlen(end) + 1); i++, pre++)
          offset[i] = *pre;
        offset[i] = '\0';
        int num = atoi(offset);
        if (num > max)
            max = num;
    }
    printf("MAX THREAD #: %d\n", max);
}

int main(int argc, char **argv)
{
    char *filename;
    int remove_f = 0;
    // print usage
    if (argc > 4 || argc < 3) {
        printUsage();
    }
    else
    {
        filename = argv[2];
        if (argc == 4) {
            remove_f = 1;
        }
    }

    printFileSize(filename);
    printFile(filename);
    printMaxThreadNumber(filename);
    if (remove_f == 1) 
        remove(filename);

    return 0;
}






