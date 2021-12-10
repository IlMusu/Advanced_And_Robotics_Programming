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
    
    //TODO close other file desc
    
    // Gets the array dimension
    int array_length = atoi(argv[3]);
    
    // Creates data array
    char data[array_length];
    
    for(int i=0; i<array_length; ++i)
    {
        if(read(pipe_fd_rd, &data[i], sizeof(char)) == -1)
            perror("Reading from pipe");
        else
        {
            printf("Reading %c from buffer\n", data[i]);
            fflush(stdout);
        }
    }
    
    // Gets the time at the end of the transmission
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    write_time_in_file(time, "/tmp/time_2");
}
