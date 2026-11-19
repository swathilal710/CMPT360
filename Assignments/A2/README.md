Assignment #2

# Academic Integrity Statement
I certify that this submission represents entirely my own work.


The program uses three structs:
- 'sched' : (Process Record): Represents a single process. It stores the pid, arrival time, cpu_time, remaining_time, first_run, completion, remaining_quantum

- 'process_t' : (Process Table): It stores a pointer to the array(entry) of 'sched' , the count of valid records, and the capacity allocated for the array. Moreover, it also contains a pointer to an array that stores the process number of the process that used the cpu burst (-1 if cpu is idle)

- 'Queue' : (Circular Queue): Represents a circular ready queue that hold the processess that's yet to run. Processes get enqueued after arrival and dequeued before run time.


# How Data is Managed
Dynamic memory allocation is used to store the process records and ready queue without hardcoding a fixed number of entries.

- Table Allocation: Memory is initially allocated for the entries using an initial capacity of 20.

- Capacity Growth: The 'dynamic_mem' function checks whether 'count == capacity'. If the array is full, it uses 'realloc' to double the capacity for the array.


- Clean-up: Function 'free_table' is used to call under fuctions that allocates memory (including the main) before program termination. It safely iterates through all the records, frees the COMMAND strings, then the record, then finally the Table. This is done to prevent any memory leaks.



# How to Compile
- From the Makefile provided, the program can be compiled using the instruction: 'make'
This produces an executable program called 'sched'. 
- To remove the executable files, the instruction 'make clean' is used. 



Run: make run


Valgrind: 
To check the program for memory errors and leaks: make valgrind
Valgrind runs using the filename, so before running it, add the filename beside the instruction: valgrind --leak-check=full ./$(MAIN) <filename.txt>


Known Errors or Limitations:
This program currently runs smoothly with no errors present.



