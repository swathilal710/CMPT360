/* -----------------------------------------------------
 * filename: sch_impl.h
 * name: Swathi Prema Lal
 * Student Id: 3147620
 * assignment: Assignment #2
 * Submission Date: October 17, 2026
 * file purpose: header file for process table
 * --------------------------------------------------------------*/
#ifndef _SCH_IMPL_H_
#define _SCH_IMPL_H_

#include <stddef.h> //For `size_t`

typedef struct sched
{
    int pid;
    int arrival;
    int cpu_time;
    int remaining_time;
    int first_run;
    int completion;
    int remaining_quantum;

} sched;

//table for storing the valid processes
typedef struct process_t{
    sched *entries;     //pointer to actual arr containing the processes
    size_t capacity;    //max # of entries arr can hold
    size_t count;       //# of items currently in the arr stored
    
    int *timeline;       //pointer to an arr that stores the process number of the process tht used the cpu burst (-1 if cpu is idle)
    size_t timeline_count;     //num of entries of the process pids in arr
    size_t timeline_capacity;   //max num of entries
} process_t;


//Function prototypes
process_t *create_table(void);
void grow_table(process_t *table);
void read_file(const char *filename, process_t *table);
int validate_record(sched *slot);
int duplicate_pid(sched *slot, process_t *table);
void free_table(process_t *table);
int compare_arrival(const void *a, const void *b);
int compare_pid(const void *a, const void *b);
int simulate_RR(process_t *table, int quantum);
void print_timeline(process_t *table);
void print_metrics(process_t *table);

// typedef struct queue
// {

//     sched **entries;   //pointer entries that point to the sched
//     size_t capacity; //max # of entries
//     size_t count;    //# of items currently in the queue
//     int head;       //item at the front of the queue about to dequeue
//     int rear;       //item at the back that got enqueued

// } queue;

// //Function Prototypes
// queue *create_arr(int n);


#endif