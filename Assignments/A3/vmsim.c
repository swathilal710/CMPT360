#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>      //for isdigit

//checking mode types
#define MODE_BB 1
#define MODE_SEG 2


//structure to store all the command line arguments and decimal values for checking
typedef struct parse {
    uint64_t base;      //PA for BB
    uint64_t limit;     //valid VA space
    int mode;           //identifies the mode type
    const char* config_path;    //pointer to congif text file
    const char* trace_path;     //pointer to trace text file

} parse;



//struct to store all the trace BB file contents
typedef struct traceBB {
    char OP;    //R or W 
    uint64_t VA;    //Virtual addr

    int accessess;     //ttl valid parse entries
    int ok;            //successful translations
    int faultB;        //# of bound faults
} traceBB;




static void print_usage() {
    fprintf(stderr, "\nUsage:\n"
        "./vmsim --mode=bb --base=N --limit=N --trace=FILE\n"
        "./vmsim --mode=seg --config=FILE --trace=FILE\n");

}

//check for non negative decimal ints
static int parse_ndecimal(const char *num, uint64_t *val){     //*num points to the str of int in the cli

    //check for errors
    if (num == NULL || num[0] == '\0' || num[0] == '-' ) {      //if the first char is negative '-'
        return -1;
    }

    //reset detecting conversion error
    errno = 0;

    //pointer to the first char that hasnt been passed
    char *end = NULL;

    unsigned long long number = strtoull(num, &end, 10);    //base 10 arg for decimals

    //if an overflow detected, or if the input is empty or if there are left over char: return -1
    if (errno != 0 || end == num || *end != '\0') {
        return -1;
    }

    //store the val
    *val = (uint64_t)number;
    
    return 0;

}

//read CLI and store in struct 
static int parse_arg(int argc, char *argv[], parse *commands) {     //temp pointer to the struct
    //check if the struct is empty
    if (commands == NULL) {
        return -1;
    }

    

    //loop for the duration of the arg count int: 4/5
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];    //store the coammnd line string
        //advance to 7 chars to access the command
        const char *cmdtype = &arg[7];

        if (strncmp(arg, "--mode=", 7) == 0) {
            
            if (strcmp(cmdtype, "bb") == 0) {
                commands->mode = MODE_BB;   //mode = 1
            

            } 
            else if (strcmp(cmdtype, "seg") == 0) {
                commands->mode = MODE_SEG;   //mode = 2
            }

            else {
                return -1;
            }
        }

        //base: base can't be -
        else if (strncmp(arg, "--base=", 7) == 0) {
            //call helper func to check for neg deci error or store it
            int result = parse_ndecimal(cmdtype, &commands->base);   //takes the addr of the 1st char in the str + store it in  base
            if (result == -1) {
                return -1;
            }
        }

    
        //limits: limits cant be - either
        else if (strncmp(arg, "--limit=", 8) == 0) {
            //call helper func to check for neg deci error or store it
            int result = parse_ndecimal(&arg[8], &commands->limit);   //takes the addr of the 1st char in the str + store it in limit
            if (result == -1) {
                return -1;
            }
    
        }

        //trace
        else if (strncmp(arg, "--trace=", 8) == 0) {
        
            //if index 8 -> 9th char is not 't' return -1
            if (arg[8] != 't') {
                fprintf(stderr, "\nTESTING: Error: trace Missing file\n");
                return -1;
            }

            //store the string in struct
            commands->trace_path = &arg[8];
            

        }

        //config
        else if (strncmp(arg, "--config=", 9) == 0) {

            //if index 9 -> 10th char is empty or is the null term-> return -1
            if (arg[9] != 't') {
                fprintf(stderr, "\nTESTING: Error: Config missing file\b");
                return -1;
            }

            //store the string in struct
            commands->config_path = &arg[9];

        }

        else {
            return -1;  //for other unrecognized command args
        }

       
    }

    //BB: checks if the mode is bb, then the limit must be > 0, else continue
    if (commands->mode == 1) {
        if (commands->limit == 0) {
            return -1;
        }
    }

    return 0;

   

}

static int validate(traceBB *temp) {

    //OP has to be with 'R' or 'W', else return -1 for malformed operation
    if (temp->OP != 'R' && temp->OP != 'W') {
        return -1;
    }

    //check VA: VA has to be an int
    //loop through VA
    // for (int i = 0; i < )


    return 0;

}

static int validate_VA(const char* Vaddr) {

    //check if VA is only composed of ints
    int len = strlen(Vaddr);    //gets the lenght of the str of VA
    for (int i = 0; i < len; i++) {
        if(!isdigit(Vaddr[i])) {        //if the charecter is not an int (if its a char) return -1
            return -1;
                
        }

    }
    return 0;

}

