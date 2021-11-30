#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>
#include <signal.h>
#include "../libraries/logger.c"

Logger logger;

int pid_mx, pid_mz, pid_wd;

fd_set fds;
struct timeval tv_zero = {0, 0};

void print_commands_info(void);
void scan_and_execute_command(void);

int main(int argc, char *argv[])
{    
    printf("################ INSPECTION CONSOLE ################\n");
    
    // Creating logger
    logger = (Logger){"InspectionConsole", atoi(argv[1])};
    
    // Getting watchdog process id
    pid_wd = atoi(argv[2]);
    
    // Opening named pipes for comunication
    int logs[2];
    logs[0] = open(argv[3], O_RDONLY);
    if(logs[0] == -1)
        error_exit(&logger, "Could not open pipe with motor_x");
        
    logs[1] = open(argv[4], O_RDONLY);
    if(logs[1] == -1)
        error_exit(&logger, "Could not open pipe with motor_z");
        
    // Getting processes id
    pid_mx = atoi(argv[5]);
    pid_mz = atoi(argv[6]);
    
    float values[2] = {0, 0};
    
    print_commands_info();
    
    while(1)
    {
        FD_ZERO(&fds);
        FD_SET(logs[0], &fds);
        FD_SET(logs[1], &fds);
        FD_SET(fileno(stdin), &fds);
        
        if(select(FD_SETSIZE+1, &fds, NULL, NULL, &tv_zero) == -1)
            error_exit(&logger, "Selecting file descriptors");
            
        // Checks if a new command is avaible
        if(FD_ISSET(fileno(stdin), &fds))
            scan_and_execute_command();
        
        // Checks if the pipes have values
        int new_value = 0;
        for(int i=0; i<2; ++i)
            if(FD_ISSET(logs[i], &fds))
            {
                if(read(logs[i], &values[i], sizeof(float)) == -1)
                    error_exit(&logger, "Reading value from console");
                new_value = 1;
            }
        
        // Print the values if new data is available
        if(new_value)
            printf("MotorX: %f | MotorZ: %f \n", values[0], values[1]);
    }
    
    if(close(logs[0]) == -1)
        error_exit(&logger, "Closing pipe with motor_x");
        
    if(close(logs[1]) == -1)
        error_exit(&logger, "Closing pipe with motor_x");
        
    return 0;
}

void print_commands_info(void)
{
    printf("These are the commands to control the hoist:\n");
    printf(" s : stop the hoist until next command\n");
    printf(" r : moves the hoist to initial position\n");
    fflush(stdout);
}

void scan_and_execute_command(void)
{
    char command[10];
    scanf("%s", command);
    
    // Sending activity signal to watchdog
    if(kill(pid_wd, SIGUSR1) == -1)
        error_exit(&logger, "Sending update signal to watchdog");
    
    int is_s = strcmp(command, "s");
    int is_r = strcmp(command, "r");
    
    if(is_s != 0 && is_r != 0)
    {
        printf("Invalid command.\n");
        fflush(stdout);
    }
    
    if(strcmp(command, "s") == 0)
    {
        printf("Sending stop signal.\n");
        fflush(stdout);
        if(kill(pid_mx, SIGUSR1) == -1)
            error_exit(&logger, "Sending STOP signal to motor_x");
        if(kill(pid_mz, SIGUSR1) == -1)
            error_exit(&logger, "Sending STOP signal to motor_z");
    }
    else if(strcmp(command, "r") == 0)
    {
        printf("Sending reset signal.\n");
        fflush(stdout);
        if(kill(pid_mx, SIGUSR2) == -1)
            error_exit(&logger, "Sending RESET signal to motor_x");
        if(kill(pid_mz, SIGUSR2) == -1)
            error_exit(&logger, "Sending RESET signal to motor_z");
    }
            
    // Log info message
    char text[80];
    strcat(strcat(strcpy(text, "Received valid command ["), command), "]");
    info(&logger, text);
}
