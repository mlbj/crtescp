#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <time.h>

#define NUM_THREADS 1
#define COURSE_IDX 1
#define ASSIGNMENT 1 

typedef struct{
    int thread_idx;
} thread_params_t;

// shared resources
char hostname[] = "mq1"; 
char descriptor[] = "pthread";  

// mutex for synchronization
pthread_mutex_t mutex;

// POSIX thread declarations and scheduling attributes
pthread_t threads[NUM_THREADS];
thread_params_t thread_params[NUM_THREADS];

// this is the function called by the thread
void *counter_thread(void *threadp){
    int i;
    char* message = (char*)malloc(256*sizeof(char));

    thread_params_t *thread_params = (thread_params_t *)threadp;
    
    // hello world from thread log
    pthread_mutex_lock(&mutex);
    produce_syslog(hostname, descriptor, "Hello World from Thread!");
    pthread_mutex_unlock(&mutex);

    return NULL;
}

// this function writes our logs to stdout
void produce_syslog(char* hostname,
                    char* descriptor,
                    char* message){
    time_t now;
    struct tm *tm_info;
    char timestamp[32] = {0}; 
    char* full_message = (char*)malloc(256*sizeof(char));

    time(&now);
    tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%b %d %H:%M:%S", tm_info);
    sprintf(full_message, "%s %s %s [COURSE:%d,ASSIGNMENT:%d] %s\n", 
            timestamp, hostname, descriptor, 
            COURSE_IDX, ASSIGNMENT,
            message);

    // show full_message;
    printf("%s", full_message);

    // free
    free(full_message);
}

int main (int argc, char *argv[]){
    int rc;
    int i;
    char uname_output[256] = {0};

    // init mutex  
    if (pthread_mutex_init(&mutex, NULL) != 0){
        perror("pthread_mutex_init");
        return EXIT_FAILURE;   
    }
  
    // uname -a log 
    FILE *fp = popen("uname -a", "r");
    if (!fp){
        perror("popen");
        return EXIT_FAILURE;
    }  
    fgets(uname_output, sizeof(uname_output), fp);
    pclose(fp);
    printf("[COURSE:%d,ASSIGNMENT:%d] %s", COURSE_IDX, ASSIGNMENT, uname_output);

    // hello world log 
    pthread_mutex_lock(&mutex);
    produce_syslog(hostname, descriptor, "Hello World from Main!");
    pthread_mutex_unlock(&mutex);
    
    // create threads
    for(i=0; i<NUM_THREADS; i++){
        thread_params[i].thread_idx=i;

        pthread_create(&threads[i],                // pointer to thread descriptor
                       (void *)0,                  // use default attributes
                       counter_thread,             // thread function entry point
                       (void *)&(thread_params[i]) // parameters to pass in
                      );

    }

    // join threads
    for(i=0; i<NUM_THREADS; i++)
        pthread_join(threads[i], NULL);
 

    // destroy mutex
    pthread_mutex_destroy(&mutex);
}
