#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>

fd_set fds;
struct timeval tv_zero = {0, 0};

int main(int argc, char *argv[])
{
    int logs[2];
    logs[0] = open(argv[1], O_RDONLY);
    logs[1] = open(argv[2], O_RDONLY);
    
    float values[2] = {0, 0};
    
    while(1)
    {
        FD_ZERO(&fds);
        FD_SET(logs[0], &fds);
        FD_SET(logs[1], &fds);
        
        if(select(FD_SETSIZE+1, &fds, NULL, NULL, &tv_zero) == -1)
            perror("Selecting file descriptors");
        
        // Check if the pipes have values
        int new_value = 0;
        for(int i=0; i<2; ++i)
            if(FD_ISSET(logs[i], &fds))
            {
                read(logs[i], &values[i], sizeof(float));
                new_value = 1;
            }
        
        // Print the values if new data is available
        if(new_value)
            printf("MotorX: %f | MotorZ: %f \n", values[0], values[1]);
    }
    
    close(logs[0]);
    close(logs[1]);
}
