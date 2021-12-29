#include "utils.h"

void fill_randomly(char* array, int length)
{
    for(int i=0; i<length; ++i)
        array[i] = 65+(random()%25);
}

int get_time_from_file(struct timespec *time, char* file)
{
    int fd_time = open(file, O_RDONLY);
    if(fd_time == -1)
        return -1;
        
    if(read(fd_time, &(time->tv_sec), sizeof(long)) == -1)
        return -1;
    if(read(fd_time, &(time->tv_nsec), sizeof(long)) == -1)
        return -1;
        
    if(close(fd_time) == -1)
        return -1;
}

int write_time_in_file(struct timespec time, char* file)
{
    int fd_time = open(file, O_WRONLY);
    if(fd_time == -1)
        return -1;
        
    if(write(fd_time, &(time.tv_sec), sizeof(long)) == -1)
        return -1;
    if(write(fd_time, &(time.tv_nsec), sizeof(long)) == -1)
        return -1;
        
    if(close(fd_time) == -1)
        return -1;
}
