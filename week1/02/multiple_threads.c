#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <time.h>
 
#define NUM_THREADS 128
#define COURSE_IDX 1
#define ASSIGNMENT 2

typedef struct{
    int thread_idx;
} thread_params_t;

// shared resources
char hostname[] = "mq1"; 
char descriptor[] = "multiple_threads";  

// mutex for synchronization
pthread_mutex_t mutex;

// POSIX thread declarations and scheduling attributes
pthread_t threads[NUM_THREADS];
thread_params_t thread_params[NUM_THREADS];
 

void *inc_thread(void *threadp){
    int i, gsum=0;
    thread_params_t *thread_params = (thread_params_t*) threadp;
    char* thread_message = (char*)malloc(128*sizeof(char));

    for(i=0; i<thread_params->thread_idx; i++){
        gsum=gsum+i;
    }   

    // create final message
    sprintf(thread_message, 
            "Thread idx=%d, sum[1...%d]=%d\n", 
            thread_params->thread_idx, 
            thread_params->thread_idx, 
            gsum);

    pthread_mutex_lock(&mutex);
    produce_syslog(hostname, descriptor, thread_message, 1);
    pthread_mutex_unlock(&mutex);
    
    // free memory
    free(thread_message);
}

void produce_syslog(char* hostname,
                    char* descriptor,
                    char* message,
                    int show_metadata){
    time_t now;
    struct tm *tm_info;
    char timestamp[32] = {0}; 
    char* full_message = (char*)malloc(256*sizeof(char));

    if (show_metadata){
        time(&now);
        tm_info = localtime(&now);
        strftime(timestamp, sizeof(timestamp), "%b %d %H:%M:%S", tm_info);
        sprintf(full_message, "%s %s %s [COURSE:%d][ASSIGNMENT:%d]: %s", 
                timestamp, hostname, descriptor, 
                COURSE_IDX, ASSIGNMENT,
                message);
    }else{
        sprintf(full_message,"[COURSE:%d][ASSIGNMENT:%d]: %s", 
                COURSE_IDX, ASSIGNMENT,
                message);
    }

    // show full_message;
    printf("%s", full_message);

    // free
    free(full_message);
}
 
 
int main (int argc, char *argv[]){
    int rc;
    int i=0;
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
    produce_syslog(hostname, descriptor, uname_output, 0);

 
    // create threads
    for (i=0; i<NUM_THREADS; i++){
        thread_params[i].thread_idx=i;
        pthread_create(&threads[i],                // pointer to thread descriptor
                       (void *)0,                  // use default attributes
                       inc_thread,                 // thread function entry point
                       (void *)&(thread_params[i]) // parameters to pass in
                      );
    }
 
    for(i=0; i<NUM_THREADS; i++){
        pthread_join(threads[i], NULL);
    }

    // destroy mutex
    pthread_mutex_destroy(&mutex);
}
