#include <semaphore.h>  // For semaphores system calls
#include <sys/stat.h>   // For mode constants
#include <sys/mman.h>   // For shared memory system calls
#include <fcntl.h>      // For O_* constants
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <malloc.h>
#include "../libraries/utils.h"
#include "../libraries/logger.h"

void on_error(int signo);
void free_resources();

Logger logger;

int array_length;
char* data = NULL;
void* ptr = NULL;

int main(int argc, char *argv[])
{
    // Creating logger
    logger = (Logger){"consumer_cirbuf", atoi(argv[1])};
    
    // So that process can release resources in case of error
    signal(SIGTERM, on_error);
    
    // Gets the array dimension
    array_length = atoi(argv[2]);
    
    // Getting the circular buffer length
    int buffer_length = atoi(argv[3]);
    
    // Opens the shared memory object and provides the file descriptor
    int shm_fd = shm_open("/cbuffer", O_CREAT | O_RDWR, 0666);
    if(shm_fd == -1)
        perror_exit(&logger, "Opening shared memory");
    
    // Configures the size of the shared memory
    if(ftruncate(shm_fd, array_length) == -1)
        perror_exit(&logger, "Resizing shared memory");
    
    // Maps the shared memory object for reading permissions
    // Returns a void* pointer so there is the need for a cast
    if((ptr = mmap(0, array_length, PROT_READ, MAP_SHARED, shm_fd, 0)) == MAP_FAILED)
        perror_exit(&logger, "Memory mapping");
    
    // Creates semaphore for handling circuilar buffer
    sem_t* sem_not_full = sem_open("buff_not_full", O_CREAT, 0666, buffer_length);
    if(sem_not_full == SEM_FAILED)
        perror_exit(&logger, "Creating buff_not_full semaphore");
        
    sem_t* sem_not_empt = sem_open("buff_not_empty", O_CREAT, 0666, 0);
    if(sem_not_empt == SEM_FAILED)
        perror_exit(&logger, "Creating buff_not_empty semaphore");
        
    sem_t* sem_buffer = sem_open("buffer", O_CREAT, 0666, 1);
    if(sem_buffer == SEM_FAILED)
        perror_exit(&logger, "Creating buffer semaphore");
    
    // Creates data array
    if((data = (char*)malloc(array_length)) == NULL)
        error_exit(&logger, "Creating array with malloc");
    
    int index_out = 0;
    
    int last_percent = 0;
    for(int i=0; i<array_length; ++i)
    {
        // Waits until the buffer is not full anymore
        if(sem_wait(sem_not_empt) == -1)
            perror_exit(&logger, "Waiting on buff_not_empty semaphore");
        
        // Locks the semaphores
        if(sem_wait(sem_buffer) == -1)
            perror_exit(&logger, "Waiting on buffer semaphore");
        
        memcpy(&data[i], ptr+index_out, sizeof(char));
        
        int percent = (int)((float)i/array_length*100);
        if(percent > last_percent && percent%10 == 0)
        {
            printf("%i%%...", percent);
            fflush(stdout);
            last_percent = percent;
        }
        
        index_out = (index_out+sizeof(char))%array_length;
        
        // Unlocks the semaphore
        if(sem_post(sem_buffer) == -1)
            perror_exit(&logger, "Posting on buffer semaphore");
        
        // Signals that the buffer is not empty anymore
        if(sem_post(sem_not_full) == -1)
            perror_exit(&logger, "Posting on buff_not_full semaphore");
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
    
    // Deallocating semaphores
    if(sem_unlink("buff_not_full") == -1 && errno != ENOENT)
        perror_cont(&logger, "Unlinking buff_not_full semaphore");
    if(sem_unlink("buff_not_empty") == -1 && errno != ENOENT)
        perror_cont(&logger, "Unlinking buff_not_empty semaphore");
    if(sem_unlink("buffer") == -1 && errno != ENOENT)
        perror_cont(&logger, "Unlinking buffer semaphore");
    
    // Unmapping shared memory
    if(ptr != NULL)
        munmap(ptr, array_length);
    
    // Deallocating shared memory
    if(shm_unlink("/cbuffer") == -1 && errno != ENOENT)
        perror_cont(&logger, "Unlinking shared memory");
}
