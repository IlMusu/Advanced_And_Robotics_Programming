#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <malloc.h>
#include "../libraries/utils.h"

int main(int argc, char *argv[])
{
    // Gets the two file descriptors 
    int pipe_fd_rd = atoi(argv[1]);
    int pipe_fd_wr = atoi(argv[2]);
    
    // Closing unused pipe end
    if(close(pipe_fd_rd) == -1)
        perror("Closing unused fd for reading");
    
    // Gets the array dimension
    int array_length = atoi(argv[3]);
    
    // Creates data array
    char* data = (char*)malloc(array_length);
    if(data == NULL)
        printf("Not enough memory");
    fill_randomly(data, array_length);
    
    // Gets the time at the beginning of the transmission
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    write_time_in_file(time, "/tmp/time_1");
    
    for(int i=0; i<array_length; ++i)
    {
        if(write(pipe_fd_wr, &data[i], sizeof(char)) == -1)
            perror("Writing on pipe");
    }
    
    // Freeing memory used for array
    free(data);
}

