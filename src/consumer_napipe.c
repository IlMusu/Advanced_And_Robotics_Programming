#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "libraries/utils.h"

int main(int argc, char *argv[])
{
    int fd_napipe = open("/tmp/napipe", O_RDONLY);
    if(fd_napipe == -1)
        perror("Opening named pipe");
    
    // Gets the array dimension
    int array_length = atoi(argv[1]);
    
    // Creates data array
    char data[array_length];
    
    for(int i=0; i<array_length; ++i)
    {
        if(read(fd_napipe, &data[i], sizeof(char)) == -1)
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
    
    if(close(fd_napipe) == -1)
        perror("Closing named pipe");
}
