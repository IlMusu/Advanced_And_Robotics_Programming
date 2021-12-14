#include <semaphore.h>  // For semaphores system calls
#include <sys/stat.h>   // For mode constants
#include <sys/mman.h>   // For shared memory system calls
#include <fcntl.h>      // For O_* constants
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libraries/utils.h"

int main(int argc, char *argv[])
{    
    // Gets the array dimension
    int array_length = atoi(argv[1]);
    
    // Opens the shared memory object and provides the file descriptor
    int shm_fd = shm_open("/cbuffer", O_CREAT | O_RDWR, 0666);
    if(shm_fd == -1)
        perror("Opening shared memory");
    
    // Configures the size of the shared memory
    if(ftruncate(shm_fd, array_length) == -1)
        perror("Resizing shared memory");
    
    // Maps the shared memory object for writing operations
    // Returns a void* pointer so there is the need for a cast
    void* ptr = mmap(0, array_length, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(ptr == MAP_FAILED)
        perror("Memory mapping");
        
    // Creates semaphore for handling circuilar buffer
    sem_t* sem_not_full = sem_open("buff_not_full", O_CREAT, 0666, array_length);
    sem_t* sem_not_empt = sem_open("buff_not_empty", O_CREAT, 0666, 0);
    sem_t* sem_buffer = sem_open("buffer", O_CREAT, 0666, 1);
    
    // Creates data array
    char data[array_length];
    fill_randomly(data, array_length);
    
    // Gets the time at the beginning of the transmission
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    write_time_in_file(time, "/tmp/time_1");
    
    int index_in = 0;
    for(int i=0; i<array_length; ++i)
    {
        // Waits until the buffer is not full anymore
        sem_wait(sem_not_full);
        
        // Locks the semaphores
        sem_wait(sem_buffer);
        
        memcpy(ptr+index_in, &data[i], sizeof(char));
        
        printf("Wrote %c on buffer at index %i.\n", data[i], index_in);
        fflush(stdout);
        
        index_in = (index_in+sizeof(char))%array_length;
        
        // Unlocks the semaphore
        sem_post(sem_buffer);
        
        // Signals that the buffer is not empty anymore
        sem_post(sem_not_empt);
    }
    
    // Deallocating semaphores
    sem_unlink("buff_not_full");
    sem_unlink("buff_not_empty");
    sem_unlink("buffer");
    
    // Unmapping shared memory
    munmap(ptr, array_length);
    
    // Deallocating shared memory
    shm_unlink("/cbuffer");
}
