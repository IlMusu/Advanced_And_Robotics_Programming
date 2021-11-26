#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <signal.h>
#include "Libraries/logger.c"

#define MOVE_FORWARD 1
#define MOVE_BACKWARD -1
#define STOP 2
#define RESET 3

void read_command();
void execute_command();
void receive_signal(int signo);

Logger logger;

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
    // Creating logger
    logger = (Logger){"Motor", atoi(argv[1])};
       
    // Getting watchdog process id
    pid_wd = atoi(argv[2]);
    
    // Opens named pipes for comunication
    int fd_cmd = open(argv[3], O_RDONLY);
    if(fd_cmd == -1)
        error_exit(&logger, "Opening pipe with command_console");
        
    int fd_ins = open(argv[4], O_WRONLY);
    if(fd_ins == -1)
        error_exit(&logger, "Opening pipe with inspection_console");
    
    // Inits the signal handler
    signal(SIGUSR1, receive_signal);
    signal(SIGUSR2, receive_signal);
    
    // Writes initial positions on log file
    write(fd_ins, &estimated_position, sizeof(float));
    
    while(1)
    {
        read_command(fd_cmd);
        execute_command(fd_ins);
        
        // Sleep for 10000 ms
        usleep(1000000);
    }
    
    // Closes named pipe
    if(close(fd_cmd) == -1)
        error_exit(&logger, "Closing pipe with command_console");
    if(close(fd_ins) == -1)
        error_exit(&logger, "Closing pipe with inspection_console");
        
    return 0;
}

void read_command(int fd_cmd)
{
    // Caching previous command
    int prev_command = command;
    
    // Use of select() to make non-blocking reads operation
    FD_ZERO(&fds);
    FD_SET(fd_cmd, &fds);
    
    if(select(FD_SETSIZE+1, &fds, NULL, NULL, &tv_zero) == -1)
        error_exit(&logger, "Selecting file descriptors");
        
    // If pipe is available for reading
    if(FD_ISSET(fd_cmd, &fds))
    {
        if(read(fd_cmd, &command, sizeof(int)) == -1)
            error_exit(&logger, "Reading command from pipe");
        
        // RESET command cannot be interrupted by command_console
        // so we just ignore the new command
        if(prev_command == RESET)
            command = prev_command;
            
        is_emergency = 0;
    }
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
            error_exit(&logger, "Sending update signal to watchdog");
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

