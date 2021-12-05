#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include "../libraries/logger.c"

void print_commands_info(void);
void decodeCommand(char *line, int *fd, int *command);
void receive_signal(int signo);
void on_termination(int signo);

Logger logger;
int fd_x, fd_z;

int main(int argc, char *argv[])
{
    printf("################ COMMAND CONSOLE ################\n");
    
    // Creating logger
    logger = (Logger){"CommandConsole", atoi(argv[1])};
    
    // Getting watchdog process id
    int pid_wd = atoi(argv[2]);
    
    // Opening files
    if((fd_x = open(argv[3], O_WRONLY)) == -1)
        error_exit(&logger, "Opening pipe with motor_x");
    if((fd_z = open(argv[4], O_WRONLY)) == -1)
        error_exit(&logger, "Opening pipe with motor_z");
        
    // Init signal handler for process termination
    signal(SIGTERM, on_termination);
    
    print_commands_info();
    
    while(1)
    {
        printf("Insert command: ");
        fflush(stdout);
        
        // Reads a string from the command line
        char line[80];
        scanf("%s", line);
        
        // Sending activity signal to watchdog
        if(kill(pid_wd, SIGUSR1) == -1)
            error_exit(&logger, "Sending update signal to watchdog");
        
        if(strcmp(line, "help") == 0)
        {
            print_commands_info();
            continue;
        }
        
        int fd = -1;
        int command = -2;
        decodeCommand(line, &fd, &command);
        
        if(fd == -1 || command == -2)
        {
            printf("Invalid command.\n");
            fflush(stdout);
            continue;
        }
        
        // Logs a info message to the log file
        char text[80];
        strcat(strcat(strcpy(text, "Received valid command ["), line), "]");
        info(&logger, text);
        
        // A new command from the command_console is available, so an
        // eventual emergency must be cleared for both motors
        int clear_emergency = 4;
        if(write(fd_x, &clear_emergency, sizeof(int)) == -1)
            error_exit(&logger, "Clearing emergency for motor_x");
        if(write(fd_z, &clear_emergency, sizeof(int)) == -1)
            error_exit(&logger, "Clearing emergency for motor_z");
        
        // Command is valid, sending it to motor
        if(write(fd, &command, sizeof(int)) == -1)
            error_exit(&logger, "Writing command on pipe");
    }
        
    return 0;
}

void on_termination(int signo)
{
    if(signo == SIGTERM)
    {
        // Closes named pipes on process termination
        if(close(fd_x) == -1)
        error_exit(&logger, "Closing pipe with motor_x");
    if(close(fd_z) == -1)
        error_exit(&logger, "Closing pipe with motor_z");
    }
}

void decodeCommand(char *line, int *fd, int *command)
{
    // Create a pointer to the first element of the string
    // This is used to analize the string
    char* pointer = &line[0];
    
    // Analizes the first part of the command
    // to understand which motor to use
    if(*pointer == 'x')
        *fd = fd_x;
    else if(*pointer == 'z')
        *fd = fd_z;
    else
        return;
    
    // Check if underscore is present
    pointer++;
    if(*pointer != '_')
        return;
        
    // Analizes the second part of the command
    // to understand how the motor should move
    pointer++;
    if(strcmp(pointer, "inc") == 0)
        *command = 1;
    else if(strcmp(pointer, "dec") == 0)
        *command = -1;
    else if(strcmp(pointer, "stp") == 0)
        *command = 2;
}

void print_commands_info(void)
{
    printf("These are the commands to move the hoist:\n");
    printf(" x_inc : makes the hoist move on positive x asis\n");
    printf(" x_dec : makes the hoist move on negative x asis\n");
    printf(" x_stp : makes the host stop the motion on x asis\n");
    printf(" z_inc : makes the hoist move on positive z asis\n");
    printf(" z_dec : makes the hoist move on negative z asis\n");
    printf(" z_stp : makes the host stop the motion on z asis\n");
    printf(" help  : prints commands and their functions\n");
    fflush(stdout);
}
