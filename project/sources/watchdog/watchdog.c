#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "../libraries/logger.c"

#define WAIT_SECONDS 60

void receive_signal(int signo);

int received_activity = 0;

int main(int argc, char *argv[])
{
    // Creating logger
    Logger logger = {"Watchdog", atoi(argv[1])};
    
    // Getting motors pid from master process
    int pid_mx, pid_mz;
    
    int fd_mas = open(argv[2], O_RDONLY);
    if(fd_mas == -1)
        error_exit(&logger, "Opening pipe with master");
        
    if(read(fd_mas, &pid_mx, sizeof(int)) == -1)
        error_exit(&logger, "Reading motor_x pid from pipe");
        
    if(read(fd_mas, &pid_mz, sizeof(int)) == -1)
        error_exit(&logger, "Reading motor_z pid from pipe");
        
    if(close(fd_mas) == -1)
        error_exit(&logger, "Closing pipe with master");
    
    // Initializing signal handler
    signal(SIGUSR1, receive_signal);
    
    while(1)
    {
        // If a signal is received, sleep() is interrupted
        // so watchdog waits another WAIT seconds
        do
        {
            received_activity = 0;
            sleep(WAIT_SECONDS);
        }while(received_activity == 1);
        
        // If arrived here, no activity was detected
        info(&logger, "No activity in time limit, resetting motors");
        
        if(kill(pid_mx, SIGUSR2) == -1)
            error_exit(&logger, "Sending RESET signal to motor_x");
        if(kill(pid_mz, SIGUSR2) == -1)
            error_exit(&logger, "Sending RESET signal to motor_z");
    }
    
    return 0;
}

void receive_signal(int signo)
{
    if(signo == SIGUSR1)
    {
        received_activity = 1;
    }
}
