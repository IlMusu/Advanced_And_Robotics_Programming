#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <malloc.h>
#include "../libraries/utils.h"
#include "../libraries/logger.h"

void on_error(int signo);
void free_resources();

Logger logger;

int sockfd;
int newsockfd;
char* data;

int main(int argc, char *argv[])
{
    // Creating logger
    logger = (Logger){"consumer_socket", atoi(argv[1])};
    
    // Gets the array dimension
    int array_length = atoi(argv[2]);
    
    // Opening socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        perror_exit(&logger, "Opening socket");
        
    struct sockaddr_in serv_addr, cli_addr;

    // Setting server data and port number (51234)  
    bzero((char *) &serv_addr, sizeof(serv_addr));  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(51234);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    // Binding server address to socket
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
        perror_exit(&logger, "Binding socket");
        
    // Makes the server listen for clients with a maximum queue of 5
    listen(sockfd, 5);
    
    // Waiting for a client request
    int clilen = sizeof(cli_addr);
    if((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) == -1)
        perror_exit(&logger, "Accepting client request");
        
    // Creates data array
    if((data = (char*)malloc(array_length)) == NULL)
        error_exit(&logger, "Creating array with malloc");
    
    int last_percent = 0;
    for(int i=0; i<array_length; ++i)
    {
        if(read(newsockfd, &data[i], sizeof(char)) == -1)
            perror_exit(&logger, "Reading from pipe");
            
        int percent = (int)((float)i/array_length*100);
        if(percent > last_percent && percent%10 == 0)
        {
            printf("%i%%...", percent);
            fflush(stdout);
            last_percent = percent;
        } 
    }
    
    // Gets the time at the end of the transmission
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    if(write_time_in_file(time, "/tmp/time_2") == -1)
        perror_exit(&logger, "Comunicating time to master");
    
    // Freeing resources on termination
    free_resources();
        
    return 0;
}

void on_error(int signo)
{
    if(signo == SIGTERM)
    {
        free_resources();
        exit(1);
    }
}

void free_resources()
{
    // Freeing memory used for array
    if(data != NULL)
        free(data);
        
    // Closing socket
    if(close(sockfd) == -1)
        perror_cont(&logger, "Closing socket");
        
    // Closing client socket
    if(close(newsockfd) == -1)
        perror_cont(&logger, "Closing client socket");
}
