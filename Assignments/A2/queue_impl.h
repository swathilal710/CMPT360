/* -----------------------------------------------------
 * filename: queue_impl.h
 * name: Swathi Prema Lal
 * Student Id: 3147620
 * assignment: Assignment #2
 * Submission Date: October 17, 2026
 * file purpose: header file for circular queue implementation
 * --------------------------------------------------------------*/

#ifndef _QUE_IMPL_H_
#define _que_IMPL_H_

#include <stddef.h> //For `size_t`

typedef struct Queue
{
        size_t capacity;    //max # of entry slots in array
        size_t count;      //num of slots
        size_t front;      //index of first elem to be dequeued
        size_t rear;       //index of last elem to be enqueued
        size_t *elements;  //pointer to the dynamically allocated integer array
	
}Queue;

Queue *createQueue(void);
int enqueue(Queue *circQ, size_t *process_index);
int dequeue(Queue *circQ, size_t *process_index);
void free_queue(Queue *circQ);



#endif