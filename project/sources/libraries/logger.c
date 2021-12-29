#include "logger.h"

void perror_exit(Logger* logger, char* text)
{
    // Printing on log and console
    perror_cont(logger, text);
    
    // Terminating caller process
    kill(getpid(), SIGTERM);
}

void error_exit(Logger* logger, char* text)
{
    // Printing on log and console
    info(logger, text);
    
    // Terminating caller process
    kill(getpid(), SIGTERM);
}


void perror_cont(Logger* logger, char* text)
{
    // Concatenating string to obtain error
    char* error = strerror(errno);
    char* middle = ": \0";
    
    char string[512] = "\0";
    strcat(string, text);
    strcat(string, middle);
    strcat(string, error);
    
    // Print error in log and console but continue executing
    info(logger, string);
}

void info(Logger* logger, char* text)
{
    // Writing error on log.txt file
    write(logger->fd, "[", 1);
    write(logger->fd, logger->prefix, strlen(logger->prefix));
    write(logger->fd, "] ", 2);
    write(logger->fd, text, strlen(text));
    write(logger->fd, "\n", 1);
    
    // Writing error on console
    printf("\n\n[");
    printf("%s", logger->prefix);
    printf("] ");
    printf("%s", text);
    printf("\n");
    fflush(stdout);
}
