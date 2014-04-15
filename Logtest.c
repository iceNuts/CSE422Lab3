/*
*   File created by Li Zeng @ April 15 3:39AM 2014 WUSTL 13 FALL
*/


#define _GNU_SOURCE
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


#define NUMCHAR  (100000)
#define FILESIZE (NUMCHAR * sizeof(char))

int active_count;
sem_t mutex;
int file_creat_f;
int file;
char *file_memory;
int len;
FILE *fp;

void *childTask(void* taskInfo);
void logFile(char* filename, char* message);
void printUsage();
int doesFileExist(const char *filename);
void *threadBatch(void* taskInfo);


struct task_info {
    char* filename;
    int thread_number;
    int parent_id;
};


/*
*   log file with mmap
*/

void logFile(char* filename, char* message)
{
    fp = fopen(filename, "a");
    fprintf(fp, "%s", message);
}

/*
*   print usage
*/

void printUsage()
{
    printf("logtest [-f <filename>] -n <n>");
}

/*
*   file not exist then return 0
*/

int doesFileExist(const char *filename) 
{
    struct stat st;
    int result = stat(filename, &st);
    return result == 0;
}


/*
*   thread batch
*/
void *threadBatch(void* taskInfo)
{    
    active_count = 0;
    sem_init(&mutex, 0, 1);

    pthread_t tid = pthread_self();
    struct task_info *task = (struct task_info*) taskInfo;
    
    /* attr map */
    char* filename = task -> filename;
    task -> parent_id = (int)tid;
    
    /*
    *   If file not exist create a new file and log
    */

    if (doesFileExist(filename) == 0) 
    {
        char message[1000] = {};
        struct timeval now;
        gettimeofday(&now, NULL);
        sprintf(message, "Main program thread [%d:%d], begin new logging event at %ld.%06u\n", getpid(), (int)tid, now.tv_sec, now.tv_usec);
        logFile(filename, message);
    }
    /*
    *   Now create new thread
    */
    int i = 0;
    int threadNumber = task -> thread_number;
    pthread_t thread_pool[threadNumber];
    /* Assign tasks */
    for (i = 0; i < threadNumber; i++) 
    {
        pthread_create(&thread_pool[i], NULL, childTask, (void*) task);
    }
    /* run tasks */
    for (i = 0; i < threadNumber; i++)
    {
        pthread_join(thread_pool[i], NULL);
    }

    sem_destroy(&mutex);

    char message[1000] = {};
    struct timeval now;
    gettimeofday(&now, NULL);
    sprintf(message, "Main program thread [%d:%d], end new logging event at %ld.%06u\n", getpid(), (int)tid, now.tv_sec, now.tv_usec);
    logFile(filename, message);
    
    return 0;
}

/*
*   child thread action 
*/
void *childTask(void* taskInfo)
{
    struct task_info *task = (struct task_info*) taskInfo;
    
    pthread_t tid = pthread_self();
    char* filename = task -> filename;
    char message[1000] = {};
    struct timeval now;
    gettimeofday(&now, NULL);

    /* start working */
    sem_wait(&mutex);
    int _active_count = active_count;
    if (active_count == 0) {
        sprintf(message, "First Process [%d:%d], concurrent process number %d, memory mapped file at %ld.%06u\n", task -> parent_id, (int)tid, ++active_count, now.tv_sec, now.tv_usec);
    }
    else 
    {
         sprintf(message, "Other Process [%d:%d], concurrent process number %d, memory mapped file at %ld.%06u\n", task -> parent_id, (int)tid, ++active_count, now.tv_sec, now.tv_usec);
    }
    logFile(filename, message);
    sem_post(&mutex);

    /* sleep for 10 sec */
    sleep(10);

    /* wake up to work again */
    message[0] = '\0';
    gettimeofday(&now, NULL);

    sem_wait(&mutex);
    sprintf(message, "Process [%d:%d], concurrent process number %d, awake at %ld.%06u\n", task -> parent_id, (int)tid, (_active_count + 1), now.tv_sec, now.tv_usec);
    logFile(filename, message);
    active_count--;
    sem_post(&mutex);

    return 0;
}


int main(int argc, char **argv)
{

    char filename[32] = {}; 
    int threadNumber;
    int current_pid = (int)getpid();
    
    // print usage
    if (argc > 5 || argc < 3) {
        printUsage();
    }
    // arguments ready
    else
    {
        // default settings
        if (argc == 3 && strcmp(argv[1], "-n") == 0) 
        {
            char local_pid[32]= {};
            sprintf(local_pid, "%d", current_pid);
            strcat(filename, "logfile_");
            strcat(filename, local_pid);
            strcat(filename, ".log");
            threadNumber = atoi(argv[2]);
        }
        // filename setting
        else if (argc == 5 && strcmp(argv[1], "-f") == 0 && strcmp(argv[3], "-n") == 0)
        {
            strcpy(filename, argv[2]);
            threadNumber = atoi(argv[4]);
        }
        // Failed
        else 
        {
            printUsage();
        }
    }

    /* initialize */
    len = 0;
    file_creat_f = 0;
    struct task_info *task = malloc(sizeof(struct task_info));
    task -> filename = filename;
    task -> thread_number = threadNumber;

    pthread_t mainThread;
    pthread_create(&mainThread, NULL, threadBatch, (void*) task);

    pthread_join(mainThread, NULL);

    return EXIT_SUCCESS;
}













