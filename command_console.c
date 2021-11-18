#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    // Gets the file descriptors for the two motors
    // This needs conversion from string to int
    int fd_motor_x = atoi(argv[1]);
    int fd_motor_z = 5; //atoi(argv[2]); 5 is temporary, just to test
    
    while(1)
    {
        printf("Insert command: ");
        fflush(stdout);
        
        // Reads 80 chars from the 
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
            fd = fd_motor_x;
        else if(*pointer == 'z')
            fd = fd_motor_z;
        
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
            // Command is valid, can be sent to motors
            write(fd, &command, sizeof(int));
        }
    }
    
    return 0;
}
