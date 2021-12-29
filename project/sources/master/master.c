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
#include "../libraries/logger.h"

void on_child_terminated(int signo);
void on_error(int signo);
void free_resources();

pid_t spawn(char** program_args);
int get_int_from_console(int bound0, int bound1);
long transfer(char* type, char** args);
long calculate_delta(const struct timespec time_1, const struct timespec time_2);

Logger logger;

// The child pids
pid_t pid_consumer = -1;
pid_t pid_producer = -1;

// All the allocated resources that need to be freed at the end
int fd_log = -1;
int fd[2] = {-1, -1};
char *fl_time_1 = "/tmp/time_1";
char *fl_time_2 = "/tmp/time_2";
char *fl_napipe = "/tmp/napipe";

int main(int argc, char *argv[])
{
    // Creating log file
    fd_log = creat("./sources/log/log.txt", 0666);
    if(fd_log == -1)
    {
        perror("Creating log file");
        exit(-1);
    }
    
    // Converting log fd into string
    char sfd_log[10]; sprintf(sfd_log, "%i", fd_log);
    
    // Creating logger
    logger = (Logger){"master", fd_log};
    
    // So that process can release resources in case of error
    signal(SIGTERM, on_error);
    // So that process can termite other children in case of error
    signal(SIGCHLD, on_child_terminated);
    
    // Initializing random
    srand(time(NULL));
    
    // Cleaning pipes if already existing
    if(unlink(fl_time_1) == -1 && errno != ENOENT)
        perror_exit(&logger, "Cleaning named pipe for time_1");
    if(unlink(fl_time_2) == -1 && errno != ENOENT)
        perror_exit(&logger, "Cleaning named pipe for time_2");
        
    // Creating named pipes to retrieve comunication time    
    if(mkfifo(fl_time_1, 0666) == -1)
        perror_exit(&logger, "Creating named pipe for time_1");
    if(mkfifo(fl_time_2, 0666) == -1 && errno != EEXIST)
        perror_exit(&logger, "Creating named pipe for time_2");
    
    printf("################ MASTER ################\n");
    printf("Choose transmission type:\n");
    printf("1 : unnamed pipe\n");
    printf("2 : named pipe\n");
    printf("3 : socket\n");
    printf("4 : shared memory with circular buffer\n");
    fflush(stdout);
    
    // Getting type of interprocess comunication
    int type = get_int_from_console(1, 4);
    
    printf("Prompt array length [1MB - 100MB]:\n");
    fflush(stdout);
    
    // Getting buffer length (converting from MB to B)
    int length = get_int_from_console(1, 100)*1024*1024;
    char slength[10]; sprintf(slength, "%i", length);
        
    long time_delta;
    
    switch(type)
    {
        case 1:
        {
            // Opening unnamed pipe
            if(pipe(fd) == -1)
                perror_exit(&logger, "Creating unnamed pipe");
            
            // Converting pipe fds into strings    
            char fd_0[10]; sprintf(fd_0, "%i", fd[0]);
            char fd_1[10]; sprintf(fd_1, "%i", fd[1]);
            
            time_delta = transfer("unpipe", (char*[]){ (char*)NULL, sfd_log, fd_0, fd_1, slength, (char*)NULL });
            break;
        }
        case 2:
        {
            // Cleaning pipe if already existing
            if(unlink(fl_napipe) == -1 && errno != ENOENT)
                perror_exit(&logger, "Cleaning named pipe");
            // Creating named pipe
            if(mkfifo(fl_napipe, 0666) == -1 && errno != EEXIST)
                perror_exit(&logger, "Creating named pipe");
                
            time_delta = transfer("napipe", (char*[]){ (char*)NULL, sfd_log, slength, (char*)NULL });
            break;
        }
        case 3:
        {   
            time_delta = transfer("socket", (char*[]){ (char*)NULL, sfd_log, slength, (char*)NULL });
            break;
        }
        case 4: 
        {
            // Getting circular buffer length
            printf("Prompt buffer length [1KB - 100KB]:\n");
            fflush(stdout);
            
            int buffer = get_int_from_console(1, 100)*1024;
            char sbuffer[6]; sprintf(sbuffer, "%i", buffer);
            
            time_delta = transfer("cirbuf", (char*[]){ (char*)NULL, sfd_log, slength, sbuffer, (char*)NULL });
            break;
        }
    }
    
    // Freeing all allocated resources
    free_resources();
    
    printf("Time for transfering data is: %lu ns (%f s) \n", time_delta, time_delta/1000000000.0F);
    fflush(stdout);
    
    return 0;
}

void free_resources()
{
    // Terminating child processes if still alive    
    if(pid_consumer != -1 && kill(pid_consumer, SIGTERM) == -1 && errno != ESRCH)
        perror_cont(&logger, "Killing command_console process");
    if(pid_producer != -1 && kill(pid_producer, SIGTERM) == -1 && errno != ESRCH)
        perror_cont(&logger, "Killing inspection_console process");
        
    // Unlinking named pipes used for getting delta time
    if(unlink(fl_time_1) == -1 && errno != ENOENT)
        perror_cont(&logger, "Unlinking file for time_1");
    if(unlink(fl_time_2) == -1 && errno != ENOENT)
        perror_cont(&logger, "Unlinking file for time_2");
        
    // Closing unnamed pipes
    if(fd[0] != -1 && close(fd[0]) == -1)
        perror_cont(&logger, "Closing unnamed pipe fd[0]");    
    if(fd[1] != -1 && close(fd[1]) == -1)
        perror_cont(&logger, "Closing unnamed pipe fd[1]");
        
    // Releasing named pipe
    if(unlink(fl_napipe) == -1 && errno != ENOENT)
        perror_cont(&logger, "Unlinking named pipe");
        
    // Closing log file descriptor
    if(close(fd_log) == -1)
        perror("Closing log file");
}

void on_error(int signo)
{
    if(signo == SIGTERM)
    {
        free_resources();
        exit(1);
    }
}

void on_child_terminated(int signo)
{
    if(signo == SIGCHLD)
    {
        int wstatus;
        int pid = wait(&wstatus);
        
        if (!WIFEXITED(wstatus))
            return;
            
        int status = WEXITSTATUS(wstatus);
        
        if(status == 0)
            return;
            
        printf("\n\n");
        printf("Child with pid %i has been terminated with status %i.\n", pid, status);
        printf("Check log file for more details.\n");
        free_resources();
        exit(0);
    }
}

long transfer(char* type, char** args)
{
    // Spawning processes based on the type
    char consumer[22] = "./exes/consumer_";
    char producer[22] = "./exes/producer_";
    
    strcat(consumer, type);
    strcat(producer, type);
    
    args[0] = consumer;
    pid_consumer = spawn(args);
    printf("Consumer PID is %i.\n", pid_consumer);
    
    args[0] = producer;
    pid_producer = spawn(args);
    printf("Producer PID is %i.\n", pid_producer);
    
    printf("Transfering...");
    fflush(stdout);
    
    // Getting the time from the processes
    struct timespec time_1, time_2;
    
    if(get_time_from_file(&time_1, fl_time_1) == -1)
        perror_exit(&logger, "Getting time from file 2");
    if(get_time_from_file(&time_2, fl_time_2) == -1)
        perror_exit(&logger, "Getting time from file 2");
    
    printf("Finished.\n");
    fflush(stdout);
    
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
    perror_exit(&logger, "Could not create child process");
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
