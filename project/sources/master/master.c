#include <stdlib.h> 
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../libraries/utils.h"

long transfer(char* type, char** args);
pid_t spawn(char** program_args);
int get_int_from_console(int bound0, int bound1);
void get_time_from_file(struct timespec *time, char* file);
long calculate_delta(const struct timespec time_1, const struct timespec time_2);

int main(int argc, char *argv[])
{
    srand(time(NULL));
    
    printf("################ MASTER ################\n");
    printf("Choose transmission type:\n");
    printf("1 : unnamed pipe\n");
    printf("2 : named pipe\n");
    printf("3 : socket\n");
    printf("4 : shared memory with circular buffer\n");
    fflush(stdout);
    
    // Gets type of interprocess comunication
    int type = get_int_from_console(1, 4);
    
    printf("Prompt array length [1MB - 100MB]:\n");
    fflush(stdout);
    
    // Gets buffer length (converting from MB to B)
    int length = get_int_from_console(1, 100)*1024*1024;
    char slength[10];
    sprintf(slength, "%i", length);
    
    long time_delta;
    
    switch(type)
    {
        case 1:
        {
            // Opens unnamed pipe 
            int fd[2];
            if(pipe(fd) == -1)
                perror("Creating unnamed pipe");
            
            // Converts pipe fds into strings    
            char fd_0[10]; sprintf(fd_0, "%i", fd[0]);
            char fd_1[10]; sprintf(fd_1, "%i", fd[1]);
            
            time_delta = transfer("unpipe", (char*[]){ (char*)NULL, fd_0, fd_1, slength, (char*)NULL });
            
            // Closes unnamed pipe
            if(close(fd[0]) == -1)
                perror("Closing unnamed pipe fd[0]");    
            if(close(fd[1]) == -1)
                perror("Closing unnamed pipe fd[1]");
            
            break;
        }
        case 2:
        {
            // Creates named pipe
            char *fl_napipe = "/tmp/napipe";
            if(mkfifo(fl_napipe, 0666) == -1)
                perror("Creating named pipe");
                
            time_delta = transfer("napipe", (char*[]){ (char*)NULL, slength, (char*)NULL });
            
            // Releases named pipe
            if(unlink(fl_napipe) == -1)
                perror("Unlinking named pipe");
        
            break;
        }
        case 3:
        {   
            time_delta = transfer("socket", (char*[]){ (char*)NULL, slength, (char*)NULL });
            break;
        }
        case 4: 
        {
            // Gets circular buffer length
            printf("Prompt buffer length [1KB - 100KB]:\n");
            fflush(stdout);
            int buffer = get_int_from_console(1, 100)*1024;
            char sbuffer[6];
            sprintf(sbuffer, "%i", buffer);
            
            time_delta = transfer("cirbuf", (char*[]){ (char*)NULL, slength, sbuffer, (char*)NULL });
            break;
        }
    }
    
    printf("Time for transfering data is: %lu ns (%f s) \n", time_delta, time_delta/1000000000.0F);
    fflush(stdout);
    
    return 0;
}

long transfer(char* type, char** args)
{
    // Creates named pipes to retrieve comunication time
    char *fl_time_1 = "/tmp/time_1";
    char *fl_time_2 = "/tmp/time_2";
    
    if(mkfifo(fl_time_1, 0666) == -1)
        perror("Creating named pipe for time_1");
    if(mkfifo(fl_time_2, 0666) == -1)
        perror("Creating named pipe for time_2");
        
    // Spawning processes based on the type
    char consumer[22] = "./exe/consumer_";
    char producer[22] = "./exe/producer_";
    
    strcat(consumer, type);
    strcat(producer, type);
    
    args[0] = consumer;
    int pid_consumer = spawn(args);
    printf("PID for consumer is %i.\n", pid_consumer);
    
    args[0] = producer;
    int pid_producer = spawn(args);
    printf("PID for producer is %i.\n", pid_producer);
    
    printf("Transfering...");
    fflush(stdout);
    
    // Getting the time from the processes
    struct timespec time_1, time_2;
    
    get_time_from_file(&time_1, fl_time_1);
    get_time_from_file(&time_2, fl_time_2);
    
    printf("Finished.\n");
    fflush(stdout);
    
    // Releases named pipes used to retrieve comunication time
    if(unlink(fl_time_1) == -1)
        perror("Unlinking file for time_1");
    if(unlink(fl_time_2) == -1)
        perror("Unlinking file for time_2");
    
    // Returning the delta time
    return calculate_delta(time_1, time_2);
}

pid_t spawn(char** program_args) 
{
    pid_t child_pid = fork();
    
    if(child_pid != 0)
        return child_pid;
     
    // First arg is the the name of the program 
    execvp(program_args[0], program_args);
    
    // System call 'execvp' only returns if an error has occurred
    perror("Could not create child process");
    return -1;
}

int get_int_from_console(int bound0, int bound1)
{
    int value;
    
    char line[80];
    int valid = 0;
    while(!valid)
    {
        scanf("%s", line);
        value = atoi(line);
        if(value < bound0 || value > bound1)
        {
            printf("Invalid input!\n");
            fflush(stdout);
        }
        else
            valid = 1;
    }
    
    return value;
}

long calculate_delta(const struct timespec time_1, const struct timespec time_2)
{
    return 1000000000*(time_2.tv_sec-time_1.tv_sec)+(time_2.tv_nsec-time_1.tv_nsec);
}
