#include <unistd.h>     
#include <stdio.h> 
#include <semaphore.h>
#include <sys/wait.h>

#define BUFFER_SIZE "5"

int spawn(const char* program, char** arg_list);

int main(int argc, char* argv[])
{    
    char * prod_args[] = { "/usr/bin/konsole",  "-e", "./producer", BUFFER_SIZE, (char*)NULL };
    char * cons_args[] = { "/usr/bin/konsole",  "-e", "./consumer", BUFFER_SIZE, (char*)NULL };
    
    spawn("/usr/bin/konsole", prod_args);
    spawn("/usr/bin/konsole", cons_args);
    
    printf("Waiting for both processes termination...\n");
    
    wait(NULL);
    wait(NULL);
    
    printf("Both processes terminated.\n");
    fflush(stdout);
}

int spawn(const char* program, char** arg_list) 
{
    pid_t child_pid = fork();
    if (child_pid != 0)
        return child_pid;
    else 
    {
        execvp(program, arg_list);
        perror("exec failed");
        return 1;
    }
}

