#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "libraries/utils.h"

int main(int argc, char *argv[])
{
    // Gets the two file descriptors 
    int pipe_fd_rd = atoi(argv[1]);
    int pipe_fd_wr = atoi(argv[2]);
    
    // Gets the array dimension
    int array_length = atoi(argv[3]);
    
    // Creates data array
    char data[array_length];
    fill_randomly(data, array_length);
    
    // Gets the time at the beginning of the transmission
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    write_time_in_file(time, "/tmp/time_1");
    
    for(int i=0; i<array_length; ++i)
    {
        if(write(pipe_fd_wr, &data[i], sizeof(char)) == -1)
            perror("Writing on pipe");
        else
        {
            printf("Writing %c on buffer\n", data[i]);
            fflush(stdout);
        }
    }
}

