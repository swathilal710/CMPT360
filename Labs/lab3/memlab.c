#define _GNU_SOURCE     //enables GNU/Linux features, helps make MAP_ANON avail for creating mem

#include <errno.h>  //records if the library system has failed
#include <inttypes.h>   //firxed width ints
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>   //memory mapping funcs like mmap
#include <sys/resource.h>   //sets limitations on memory
#include <sys/types.h>     
#include <sys/wait.h>       //wait(), waitpid(), status
#include <unistd.h>     //Gets the page size, creates a child process, and exits the child without running normal cleanup handlers.


int global_var = 10;    //int init to 10: exits entire time prog runs

//defines a func that prints the prog's supported commands
//para: const char *prog -> pointer to a str containing the prog's name
//need this for A3
static void print_usage(const char *prog) {
    fprintf(stderr,
            "Usage:\n"
            "  %s --translate BASE BOUND VA\n"
            "  %s --regions\n"
            "  %s --heap\n"
            "  %s --fault\n"
            "  %s --limit\n",
            prog, prog, prog, prog, prog);
}

// Parses a non-negative integer string into a uint64_t.
// Returns 0 on success and -1 if the input is invalid or out of range.
static int parse_u64(const char *text, uint64_t *value) {
    // uint64_t is an unsigned integer type that is exactly 64 bits wide.
    if (text[0] == '-') {
        return -1;
    }

    // strtoull() sets end to the first character it could not parse.
    char *end = NULL;

    // Clear errno so conversion errors, including overflow, can be detected.
    errno = 0;
    // Base 0 allows decimal, hexadecimal (0x...), and octal (0...) input.
    unsigned long long v = strtoull(text, &end, 0);

    // Reject overflow, empty input, or any trailing non-numeric characters.
    if (errno != 0 || end == text || *end != '\0') {
        return -1;
    }

    // Store the parsed value through the output pointer.
    *value = (uint64_t)v;
    return 0;
}

static void run_translate(const char *base_s, const char *bound_s, const char *va_s) {
    uint64_t base, bound, va;
    if (parse_u64(base_s, &base) != 0 ||
        parse_u64(bound_s, &bound) != 0 ||
        parse_u64(va_s, &va) != 0) {
        fprintf(stderr, "BASE, BOUND, and VA must be non-negative integers.\n");
        exit(EXIT_FAILURE);
    }

    printf("Base  = %" PRIu64 "\n", base);
    printf("Bound = %" PRIu64 "\n", bound);
    printf("VA    = %" PRIu64 "\n", va);

    if (va >= bound) {
        printf("Result: PROTECTION FAULT (VA must satisfy 0 <= VA < bound)\n");
        return;
    }

    if (UINT64_MAX - base < va) {
        printf("Result: arithmetic overflow\n");
        return;
    }

    printf("Result: VALID, PA = %" PRIu64 "\n", base + va);
}

static void run_regions(void) {
    int local_var = 20;
    int *heap_var = malloc(1024);
    if (heap_var == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    printf("Code   (run_regions): %p\n", (void *)&run_regions);     //addr of complied functions/instructions
    printf("Global (global_var) : %p\n", (void *)&global_var);      //addr of global + static variables
    printf("Heap   (allocation) : %p\n", (void *)heap_var);         //dynamic mem (malloc())
    printf("Stack  (local_var)  : %p\n", (void *)&local_var);       //local variables + return calls

    //Due to ASLR: addr randomizes each running of the process

    free(heap_var);
}

static void run_heap(void) {
    enum { BLOCKS = 10, BLOCK_SIZE = 1024 };
    void *blocks[BLOCKS] = {0};

    for (int i = 0; i < BLOCKS; ++i) {
        blocks[i] = malloc(BLOCK_SIZE);
        if (blocks[i] == NULL) {
            perror("malloc");
            for (int j = 0; j < i; ++j) {
                free(blocks[j]);
            }
            exit(EXIT_FAILURE);
        }
        printf("Block %2d: %p\n", i, blocks[i]);
    }

    puts("Note: the C standard does not guarantee that separate malloc() blocks are contiguous or monotonic.");

    for (int i = 0; i < BLOCKS; ++i) {
        free(blocks[i]);
    }
}

static void run_fault(void) {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) {
        perror("sysconf");
        exit(EXIT_FAILURE);
    }

    //creating protected memoery 
    void *page = mmap(NULL, (size_t)page_size, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (page == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    //changes mapping to PROT_NONE so that an attempted access to memeory trigger protection fault
    if (mprotect(page, (size_t)page_size, PROT_NONE) != 0) {
        perror("mprotect");
        munmap(page, (size_t)page_size);
        exit(EXIT_FAILURE);
    }

    fflush(NULL);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        munmap(page, (size_t)page_size);
        exit(EXIT_FAILURE);
    }

    //waiting for a child process
    if (pid == 0) {
        puts("Child: attempting to write to a page with PROT_NONE...");
        fflush(stdout);
        volatile unsigned char *p = (volatile unsigned char *)page;
        p[0] = 42;
        _exit(EXIT_SUCCESS);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        munmap(page, (size_t)page_size);
        exit(EXIT_FAILURE);
    }

    if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        printf("Parent: child terminated by signal %d (%s)\n", sig, strsignal(sig));
    } else if (WIFEXITED(status)) {
        printf("Parent: child exited normally with status %d\n", WEXITSTATUS(status));
    }

    munmap(page, (size_t)page_size);
}

