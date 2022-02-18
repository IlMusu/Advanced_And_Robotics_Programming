#include "logger.h"

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define WHITE   "\033[37m"

void info_hidden(Logger* logger, char* text, char* color, int console);

int create_logger(Logger* logger, char* prefix, char* path)
{
    // Creating log file
    int fd = open(path, O_WRONLY|O_CREAT|O_APPEND, 0666);
    if(fd == -1)
        return -1;
        
    // Getting absolute path for log file
    char* absolute_path = realpath(path, NULL);
    if(absolute_path == NULL)
        return -1;
        
    logger->prefix = prefix;
    logger->path = absolute_path;
    logger->fd = fd;
    
    return 0;
}

int destroy_logger(Logger* logger)
{
    // Closing the opened log file
    if(close(logger->fd))
        return -1;
        
    return 0;
}

// Prints error from perror and makes process terminate
void perror_exit(Logger* logger, char* text)
{
    // Printing on log and console
    perror_cont(logger, text);
    
    // Terminating caller process
    if(kill(getpid(), SIGTERM) == -1)
        perror("Logger sending SIGTERM signal");
}

// Prints error from perror and makes process continue
void perror_cont(Logger* logger, char* text)
{
    // Creating the full error string
    char* text_perror;
    asprintf(&text_perror, "%s : %s", text, strerror(errno));
    
    // Actually printing the string in log file
    error_cont(logger, text_perror);
    
    // Deallocating the string after usage
    free(text_perror);
}

// Prints error in text and makes process terminate
void error_exit(Logger* logger, char* text)
{
    // Printing on log and console
    error_cont(logger, text);
    
    // Terminating caller process
    if(kill(getpid(), SIGTERM) == -1)
        perror("Logger sending SIGTERM signal");
}

// Prints error in text and makes process continue
void error_cont(Logger* logger, char* text)
{
    info_hidden(logger, text, RED, 1);
}

void info(Logger* logger, char* text, int console)
{
    info_hidden(logger, text, WHITE, console);
}

void info_hidden(Logger* logger, char* text, char* color, int console)
{
    // Getting current time for log
    char now[9];
    strftime(now, 9, "%X", localtime(&(time_t){time(NULL)}));
    
    // Writing full entry on log file
    char* entry;
    asprintf(&entry, "%s | [%s] %s\n", now, logger->prefix, text);
    
    // Actually printing the string in the log file
    if(write(logger->fd, entry, strlen(entry)) == -1)
        perror("Writing entry on log file");
    
    // Writing info on console
    if(console)
    {
        printf("\n %s %s RESET \n", color, entry);
        fflush(stdout);
    }
    
    // String needs to be deallocated when using asprintf
    free(entry);
}
