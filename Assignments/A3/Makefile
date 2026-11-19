CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic

MAIN = vmsim
MAIN_SRCS = vmsim.c


all: $(MAIN)

$(MAIN): $(MAIN_SRCS) 
	$(CC) $(CFLAGS) $(MAIN_SRCS) -o $(MAIN)



.PHONY: clean


clean:
	rm -f $(MAIN)
