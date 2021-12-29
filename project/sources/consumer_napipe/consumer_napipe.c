#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <malloc.h>
#include "../libraries/utils.h"
#include "../libraries/logger.h"

void on_error(int signo);
void free_resources();

Logger logger;

int fd_napipe;
char* data = NULL;

int main(int argc, char *argv[])
{
    // Creating logger
    logger = (Logger){"consumer_napipe", atoi(argv[1])};
    
    // So that process can release resources in case of error
    signal(SIGTERM, on_error);
    
    // Opening named pipe
    if((fd_napipe = open("/tmp/napipe", O_RDONLY)) == -1)
        perror_exit(&logger, "Opening named pipe");
    
    // Gets the array dimension
    int array_length = atoi(argv[2]);
    
    // Creates data array
    if((data = (char*)malloc(array_length)) == NULL)
        error_exit(&logger, "Creating array with malloc");
    
    int last_percent = 0;
    for(int i=0; i<array_length; ++i)
    {
        if(read(fd_napipe, &data[i], sizeof(char)) == -1)
            perror_exit(&logger, "Reading from pipe");
            
        int percent = (int)((float)i/array_length*100);
        if(percent > last_percent && percent%10 == 0)
        {
            printf("%i%%...", percent);
            fflush(stdout);
            last_percent = percent;
        } 
    }
    
    // Gets the time at the end of the transmission
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    if(write_time_in_file(time, "/tmp/time_2") == -1)
        perror_exit(&logger, "Comunicating time to master");
    
    // Freeing resources on termination
    free_resources();
    
    return 0;
}

void on_error(int signo)
{
    if(signo == SIGTERM)
    {
        free_resources();
        exit(1);
    }
}

void free_resources()
{
    // Freeing memory used for array
    if(data != NULL)
        free(data);
        
    // Closing named pipe
    if(close(fd_napipe) == -1)
        perror_cont(&logger, "Closing named pipe");
}
