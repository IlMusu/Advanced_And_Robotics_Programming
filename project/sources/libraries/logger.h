#ifndef ASSIGNEMNT2_LOGGER_LIB
#define ASSIGNEMNT2_LOGGER_LIB

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

typedef struct Logger
{
    char* prefix;
    int fd;
}Logger;

void perror_exit(Logger* logger, char* text);
void error_exit(Logger* logger, char* text);
void perror_cont(Logger* logger, char* text);
void info(Logger* logger, char* text);

#endif //ASSIGNEMNT2_LOGGER_LIBR
