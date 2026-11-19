CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic

MAIN = sched
SRCS = sched.c sch_impl.c queue_impl.c
HDRS = sch_impl.h queue_impl.h

all: $(MAIN)

$(MAIN): $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) $(SRCS) -o $(MAIN)


.PHONY: run valgrind clean		#declare as targets

# ---- run ----
run:
	./$(MAIN) 

#---- Valgrind --------

valgrind: $(MAIN)
	valgrind --leak-check=full ./$(MAIN) 

# ---- Clean ----

clean:
	rm -f $(MAIN) 