#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

pid_t spawn(const char* program, char** arg_list);

int main(int argc, char *argv[])
{ 
    //###################### CREATING FILES FOR COMUNICATION ###########################
        
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
        
    // Creates named pipe for comunication between master and watchdog    
    char* fl_mw = "/tmp/master_watchdog";
    if(mkfifo(fl_mw, 0666) == -1)
        perror("Creating pipe for master and watchdog");
    
    //########################### CREATING PROCESSES #################################
    
    // child (watchdog)
    char * args_wd[] = { "./watchdog", fl_mw, (char*)NULL };
    pid_t pid_wd = spawn("./watchdog", args_wd);
    
    char spid_wd[10];
    sprintf(spid_wd, "%i", pid_wd);
    
    // child (motor_x)
    char * args_x[] = { "./motor", fl_cx, fl_ix, spid_wd, (char*)NULL };
    pid_t pid_mx = spawn("./motor", args_x);
    
    // child (motor_z)
    char * args_z[] = { "./motor", fl_cz, fl_iz, spid_wd, (char*)NULL };
    pid_t pid_mz = spawn("./motor", args_z);
    
    // Comunicates to watchdog the pid of the two motors
    int fd_wd = open(fl_mw, O_WRONLY);
    write(fd_wd, &pid_mx, sizeof(int));
    write(fd_wd, &pid_mz, sizeof(int));
    close(fd_wd);
    unlink(fl_mw);
    
    char spid_mx[10];
    char spid_mz[10];
    sprintf(spid_mx, "%i", pid_mx);
    sprintf(spid_mz, "%i", pid_mz);
    
    // child (command_console)
    char * args_cc[] = { "/usr/bin/konsole",  "-e", "./command_console", fl_cx, fl_cz, spid_wd, (char*)NULL };
    pid_t pid_cc = spawn("/usr/bin/konsole", args_cc);
    
    // child (inspection_console)    
    char * args_ic[] = { "/usr/bin/konsole",  "-e", "./inspection_console", fl_ix, fl_iz, spid_mx, spid_mz, spid_wd, (char*)NULL };
    pid_t pid_ic = spawn("/usr/bin/konsole", args_ic);
    
    //########################## CLEANING UP RESOURCES #############################
    
    // Waits for the termination of one of it's children
    wait(NULL);
    // If event happens, terminates all of them ...
    printf("Terminating all children and freeing resources.\n");
    kill(pid_mx, SIGKILL);
    kill(pid_mz, SIGKILL);
    kill(pid_wd, SIGKILL);
    kill(pid_cc, SIGKILL);
    kill(pid_ic, SIGKILL);
    // ... and then, it frees all the resources
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

