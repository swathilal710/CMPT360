/* -----------------------------------------------------
 * filename: sch_impl.c
 * name: Swathi Prema Lal
 * Student Id: 3147620
 * assignment: Assignment #2
 * Submission Date: October 17, 2026
 * file purpose: Implements table array for processes
 * --------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "sch_impl.h"

#define INITIAL_SIZE 20

process_t *create_table(void)
{
    process_t *table = malloc(sizeof(process_t));

    if (table == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }

    table->capacity = INITIAL_SIZE;
    table->count = 0;

    table->timeline = NULL;     //currently empty: no mem allocated yet
    table->timeline_count = 0;
    table->timeline_capacity = 0;

    table->entries = malloc(table->capacity * sizeof(sched));

    if (table->entries == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        free(table);
        return NULL;
    }

    return table;
}

void grow_table(process_t *table)
{
    if (table->count < table->capacity)
    {
        return;
    }

    size_t new_capacity = table->capacity * 2;
    sched *new_entries = realloc(table->entries,
        new_capacity * sizeof(sched));

    if (new_entries == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        free_table(table);
        exit(1);
    }

    table->entries = new_entries;
    table->capacity = new_capacity;
}

void free_table(process_t *table)
{
    if (table != NULL)
    {
        free(table->timeline);
        free(table->entries);
        free(table);
    }
}
