#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
 
// 0: go to initial position
// 2: stop
// 1: move_forward
//-1: move_backward
int command = 2;

float current_position = 0.0;
float estimated_position = 0.0;

float step = 0.01F;

int msleep(long msec);

int main(int argc, char *argv[])
{
    int pipe_fd = atoi(argv[1]);
    
    while(1)
    {
        char temp[10];
        // returns -1 if pipe is empty, hence, no command
        if(read(pipe_fd, temp, sizeof(int)) != -1)
        {
            command = atoi(temp);
            printf("command read %s\n", temp);
        }
        
        if(command == 0)
        {
            // reset procedure
            if(current_position > 0)
            {
                current_position -= step;
                if(current_position < 0)
                    current_position = 0;
            }
        }
        
        else if( (command == 1 && current_position < 1) || 
                 (command == -1 && current_position > 0) )
        {
        
            // move procedure
            current_position += command*step;
            if(current_position > 1)
                current_position = 1;
            else if(current_position < 0)
                current_position = 0;
                
            estimated_position = current_position + ((float)(random() % 100))/10000;
            //TODO send estimate position 
        }
        
        //printf("Current position is %f.\n", current_position);
        
        msleep(100);
    }
}

int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}
