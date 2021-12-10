#include "utils.h"

void fill_randomly(char* array, int length)
{
    for(int i=0; i<length; ++i)
    {
        char value = 65+(random()%25);
        array[i] = value;
    }
}

void get_time_from_file(struct timespec *time, char* file)
{
    int fd_time = open(file, O_RDONLY);
    if(fd_time == -1)
        perror("Opening file for time");
        
    if(read(fd_time, &(time->tv_sec), sizeof(long)) == -1)
        perror("Reading seconds");
    if(read(fd_time, &(time->tv_nsec), sizeof(long)) == -1)
        perror("Reading nanoseconds");  
        
    if(close(fd_time) == -1)
        perror("Closing file for time");
}

void write_time_in_file(struct timespec time, char* file)
{
    int fd_time = open(file, O_WRONLY);
    if(fd_time == -1)
        perror("Opening file for time");
        
    if(write(fd_time, &(time.tv_sec), sizeof(long)) == -1)
        perror("Writing seconds");
    if(write(fd_time, &(time.tv_nsec), sizeof(long)) == -1)
        perror("Writing nanoseconds");  
        
    if(close(fd_time) == -1)
        perror("Closing file for time");
}
