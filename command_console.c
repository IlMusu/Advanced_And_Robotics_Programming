#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[])
{    
    // Opening files
    int fd_x = open(argv[1], O_WRONLY);
    int fd_z = open(argv[2], O_WRONLY);
    
    while(1)
    {
        printf("Insert command: ");
        fflush(stdout);
        
        // Reads a string from the command line
        char line[80];
        scanf("%s", line);
        
        // Create a pointer to the first element of the string
        // This is used to analize the string
        char* pointer = &line[0];
        
        int fd = -1;
        int command = -2;
        
        // Analizes the first part of the command
        // to understand which motor to use
        if(*pointer == 'x')
            fd = fd_x;
        else if(*pointer == 'z')
            fd = fd_z;
        
        if(fd != -1)
        {
            // Check if underscore is present
            pointer++;
            if(*pointer == '_')
            {
                // Analizes the second part of the command
                // to understand how the motor should move
                pointer++;
                if(strcmp(pointer, "inc") == 0)
                    command = 1;
                else if(strcmp(pointer, "dec") == 0)
                    command = -1;
                else if(strcmp(pointer, "stp") == 0)
                    command = 2;
            }
        }
        
        if(fd == -1 || command == -2)
        {
            printf("Invalid command.\n");
            fflush(stdout);
            continue;
        }
        else
        {
            // Command is valid, sending it to motor
            if(write(fd, &command, sizeof(int)) == -1)
                perror("Passing command to motor");
        }
    }
    
    // Closing files
    close(fd_x);
    close(fd_z);
    
    return 0;
}
