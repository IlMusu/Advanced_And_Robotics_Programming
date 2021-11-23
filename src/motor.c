#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <signal.h>

#define MOVE_FORWARD 1
#define MOVE_BACKWARD -1
#define STOP 2
#define RESET 3

int read_command();
void execute_command();
void receive_signal(int signo);

int command = STOP;
int is_emergency = 0;

float current_position = 0.0;
float estimated_position = 0.0;
float step = 0.05F;

int pid_wd;

fd_set fds;
struct timeval tv_zero = {0, 0};

int main(int argc, char *argv[])
{
    // Opens named pipes for comunication
    int fd_cmd = open(argv[1], O_RDONLY);
    int fd_log = open(argv[2], O_WRONLY);
    // Getting watchdog process id
    pid_wd = atoi(argv[3]);
    
    // Inits the signal handler
    signal(SIGUSR1, receive_signal);
    signal(SIGUSR2, receive_signal);
    
    // Writes initial positions on log file
    write(fd_log, &estimated_position, sizeof(float));
    
    while(1)
    {
        if(read_command(fd_cmd) != -1)
            execute_command(fd_log);
        else
        {
            perror("Could not read new command");
            fflush(stdout);
        }
        
        // Sleep for 10000 ms
        usleep(1000000);
    }
    
    // Closes named pipe
    close(fd_cmd);
    close(fd_log);
}

int read_command(int fd)
{
    int prev_command = command;
        
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
        
        // RESET command cannot be interrupted
        if(prev_command == RESET)
            command = prev_command;
            
        is_emergency = 0;
        break;
    }
    
    return value;
}

void execute_command(int fd_log)
{
    int moved = 0;
    
    // Reset procedure
    if(command == RESET && current_position > 0)
    {
        current_position -= step;
        // This marks the end of execution of RESET, command
        // is set to STOP to read instructions again.
        if(current_position <= 0)
        {
            current_position = 0;
            command = STOP;
        }
        
        moved = 1;
    }
    
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
            
        moved = 1;
    }
    
    if(moved != 0)
    {
        // Calculates estimated position
        estimated_position = current_position + ((float)(random() % 100))/10000;
        
        // Writes estimated position on log file
        write(fd_log, &estimated_position, sizeof(float));
        
        // Sending activity signal to watchdog
        if(kill(pid_wd, SIGUSR1) == -1)
            perror("Cannot send signal to watchdog");
    }
}

void receive_signal(int signo)
{
    if(signo == SIGUSR1)
    {
        command = STOP;
        is_emergency = 1;
    }
    else if(signo == SIGUSR2)
    {   
        // Can reset only when is not an emergency
        if(is_emergency != 1)
            command = RESET;
    }
}

