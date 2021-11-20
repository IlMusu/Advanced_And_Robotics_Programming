#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>

#define MOVE_FORWARD 1
#define MOVE_BACKWARD -1
#define STOP 2

int readCommand();
void executeCommand();

int command = STOP;
float current_position = 0.0;
float estimated_position = 0.0;
float step = 0.01F;

fd_set fds;
struct timeval tv_zero = {0, 0};

int main(int argc, char *argv[])
{
    // Opens named pipes for comunication
    int fd_cmd = open(argv[1], O_RDONLY);
    int fd_log = open(argv[2], O_WRONLY);
    
    while(1)
    {
        if(readCommand(fd_cmd) != -1)
            executeCommand(fd_log);
        else
        {
            perror("Could not read new command");
            fflush(stdout);
        }
    
        printf("Current position is %f.\n", current_position);
        
        // Sleep for 100 ms
        usleep(100000);
    }
    
    // Closes named pipe
    close(fd_cmd);
    close(fd_log);
}

int readCommand(int fd)
{
    // Return value used to check if an error happened
    // and if an new command has beed read or not
    int value = 0;
    
    // Use of select() to make non-blocking reads operation
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    int ret = select(FD_SETSIZE+1, &fds, NULL, NULL, &tv_zero);
    
    switch(ret)
    {
    case -1:
        // Some error occurred
        value = -1;
        break;
    case 0:
        // File is not available
        break;
    default:
        // File is available for reading
        value = read(fd, &command, sizeof(int));
        break;
    }
    
    return value;
}

void executeCommand(int fd_log)
{
    // Move procedure
    if((command == MOVE_FORWARD  && current_position < 1) || 
       (command == MOVE_BACKWARD && current_position > 0) )
    {
        // Calcutes new real position
        current_position += command*step;
        if(current_position > 1)
            current_position = 1;
        else if(current_position < 0)
            current_position = 0;
            
        // Calculates estimated position
        estimated_position = current_position + ((float)(random() % 100))/10000;
        
        // Writes estimated position on log file
        write(fd_log, &estimated_position, sizeof(float));
    }
}

