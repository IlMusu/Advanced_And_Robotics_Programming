#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int pipe_fd = atoi(argv[1]);
    
    while(1)
    {
        // Reads at most 80 chars from the command line
        printf("Insert command: ");
        char value[80];
        fgets(value, 80, stdin);
        
        if(strcmp(value, "x_inc"))
        {
            int command = 1231;
            write(pipe_fd, &command, sizeof(int)); 
        }
        else if(strcmp(value, "x_dec"))
        {
        
        }
        else if(strcmp(value, "x_stop"))
        {
        
        }
        else if(strcmp(value, "z_inc"))
        {
        
        }
        else if(strcmp(value, "z_dec"))
        {
        
        }
        else if(strcmp(value, "z_stop"))
        {
        
        }
        else
        {
            printf("Invalid command.\n");
        }
    }
    
    return 0;
}
