#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void receive_signal(int signo);

int received_activity = 0;

int main(int argc, char *argv[])
{
    //int pid_mx = atoi(argv[1]);
    //int pid_mz = atoi(argv[2]);
    
    printf("Watchdog initialized\n");
    fflush(stdout);
    
    signal(SIGUSR1, receive_signal);
    
    while(1)
    {
        printf("Waiting 60 seconds...\n");
        fflush(stdout);
        
        do
        {
            received_activity = 0;
            sleep(10);
        }while(received_activity == 1);
        
        printf("No activity detected\n");
        fflush(stdout);
        //kill(pid_mx, SIGUSR2);
        //kill(pid_mz, SIGUSR2);
    }
}

void receive_signal(int signo)
{
    if(signo == SIGUSR1)
    {
        received_activity = 1;
        printf("Watchdog received signal.\n");
        fflush(stdout);
    }
}
