#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

int spawn(const char* program, char** arg_list);

int main(int argc, char *argv[])
{ 
    // Creates named pipe for comunication between command_console and motor_x
    char* fl_cx = "/tmp/cmd_motor_x";
    if(mkfifo(fl_cx, 0666) == -1)
        perror("Creating pipe for command_console and motor_x");
        
    // Creates named pipe for comunication between command_console and motor_z
    char* fl_cz = "/tmp/cmd_motor_z";
    if(mkfifo(fl_cz, 0666) == -1)
        perror("Creating pipe for command_console and motor_z");
       
    // Creates named pipe for comunication between inspection_console and motor_x
    char* fl_ix = "/tmp/isp_motor_x";
    if(mkfifo(fl_ix, 0666) == -1)
        perror("Creating pipe for inspection_console and motor_x");
        
    // Creates named pipe for comunication between inspection_console and motor_z
    char* fl_iz = "/tmp/isp_motor_z";
    if(mkfifo(fl_iz, 0666) == -1)
        perror("Creating pipe for inspection_console and motor_z");
    
    // child (motor_x)
    char * args_x[] = { "/usr/bin/konsole",  "-e", "./motor", fl_cx, fl_ix, (char*)NULL };
    spawn("/usr/bin/konsole", args_x);
    
    // child (motor_z)
    char * args_z[] = { "/usr/bin/konsole",  "-e", "./motor", fl_cz, fl_iz, (char*)NULL };
    spawn("/usr/bin/konsole", args_z);
    
    // child (command_console)
    char * args_cc[] = { "/usr/bin/konsole",  "-e", "./command_console", fl_cx, fl_cz, (char*)NULL };
    spawn("/usr/bin/konsole", args_cc);
    
    // child (inspection_console)
    char * args_ic[] = { "/usr/bin/konsole",  "-e", "./inspection_console", fl_ix, fl_iz, (char*)NULL };
    spawn("/usr/bin/konsole", args_ic);
    
    // Wait to the child processes to terminate
    // This is done to free the resources
    for(int i=0; i<4; ++i)
        wait(NULL);
    
    printf("Freeing resources.\n");
    unlink(fl_cx);
    unlink(fl_cz);
    
    return 0;
}

pid_t spawn(const char* program, char** arg_list) 
{
    pid_t child_pid = fork();
    
    if (child_pid != 0)
        return child_pid;
    else 
        return execvp(program, arg_list);
}

