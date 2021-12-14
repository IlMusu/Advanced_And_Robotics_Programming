#include <stdlib.h> 
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "libraries/utils.h"

pid_t spawn(char** program_args);
int get_int_from_console(int bound0, int bound1);
void get_time_from_file(struct timespec *time, char* file);
long calculate_diff(const struct timespec time_1, const struct timespec time_2);

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
    
    printf("Prompt array length [1000B - 1000000B]:\n");
    fflush(stdout);
    
    // Gets buffer length
    int length = get_int_from_console(10, 1000000);
    char slength[10];
    sprintf(slength, "%i", length);
    
    // Creates named pipes to retrieve comunication time
    char *fl_time_1 = "/tmp/time_1";
    char *fl_time_2 = "/tmp/time_2";
    
    if(mkfifo(fl_time_1, 0666) == -1)
        perror("Creating named pipe for time_1");
    if(mkfifo(fl_time_2, 0666) == -1)
        perror("Creating named pipe for time_2");
    
    struct timespec time_1, time_2;
    
    switch(type)
    {
        case 1:
        {
            int fd[2];
            if(pipe(fd) == -1)
                perror("Could not create pipe");
                
            char fd_0[10];
            char fd_1[10];
            sprintf(fd_0, "%i", fd[0]);
            sprintf(fd_1, "%i", fd[1]);
            
            spawn((char*[]){ "./exe/producer_unpipe", fd_0, fd_1, slength, (char*)NULL });
            spawn((char*[]){ "./exe/consumer_unpipe", fd_0, fd_1, slength, (char*)NULL });
            
            get_time_from_file(&time_1, fl_time_1);
            get_time_from_file(&time_2, fl_time_2);
            
            wait(NULL);
            wait(NULL);
            
            close(fd[0]);       
            close(fd[1]);
            
            break;
        }
        case 2:
        {
            // Creates named pipe for interprocess comunication
            char *fl_napipe = "/tmp/napipe";
            if(mkfifo(fl_napipe, 0666) == -1)
                perror("Creating named pipe");
                
            // Creates processes    
            spawn((char*[]){ "./exe/producer_napipe", slength, (char*)NULL });
            spawn((char*[]){ "./exe/consumer_napipe", slength, (char*)NULL });
            
            get_time_from_file(&time_1, fl_time_1);
            get_time_from_file(&time_2, fl_time_2);
            
            wait(NULL);
            wait(NULL);
            
            if(unlink(fl_napipe) == -1)
                perror("Unlinking named pipe");
        
            break;
        }
        case 3:
        {
            break;
        }
        case 4: 
        {
            spawn((char*[]){ "./exe/producer_cirbuf", slength, (char*)NULL });
            spawn((char*[]){ "./exe/consumer_cirbuf", slength, (char*)NULL });
            
            get_time_from_file(&time_1, fl_time_1);
            get_time_from_file(&time_2, fl_time_2);
            
            wait(NULL);
            wait(NULL);
            
            break;
        }
    }
    
    long time_diff = calculate_diff(time_1, time_2);
        
    printf("Child processes terminated execution.\n");
    printf("Time for transfering data is: %lu\n", time_diff);
    fflush(stdout);
    
    if(unlink(fl_time_1) == -1)
        perror("Unlinking file for time_1");
    if(unlink(fl_time_2) == -1)
        perror("Unlinking file for time_2");
    
    return 0;
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

long calculate_diff(const struct timespec time_1, const struct timespec time_2)
{
    return 1000000000*(time_2.tv_sec-time_1.tv_sec)+(time_2.tv_nsec-time_1.tv_nsec);
}
