/* -----------------------------------------------------
 * filename: queue_impl.c
 * name: Swathi Prema Lal
 * Student Id: 3147620
 * assignment: Assignment #2
 * Submission Date: October 17, 2026
 * file purpose: implements circular queue
 * --------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "queue_impl.h"


#define INITIAL_SIZE 20

Queue *createQueue(void)
{
    //create circular queuue
    Queue *circQ;

    circQ = malloc(sizeof(Queue));
    if (circQ == NULL) {
        return NULL;
    }
    //initlaize
    circQ->capacity = INITIAL_SIZE;
    circQ->elements = malloc(circQ->capacity * sizeof(*circQ->elements));
    if (circQ->elements == NULL) {
        free(circQ);
        return NULL;
    }
    
    circQ->count = 0;

    circQ->front = 0;
    circQ->rear = 0;
    
    return circQ;
}


//add process to the rear of queue
int enqueue(Queue *circQ, size_t *proc_index) { 

    if(circQ == NULL){
        return 0;
    }
    //check if queue is full
    if (circQ->count == circQ->capacity) {
        fprintf(stderr, "Queue is full.\n");
        return 0;
    }

  
    if (circQ->count == 0) //queue is empty
    { 
        circQ->front = 0;
        circQ->rear = 0; 
    } 

    
    //insert the index at the end 
    circQ->elements[circQ->rear] = *proc_index;   
    //move rear forwrad + wrap around
    circQ->rear = (circQ->rear + 1) % circQ->capacity; 
    circQ->count++;
    
    
    return 1;
} 


int dequeue(Queue *circQ, size_t *process_index) {
    //check if queue is empty 
    if (circQ->count == 0) {
        fprintf(stderr, "Queue is empty\n");

        circQ->front = 0;
        circQ->rear = 0;
        return 0;
    }

    //find the front
    *process_index = circQ->elements[circQ->front];   
    //Move front forward, wrap  if necessary
    circQ->front = (circQ->front + 1) % circQ->capacity; 
    circQ->count--;

    return 1;

}



//Free the queue and its array
void free_queue(Queue *circQ)
{
    if (circQ != NULL) {
        free(circQ->elements);
        free(circQ);
        
    }
}
