#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define MOVE_FORWARD 1
#define MOVE_BACKWARD -1
#define STOP 2

int command = STOP;

float current_position = 0.0;
float estimated_position = 0.0;

float step = 0.01F;

int main(int argc, char *argv[])
{
    int pipe_fd = atoi(argv[1]);
    
    while(1)
    {
        // returns -1 if pipe is empty, hence, no new command
        if(read(pipe_fd, &command, sizeof(int)) != -1)
            printf("New command received: %i. \n", command);
        
        // move procedure
        if((command == MOVE_FORWARD  && current_position < 1) || 
           (command == MOVE_BACKWARD && current_position > 0) )
        {
            current_position += command*step;
            if(current_position > 1)
                current_position = 1;
            else if(current_position < 0)
                current_position = 0;
                
            estimated_position = current_position + ((float)(random() % 100))/10000;
            //TODO send estimate position 
        }
        
        printf("Current position is %f.\n", current_position);
        
        // Sleep for 100 ms
        usleep(100000);
    }
}
