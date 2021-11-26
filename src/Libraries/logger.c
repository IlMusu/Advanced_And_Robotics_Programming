#include <errno.h>
#include <string.h>

typedef struct Logger
{
    char* prefix;
    int fd;
}Logger;

void error_exit(Logger* logger, char* text)
{
    char* error = strerror(errno);
    write(logger->fd, "[", 1);
    write(logger->fd, logger->prefix, strlen(logger->prefix));
    write(logger->fd, "] ", 2);
    write(logger->fd, text, strlen(text));
    write(logger->fd, ": ", 2);
    write(logger->fd, error, strlen(error));
    write(logger->fd, "\n", 1);
    exit(-1);
}

void info(Logger* logger, char* text)
{
    write(logger->fd, "[", 1);
    write(logger->fd, logger->prefix, strlen(logger->prefix));
    write(logger->fd, "] ", 2);
    write(logger->fd, text, strlen(text));
    write(logger->fd, "\n", 1);
}