static int read_BB(FILE *traceFile, parse *commands) {

    //temp struct for storing OP + VA for validation
    traceBB temp = {0};     //initilaize every entry to 0
    
    char buf[500];
    char firstChar;   //store first char
    int lineNum = 0;

    //loop through + read line by line and store each record into temp struct
    while (fgets(buf, sizeof(buf), traceFile)) {

        lineNum++;   //count all entries


        //skip comments (#)
        //space before %c tells sscanf() to skip whitespace before reading the first character
        if (sscanf(buf, " %c", &firstChar) != 1 || firstChar == '#')
        {
            continue;      //skip char
        }
        

        char Vaddr[50];     //temp store va as string for negative + valid check
        char extra;     //store any extra entries

        //first store OP + VA
        //read exactly 2 chars: 1 char + 1 int
        //Read exactly three integers
        int valid_char = sscanf(buf, " %c %s %c", &temp.OP, Vaddr, &extra);  

        //check for any extra entries or empty entry
        if (valid_char != 2 || firstChar == ' ') 
        {
            printf("malformed trace: %s:%d: expected \"OP ADDRESS\"\n", commands->trace_path, lineNum);
            continue;
        }

        
        //call helper func to check for VA validity
        if (validate_VA(Vaddr) == -1) {
            printf("malformed trace: %s:%d: invalid address\n", commands->trace_path, lineNum);
            continue;

        }

        //validate VA, check for negative, call helper
        int result = parse_ndecimal(Vaddr, &temp.VA);
        if (result != 0) {
            continue;  
        }

        //Validate OP + VA
        int check = validate(&temp);

        if (check != 0) {
            printf("malformed trace: %s:%d: invalid operation\n", commands->trace_path, lineNum);
            continue;
        }


        //Increment total valid memory accesses
        temp.accessess++;


        //Check bounds
        if (temp.VA >= commands->limit) {
            temp.faultB++;

            printf("%c %lu -> fault: BOUNDS\n", temp.OP, temp.VA);
        }

        //if VA is valid, calc PA
        else {
            uint64_t PA = commands->base + temp.VA;

            temp.ok++;

            printf("%c %lu  -> PA %lu : ok\n", temp.OP, temp.VA, PA);
        }

        
        
        
    }

    //check for accesses validity
    //if accessess != ok + faults.bounds, print error
    int ttlAcc = temp.ok + temp.faultB;
    if (temp.accessess != ttlAcc ) {
        return -1;
    }
    printf("\nTEST: Accesses validation passed: %d\n", ttlAcc);

    //Print stats
    printf("== stats ==\n");
    printf("accesses = %d, ok = %d, faults.bounds = %d\n",
           temp.accessess, temp.ok, temp.faultB);


    return 0;
}



//passing in the stored commands to open file based on the mode types/etc
static int parse_file(parse *commands){

    FILE *config = NULL;  
    FILE *trace = NULL;

    //if the mode type is seg
    if (commands->mode == 2) {

        printf("\nSegmentation mode to be completed by Milestone 3.\n");

        //open 
        config = fopen(commands->config_path, "r"); 
        
        if (config != NULL) {
            printf("Configuration file opened successfully.\n");

            trace = fopen(commands->trace_path, "r");

            if (trace != NULL) {
                printf("Trace file opened successfully.\n");
            }

        }

        if (config == NULL) {
            fprintf(stderr, "Error: unable to open config file.\n");
            fclose(config);
            return -1;

        }
        
        if (trace == NULL) {
            fprintf(stderr, "Error: unable to open trace file.\n");
            fclose(trace);
            return -1;
        }
    }
    else {  
        trace = fopen(commands->trace_path, "r");
        read_BB(trace, commands);   //call the file reading helper func

        

        if (trace == NULL) {
            fprintf(stderr, "Error: unable to open trace file.\n");
            fclose(trace);
            return -1;
        }

        fclose(trace);



    }
    return 0;


}





int main(int argc, char *argv[]) {

    if (argc < 2) {
        print_usage();
        return EXIT_FAILURE;
    }

    //initialize struct
    parse commands = {0};       //all fields are either 0 or NULL

    //Parse command-line arguments
    int arg = parse_arg(argc, argv, &commands);
    //if return val is error, print error msg
    if (arg == -1) {
        print_usage();
        return EXIT_FAILURE;
    }

    //open + parse file types
    int file = parse_file(&commands);
    //if return is -1, exit 
    if (file == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;



}

    