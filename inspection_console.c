#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>
#include <signal.h>

int pid_mx, pid_mz;

fd_set fds;
struct timeval tv_zero = {0, 0};

void print_commands_info(void);
void scan_and_execute_command(void);

int main(int argc, char *argv[])
{
    pid_mx = atoi(argv[1]);
    pid_mz = atoi(argv[2]);
    
    int logs[2];
    logs[0] = open(argv[3], O_RDONLY);
    logs[1] = open(argv[4], O_RDONLY);
    
    float values[2] = {0, 0};
    
    print_commands_info();
    
    while(1)
    {
        FD_ZERO(&fds);
        FD_SET(logs[0], &fds);
        FD_SET(logs[1], &fds);
        FD_SET(fileno(stdin), &fds);
        
        if(select(FD_SETSIZE+1, &fds, NULL, NULL, &tv_zero) == -1)
            perror("Selecting file descriptors");
            
        // Checks if a new command is avaible
        if(FD_ISSET(fileno(stdin), &fds))
            scan_and_execute_command();
        
        // Checks if the pipes have values
        int new_value = 0;
        for(int i=0; i<2; ++i)
            if(FD_ISSET(logs[i], &fds))
            {
                read(logs[i], &values[i], sizeof(float));
                new_value = 1;
            }
        
        // Print the values if new data is available
        if(new_value)
        {
            // TODO maybe send command to watchdog
            printf("MotorX: %f | MotorZ: %f \n", values[0], values[1]);
        }
    }
    
    close(logs[0]);
    close(logs[1]);
}

void print_commands_info(void)
{
    printf("- These are the commands to control the hoist:\n");
    printf(" s : stop the hoist until next command\n");
    printf(" r : moves the hoist to initial position\n");
    fflush(stdout);
}

void scan_and_execute_command(void)
{
    char command[10];
    scanf("%s", command);
    
    //TODO send command to watchdog
    
    if(strcmp(command, "s") == 0)
    {
        printf("Sending stop signal.\n");
        fflush(stdout);
        kill(pid_mx, SIGUSR1);
        kill(pid_mz, SIGUSR1);
    }
    else if(strcmp(command, "r") == 0)
    {
        printf("Sending reset signal.\n");
        fflush(stdout);
        kill(pid_mx, SIGUSR2);
        kill(pid_mz, SIGUSR2);
    }
    else
    {
        printf("Invalid command.\n");
        fflush(stdout);
    }
}