//sets/preserves a finite lim on VA, repeatedly requests 1 MiB blocks, report when it stops and fres
static void run_limit(void) {
    const rlim_t target = (rlim_t)256 * 1024 * 1024;    //256 MiB mem-space lim
    const size_t block_size = 1024 * 1024;      //1 MiB per alloc
    enum { MAX_BLOCKS = 512 };      //lim of attempts or ttl requests
    void *blocks[MAX_BLOCKS] = {0};     //init all slots to 0, pointer to free allocation better

    struct rlimit lim;
    if (getrlimit(RLIMIT_AS, &lim) != 0) {      //getrlimit: asks what the addr space is, RLIMIT_AS: lims ttl VA to proc
        perror("getrlimit");    //if OS fails, program prints error + exits
        exit(EXIT_FAILURE);
    }

    rlim_t new_soft = target;

    if (lim.rlim_max != RLIM_INFINITY && new_soft > lim.rlim_max) {     //hard lim
        new_soft = lim.rlim_max;
    }
    //if curr lim soft lim is alr lwr, keep it low
    if (lim.rlim_cur != RLIM_INFINITY && lim.rlim_cur < new_soft) {     //soft lim
        new_soft = lim.rlim_cur;
    }

    //asking OS to enforce it
    struct rlimit new_lim = lim;
    new_lim.rlim_cur = new_soft;    //copy coft lim + ask OS to apply
    if (setrlimit(RLIMIT_AS, &new_lim) != 0) {
        perror("setrlimit");
        exit(EXIT_FAILURE);
    }

    //make sure lim is finite (however, soft lim starts with 256 MiB and lowers so redundant)
    if (new_soft == RLIM_INFINITY) {
        puts("Unable to establish a finite address-space limit on this system.");
        exit(EXIT_FAILURE);
    }

    //converts byte coun tMiB to readable status
    printf("Soft virtual-address-space limit: %.0f MiB\n",
           (double)new_soft / (1024.0 * 1024.0));

    
    //ask to get more MiB
    int count = 0;
    for (; count < MAX_BLOCKS; ++count) {
        blocks[count] = malloc(block_size);
        if (blocks[count] == NULL) {    //if malloc returns NULL:  stops request
            break;
        }
        //volatile: touches only the 1st byte of each 1 MiB block
        ((volatile unsigned char *)blocks[count])[0] = 0;   //else: write 0 to first byte of block
    }

    //reporting outcome
    //if loop stopped early: report how many allocations succeeded before malloc returned NULL
    if (count < MAX_BLOCKS) {
        printf("Allocation stopped safely after %d successful 1-MiB blocks: malloc() returned NULL.\n", count);
    } else {
        //if loop = 512: stopped on its own
        puts("Safety stop reached before malloc() failed.");
    }

    //cleaning up: frees every successdully allocated block
    for (int i = 0; i < count; ++i) {
        free(blocks[i]);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "--translate") == 0) {
        if (argc != 5) {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
        run_translate(argv[2], argv[3], argv[4]);
    } else if (strcmp(argv[1], "--regions") == 0 && argc == 2) {
        run_regions();
    } else if (strcmp(argv[1], "--heap") == 0 && argc == 2) {
        run_heap();
    } else if (strcmp(argv[1], "--fault") == 0 && argc == 2) {
        run_fault();
    } else if (strcmp(argv[1], "--limit") == 0 && argc == 2) {
        run_limit();
    } else {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
