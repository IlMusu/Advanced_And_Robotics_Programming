#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int spawn(const char* program, char** arg_list);

int main(int argc, char *argv[])
{
    // creates pipe for command_console and motor_x
    int pipe_fd[2];
    if(pipe(pipe_fd) == -1)
        perror("Creating pipe for command_console and motor_x");
    
    // makes the pipe non-blocking for reading
    if(fcntl(pipe_fd[0], F_SETFL, O_NONBLOCK) == -1)
        perror("Making pipe non-blocking");
        
    printf("pipe fds are: %i %i.\n", pipe_fd[0], pipe_fd[1]);

    // child (command_console)
    char str1[10];
    sprintf(str1, "%d", pipe_fd[1]);
    char * arg_list_1[] = { "/usr/bin/konsole",  "-e", "./command_console", str1, (char*)NULL };
    spawn("/usr/bin/konsole", arg_list_1);
    
    // child (motor_x)
    char str0[10];
    sprintf(str0, "%d", pipe_fd[0]);
    char * arg_list_0[] = { "/usr/bin/konsole",  "-e", "./motor", str0, (char*)NULL };
    spawn("/usr/bin/konsole", arg_list_0);
    
    return 0;
}


int spawn(const char* program, char** arg_list) 
{
    pid_t child_pid = fork();
    if (child_pid != 0)
    {
        // On parent process, returns the just
        // created child process ID
        return child_pid;
    }
    else 
    {
        // On child process, changes the program
        // with the one passed as argument
        if(execvp (program, arg_list) == -1)
            perror("exec failed");
        return 1;
    }
}

