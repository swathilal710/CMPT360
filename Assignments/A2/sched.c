/* -----------------------------------------------------
 * filename: sched.c
 * name: Swathi Prema Lal
 * Student Id: 3147620
 * assignment: Assignment #2
 * Submission Date: October 17, 2026
 * file purpose: Reads the workload file, validates processes, 
    and simulates scheduling algorithms (FCFS/RR)
 * --------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sch_impl.h"
#include "queue_impl.h"



//reads the workload file, validates every process, and stores valid processes in the table.
void read_file(const char *filename, process_t *table)
{
    FILE *fp = fopen(filename, "r");

    if (fp == NULL){
        fprintf(stderr, "Error opening File\n");
        free_table(table);
        exit(1);
    }

    char buf[500];  //temp arr to hold 1 line

    //Read each line from the file
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        char first_char;
        sched slot;     //temp struct to store the chars

        //Skip blank lines and comments
        //if the sscanf was able to successfully ignore the space and store it in first_char
        if (sscanf(buf, " %c", &first_char) != 1 || first_char == '#')
        {
            continue;      //skip char
        }

        //Read exactly three integers
        int valid_char = sscanf(buf, " %d %d %d",
            &slot.pid,
            &slot.arrival,
            &slot.cpu_time);

        // If the elements/records stored aren't 3 in ttl
        if (valid_char != 3)
        {
            free_table(table);
            fclose(fp);
            exit(1);
        }
           
        
        //validate records
        int check = validate_record(&slot);
        if (check == 0) {    //if the return val is 0 -> raise error and stop immediately : free Table
            fclose(fp);
            free_table(table);
            exit(1);

        }

        int dupliID = duplicate_pid(&slot, table);
        if (dupliID == 0) {     //if the return val is 0 -> raise error and stop immediately : free Table
            fclose(fp);
            free_table(table);

            exit(1);
        }

        //Initialize sched fields
        slot.remaining_time = slot.cpu_time; 
        slot.first_run = -1;      //init to indicate it hasnt run, if 0 = run
        slot.completion = 0;
        slot.remaining_quantum = 0;

        //Expand memory if the table is full
        grow_table(table);

        //store process in the table from the slot
        table->entries[table->count] = slot;
        table->count++;

    }

    
    fclose(fp);

}
int validate_record(sched *slot) {

        //PID must be nonnegative
        if (slot->pid < 0) {
            return 0;
        }

        //arrival time must be >= 0
        if (slot->arrival < 0) {
            return 0;
        }


        //CPU time must be positive
        if (slot->cpu_time <= 0) {
            return 0;
        }


        return 1;
    

}


int duplicate_pid(sched *slot, process_t* table) {
    
    if (table == NULL) {
        return 0;
    }
   
    int current_PID = slot->pid;
    size_t i;
    for (i = 0; i < table->count; i++) {
        if (table->entries[i].pid == current_PID) {  //if the PID at the index in the Table struct == current stored PID -> duplicate
            return 0;
        
        }
        
    }
    
    return 1;
     
    
}

//sort by Arrival Time 
int compArrival(const void* a, const void* b) {
    sched* A = (sched*) a;
    sched* B = (sched*) b;

    //If arrival times are different, sort by arrival time
    if (A->arrival != B->arrival) {
        return (A->arrival - B->arrival);
    }
    
    //If they arrived at the exact same time, smaller PID goes first
    return (A->pid - B->pid);
}

//sort  by PID
int compPid(const void* a, const void* b) {
    sched* A = (sched*) a;
    sched* B = (sched*) b;
    
    return (A->pid - B->pid);
}

//FIFO
void simulate_fcfs(process_t *table){

    if ((table == NULL) || (table->count <= 0)) {
        return;
    }
    int current_time = 0;

    //sort processes by arrival time, then PID
    qsort(table->entries, table->count, sizeof(table->entries[0]), compArrival);

    for (size_t i = 0; i < table->count; i++)
    {
        sched *p = &table->entries[i];      //pointer to the first process in the enrty

        //Wait if the process has not arrived yet
        if (current_time < p->arrival)
        {
            current_time = p->arrival;
        }

        //Record when the process first starts running
        p->first_run = current_time;

        //Run the process until its CPU burst is complete
        current_time += p->cpu_time;

        //Record the completion time
        p->completion = current_time;

        //The process stopped executing
        p->remaining_time = 0;
    }
}

//Record which process used the CPU for one time unit.
int timeline(process_t *table, int pid)
{
    //if there are no more spots left in the timeline arr
    if (table == NULL || table->timeline_count >= table->timeline_capacity) {
        fprintf(stderr, "Timeline capacity exceeded.\n");
        return 0;
    }

    //Store the pid, or -1 if the CPU is idle
    table->timeline[table->timeline_count] = pid;
    table->timeline_count++;

    return 1;
}


int simulate_RR(process_t *table, int quantum)
{
    if (table == NULL || quantum <= 0 || table->count == 0) {
        return 0;
    }

    //new timeline for RR

    table->timeline = NULL;
    table->timeline_count = 0;
    table->timeline_capacity = 0;

    //find ttl timeline
    int ttl_cpu = 0;    //tracks ttl cpu bursts
    int new_arrival = 0;    //tracks arrival time of new processes
    //loop through every process in the table and find the ttl cpu usage of it
    for (size_t i = 0; i < table->count; i++) {
        ttl_cpu += table->entries[i].cpu_time;
        //if this process arrives later than new one, make that the new arrival 
        if (table->entries[i].arrival > new_arrival) {
            new_arrival = table->entries[i].arrival;
        }
    }

    int max_time = new_arrival + ttl_cpu;

    table->timeline_capacity = (size_t)max_time;    //max numbre of timeline slots
    table->timeline = malloc(table->timeline_capacity * sizeof(*table->timeline)); //allocate enough room for the max time
    

    qsort(table->entries, table->count, sizeof(table->entries[0]), compArrival);

    //loop through table and initialize the process data 
    for (size_t i = 0; i < table->count; i++) {
        sched *p = &table->entries[i];
        p->remaining_time = p->cpu_time;    //time left == full cpu burst
        p->first_run = -1;  //cant be 0 since some processes start at 0
        p->completion = 0;
        p->remaining_quantum = 0;
    }

    Queue *readyQ = createQueue();  //create queue
    

    int current_time = 0;
    size_t next_prc = 0;    //new process abt to be enqueued
    size_t completed = 0;   //# of processes thats finished running

    //continue until all processes has run
    while (completed < table->count) {

        //if theres a new process and it has arrived by the current time => enqeuue
        while (next_prc < table->count && table->entries[next_prc].arrival <= current_time) {
            enqueue(readyQ, &next_prc);
            next_prc++;
        }

       

        //dequeue next ready process
        size_t process;    //hold the index of the removed process
        dequeue(readyQ, &process);
        sched *p = &table->entries[process];    //pointer to point to the remoeved process
        if (p->first_run == -1) {   //if its the first time running, begins at cur cpu time
            p->first_run = current_time;
        }

        //run process for 1 quantum + record
        p->remaining_quantum = quantum;

        while (p->remaining_quantum > 0 && p->remaining_time > 0) {
            timeline(table, p->pid);    //record process time

            current_time++;
            p->remaining_time--;
            p->remaining_quantum--;     //decreemnt tick by 1 quantum
        }

        //check if process has used all the cpu time
        if (p->remaining_time == 0) {
            p->completion = current_time;
            completed++;
        } 
        //else if process still need cpu time, enqueue it back into the arr to go again
        enqueue(readyQ, &process);
        }

    }

    //free made queue
    free_queue(readyQ);
    return 1;
}



//Print the CPU timeline
void print_timeline(process_t *table)
{
    int ttl_time = 0;
    if (table->timeline != NULL) {      
        ttl_time = table->timeline_count;   //ttl time is the timeline count per process
    } 
    //for FIFO, no timeline
    else {
        for (size_t i = 0; i < table->count; i++) {
            //if process finish before ttl time
            if (table->entries[i].completion > ttl_time) {
                ttl_time = table->entries[i].completion;
            }
        }
    }

    //Print the time row
    printf("time:");

    for (int t = 0; t < ttl_time; t++) {
        printf(" %d", t);
    }

    printf("\n");

    //Print the CPU run 
    printf("run :");

    for (int t = 0; t < ttl_time; t++) {
        int pid = -1;   //-1 b/c 0 could be an pid

        if (table->timeline != NULL) {  //for RR
            pid = table->timeline[t];
        } 
        //for FIFO
        else {
            for (size_t i = 0; i < table->count; i++) {
                sched *p = &table->entries[i];
                //process has started by time t and has not completed yet
                if (t >= p->first_run && t < p->completion) {
                    pid = p->pid;
                }
            }
        }

        //i fno process was found -> cpu is idle
        if (pid == -1)
        {
            printf(" -");
        }
        else
        {
            printf(" %d", pid);
        }
    }

    printf("\n");
}



//Print process metrics and system metrics
void print_metrics(process_t *table)
{
    if (table == NULL || table->count == 0) {
        return;
    }

    long int ttl_tat = 0;   
    long int ttl_resp = 0;
    int context_switches = 0;
    int prev_pid = -1;
    int total_time = 0;

    //Sort processes by PID: ascending order 
    qsort(table->entries, table->count, sizeof(table->entries[0]), compPid);

    //Find the final completion time
    for (size_t i = 0; i < table->count; i++)
    {
        if (table->entries[i].completion > total_time)
        {
            total_time = table->entries[i].completion;
        }
    }

    //calculate and print each process's metrics
    for (size_t i = 0; i < table->count; i++)
    {
        sched *p = &table->entries[i];

        int tat = p->completion - p->arrival;

        int response = p->first_run - p->arrival;

        printf("P%d: first run=%d completion=%d TAT=%d RESP=%d\n",
            p->pid,
            p->first_run,
            p->completion,
            tat,
            response
        );

        ttl_tat += tat;
        ttl_resp += response;
    }

    //find context switches for RR
    for (int t = 0; t < total_time; t++) {
        int current_pid = -1;

        if (table->timeline != NULL) {
            current_pid = table->timeline[t];
        } 
        
        //for FIFO
        else {
            for (size_t i = 0; i < table->count; i++) {
                sched *p = &table->entries[i];
                if (t >= p->first_run && t < p->completion) {
                    current_pid = p->pid;
                }
            }
        }

        //Count a switch: if process is running and one has already run and they are not != -> count swithc
        if (current_pid >= 0 && prev_pid >= 0 && current_pid != prev_pid)
        {
            context_switches++;
        }

        //Idle time does not reset the previous PID
        if (current_pid >= 0)
        {
            prev_pid = current_pid;
        }
    }

    //calc metrics
    double avgTat = (double) ttl_tat / table->count;
    double avgResp = (double) ttl_resp / table->count;

   
    //Print the system metrics
    printf("System: ctx_switches=%d, avgTAT=%.3f, avgRESP=%.3f\n",
        context_switches, avgTat, avgResp);

    
    
}




int main(int argc, char *argv[])
{
    const char *filename = NULL;     //pointer to the string in argv that stores the filename
    const char *policy = NULL;  //pointer to policy instr
    
    int quantum = 0;     //quantum num
   

    //parse CLI args
    //loop through command line args and comapre the program name with the char length to see if they match
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "--policy=", 9) == 0) {
            policy = &argv[i][9];

            //if policy is not "FCFS" and "RR"
            if (strcmp(policy, "FCFS") != 0 && strcmp(policy, "RR") != 0) {
                fprintf(stderr, "Usage: ./sched --policy=FCFS|RR [--quantum=N] --in=FILE\n");
                return 1;
            }

            
        } 
        if (strncmp(argv[i], "--in=", 5) == 0) {
            //check if theres anything after "--in="
            if (argv[i][5] == '\0') {
                fprintf(stderr, "Usage: ./sched --policy=FCFS|RR [--quantum=N] --in=FILE\n");
                return 1;
            }

            filename = &argv[i][5];
        } 

        if (strncmp(argv[i], "--quantum=", 10) == 0) {
            quantum = atoi(&argv[i][10]);
            

        } 
    }

    process_t *table = create_table();
    if (table == NULL) {
        return 1;
    }

    read_file(filename, table);

    if (strcmp(policy, "RR") == 0) {
        simulate_RR(table, quantum);
    } 
    else {
        simulate_fcfs(table);
    }

    print_timeline(table);
    print_metrics(table);
    free_table(table);
    return 0;
}