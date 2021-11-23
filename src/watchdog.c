#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define WAIT_SECONDS 60

void receive_signal(int signo);

int received_activity = 0;

int main(int argc, char *argv[])
{
    int pid_mx, pid_mz;
    // Creating named pipe for comunication with master
    // This is used to get motor processes id
    int fd_mas = open(argv[1], O_RDONLY);
    read(fd_mas, &pid_mx, sizeof(int));
    read(fd_mas, &pid_mz, sizeof(int));
    close(fd_mas);
    
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
        printf("Watchdog: No activity in %i seconds, resetting.\n", WAIT_SECONDS);
        fflush(stdout);
        
        kill(pid_mx, SIGUSR2);
        kill(pid_mz, SIGUSR2);
    }
}

void receive_signal(int signo)
{
    if(signo == SIGUSR1)
    {
        received_activity = 1;
    }
}
