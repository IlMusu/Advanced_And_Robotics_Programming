#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

pid_t spawn(const char* program, char** arg_list);

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
    char * args_x[] = { "./motor", fl_cx, fl_ix, (char*)NULL };
    pid_t pidmx = spawn("./motor", args_x);
    
    // child (motor_z)
    char * args_z[] = { "./motor", fl_cz, fl_iz, (char*)NULL };
    pid_t pidmz = spawn("./motor", args_z);
    
    char smx[10];
    char smz[10];
    sprintf(smx, "%i", pidmx);
    sprintf(smz, "%i", pidmz);
    
    // child (watchdog)
    char * args_wd[] = { "./watchdog", smx, smz, (char*)NULL };
    pid_t pidwd = spawn("./watchdog", args_wd);
    
    printf("Watchdog id: %i\n", pidwd);
    
    char swd[10];
    sprintf(swd, "%i", pidwd);
    
    // child (command_console)
    char * args_cc[] = { "/usr/bin/konsole",  "-e", "./command_console", fl_cx, fl_cz, swd, (char*)NULL };
    spawn("/usr/bin/konsole", args_cc);
    
    // child (inspection_console)    
    char * args_ic[] = { "/usr/bin/konsole",  "-e", "./inspection_console", smx, smz, fl_ix, fl_iz, (char*)NULL };
    spawn("/usr/bin/konsole", args_ic);
    
    // Wait to the child processes to terminate
    // This is done to free the resources
    for(int i=0; i<4; ++i)
        wait(NULL);
    
    printf("Freeing resources.\n");
    unlink(fl_cx);
    unlink(fl_cz);
    unlink(fl_ix);
    unlink(fl_iz);
    
    return 0;
}

pid_t spawn(const char* program, char** arg_list) 
{
    pid_t child_pid = fork();
    
    if(child_pid != 0)
        return child_pid;
    else 
        return execvp(program, arg_list);
}

