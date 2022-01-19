#ifndef ASSIGNEMNT3_LOGGER_LIB
#define ASSIGNEMNT3_LOGGER_LIB

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>

typedef struct Logger
{
    char* prefix;
    int fd;
}Logger;

int create_logger(Logger* logger, char* prefix, char* path);
void perror_exit(Logger* logger, char* text);
void perror_cont(Logger* logger, char* text);
void error_exit(Logger* logger, char* text);
void error_cont(Logger* logger, char* text);
void info(Logger* logger, char* text, int console);

#endif //ASSIGNEMNT3_LOGGER_LIBR
