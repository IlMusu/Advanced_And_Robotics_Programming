#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "libraries/utils.h"

int main(int argc, char *argv[])
{
    int fd_napipe = open("/tmp/napipe", O_WRONLY);
    if(fd_napipe == -1)
        perror("Opening named pipe");
        
    // Gets the array dimension
    int array_length = atoi(argv[1]);
    
    // Creates data array
    char data[array_length];
    fill_randomly(data, array_length);
    
    // Gets the time at the beginning of the transmission
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    write_time_in_file(time, "/tmp/time_1");
    
    for(int i=0; i<array_length; ++i)
    {
        if(write(fd_napipe, &data[i], sizeof(char)) == -1)
            perror("Writing on pipe");
        else
        {
            printf("Writing %c on buffer\n", data[i]);
            fflush(stdout);
        }
    }
    
    if(close(fd_napipe) == -1)
        perror("Closing named pipe");
}

