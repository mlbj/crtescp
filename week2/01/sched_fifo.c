#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <syslog.h>


// assignment definitions 
#define COURSE_IDX 1
#define ASSIGNMENT_IDX 3 

// number of threads to create
#define NUM_THREADS 128

// this is the policy we will use for scheduling.
// we could also use SCHED_OTHER or SCHED_RR, but 
// the problem explicitly asks for SCHED_FIFO
#define SCHED_POLICY SCHED_FIFO

// thread parameters struct
typedef struct{
    int thread_idx;
} thread_params_t;

// mutex for synchronization
pthread_mutex_t mutex;

// POSIX thread declarations and scheduling attributes
pthread_t threads[NUM_THREADS];
pthread_t startupthread;
thread_params_t thread_params[NUM_THREADS];
pthread_attr_t fifo_sched_attr;
struct sched_param fifo_param;


// this function sets the scheduler to SCHED_FIFO
void set_scheduler(void){
    int max_prio, cpuidx;
    cpu_set_t cpuset;
 
    // initialises the fifo_sched_attr variable to default settings
    pthread_attr_init(&fifo_sched_attr);
  
    // the inherit-scheduler attribute determines whether a thread created using the
    // thread attributes object attr will inherit its scheduling
    // attributes from the calling thread or whether it will take them
    // from attr. PTHREAD_EXPLICIT_SCHED means that threads that are created using 
    // attr take their scheduling attributes from the values specified 
    // by the attributes object. 
    pthread_attr_setinheritsched(&fifo_sched_attr, PTHREAD_EXPLICIT_SCHED);
  
    // this sets the scheduling policy
    pthread_attr_setschedpolicy(&fifo_sched_attr, SCHED_POLICY);
  
    // clears the cpuset variable and sets to the indicated CPU 
    CPU_ZERO(&cpuset);     
    cpuidx=(0);
    CPU_SET(cpuidx, &cpuset); 
  
    // uses the cpuset to set the thread affinity attribute to the predefined core
    pthread_attr_setaffinity_np(&fifo_sched_attr, sizeof(cpu_set_t), &cpuset);
  
    // returns  the  maximum  priority value that can  be used with the scheduling 
    // algorithm, and sets this value in fifo_param.sched_priority
    max_prio = sched_get_priority_max(SCHED_POLICY);
    fifo_param.sched_priority = max_prio;

    // sets both the scheduling policy and parameters for the thread whose
    // ID is specified in pid 
    if(sched_setscheduler(getpid(), SCHED_POLICY, &fifo_param) < 0){
        perror("sched_setscheduler");
    }
  
    // sets the scheduling  parameter  attributes  of  the  thread  attributes  
    // object referred to by attr to the values specified in the buffer pointed
    // to by param 
    pthread_attr_setschedparam(&fifo_sched_attr, &fifo_param);
   
}


// function to be called by each thread. 
void* inc_thread(void* threadp){
    int i, gsum=0;
    thread_params_t *thread_params = (thread_params_t*) threadp; 

    // thread logic 
    for(i=0; i<thread_params->thread_idx; i++){
        gsum=gsum+i;
    }   

    syslog(LOG_INFO, 
           "[COURSE:%d][ASSIGNMENT:%d]: Thread idx=%d, sum[1...%d]=%d Running on core : %d\n", 
           COURSE_IDX,
           ASSIGNMENT_IDX,
           thread_params->thread_idx, 
           thread_params->thread_idx, 
           gsum, 
           sched_getcpu());
}


// first thread. this will create all other threads.
void startup_thread(void* threadp){
    int i;
    
    // stdout debug 
    printf("This is the startup thread. It is running on CPU %d\n", sched_getcpu());
    
    // create threads  
    for (i=0; i<NUM_THREADS; i++){
        thread_params[i].thread_idx=i;
        pthread_create(&threads[i],                // pointer to thread descriptor
                       &fifo_sched_attr,           // use FIFO RT max priority attributes
                       (void*) inc_thread,         // thread function entry point
                       (void*) &(thread_params[i]) // parameters to pass in
        );
    }

    // join threads
    for(i=0; i<NUM_THREADS; i++){
        pthread_join(threads[i], NULL);
    }
}


int main (int argc, char *argv[]){
    int i=0;
    char uname_command[256] = {0};

    // clear /var/log/syslog and then send the uname -a output to it
    // NOTICE: this may need sudo 
    sprintf(uname_command, 
            "(echo -n [COURSE:%d][ASSIGNMENT:%d]:\\  &&  uname -a) | tee /var/log/syslog",
            COURSE_IDX,
            ASSIGNMENT_IDX);
    system(uname_command);
    
    // set scheduler
    set_scheduler();

    // create startup thread and join it 
    pthread_create(&startupthread, &fifo_sched_attr, (void*) startup_thread, 0);
    pthread_join(startupthread, NULL);
}
