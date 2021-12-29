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

char* data = NULL;

int main(int argc, char *argv[])
{
    // Creating logger
    Logger logger = {"consumer_unpipe", atoi(argv[1])};
    
    // So that process can release resources in case of error
    signal(SIGTERM, on_error);
    
    // Gets the two file descriptors 
    int pipe_fd_rd = atoi(argv[2]);
    int pipe_fd_wr = atoi(argv[3]);
    
    // Closing unused pipe end
    if(close(pipe_fd_wr) == -1)
        perror_exit(&logger, "Closing unused pipe end");
    
    // Gets the array dimension
    int array_length = atoi(argv[4]);
    
    // Creates data array
    char* data = (char*)malloc(array_length);
    if(data == NULL)
        error_exit(&logger, "Creating array with malloc");
    
    int last_percent = 0;
    for(int i=0; i<array_length; ++i)
    {
        if(read(pipe_fd_rd, &data[i], sizeof(char)) == -1)
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
}
