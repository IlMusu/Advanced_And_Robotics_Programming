#include <sys/mman.h>   // For shared memory system calls
#include <semaphore.h>  // For semaphores system calls
#include <sys/stat.h>   // For mode constants
#include <fcntl.h>      // For O_* constants
#include <unistd.h>     
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_buffer(void* buffer, int size);

int main(int argc, char *argv[])
{
    // Reading buffer size from input args
    int buffer_size = atoi(argv[1]);
    
    // Opens the shared memory object and provides the file descriptor
    int shm_fd = shm_open("/cbuffer", O_CREAT | O_RDWR, 0666);
    if(shm_fd == -1)
        perror("Opening shared memory");
    
    // Configures the size of the shared memory
    if(ftruncate(shm_fd, buffer_size) == -1)
        perror("Resizing shared memory");
    
    // Maps the shared memory object for reading permissions
    // Returns a void* pointer so there is the need for a cast
    void* ptr = mmap(0, buffer_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    if(ptr == MAP_FAILED)
        perror("Memory mapping");
    
    // Creates semaphore for handling circuilar buffer
    sem_t* sem_not_full = sem_open("buff_not_full", O_CREAT, 0666, buffer_size);
    sem_t* sem_not_empt = sem_open("buff_not_empty", O_CREAT, 0666, 0);
    sem_t* sem_buffer = sem_open("buffer", O_CREAT, 0666, 1);
    
    int index_out = 0;
    
    for(int i=0; i<15; ++i)
    {
        // Consumer is slow
        usleep(2000*1000);
        
        // Waits until the buffer is not full anymore
        sem_wait(sem_not_empt);
        
        // Locks the semaphores
        sem_wait(sem_buffer);
        
        char value;
        memcpy(&value, ptr+index_out, sizeof(char));
        
        printf("Read %c from buffer at index %i. ", value, index_out);
        print_buffer(ptr, buffer_size);
        printf("\n");
        fflush(stdout);
        
        index_out = (index_out+sizeof(char))%buffer_size;
        
        // Unlocks the semaphore
        sem_post(sem_buffer);
        
        // Signals that the buffer is not empty anymore
        sem_post(sem_not_full);
    }
    
    // Deallocating semaphores
    sem_unlink("buff_not_full");
    sem_unlink("buff_not_empty");
    sem_unlink("buffer");
    
    // Unmapping shared memory
    munmap(ptr, buffer_size);
    
    // Deallocating shared memory
    shm_unlink("/cbuffer");
}

void print_buffer(void* buffer, int size)
{
    char tmp[size+1];
    memcpy(tmp, buffer, sizeof(char)*size);
    tmp[size] = '\0';
    printf("Buffer is %s.", tmp);
}
