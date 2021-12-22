#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>
#include "libraries/utils.h"

void error(char *msg);

int main(int argc, char *argv[])
{
    // Gets the array dimension
    int array_length = atoi(argv[1]);
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
        
    struct sockaddr_in serv_addr, cli_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));

    // Sets server data and port number (51234)    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(51234);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
        
    // Makes the server listen for clients with a maximum queue f 5
    listen(sockfd, 5);
    
    int clilen = sizeof(cli_addr);
    // This makes the process block until a new client makes a request
    int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
        
    // Creates data array
    char data[array_length];
    
    for(int i=0; i<array_length; ++i)
    {
        // Reads one char from the socket
        if(read(newsockfd, &data[i], sizeof(char)) == -1)
            perror("Reading from socket");
        else
        {
            printf("Reading %c from socket\n", data[i]);
            fflush(stdout);
        }
    }
    
    // Gets the time at the end of the transmission
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    write_time_in_file(time, "/tmp/time_2");
        
    return 0;
}

void error(char *msg)
{
    perror(msg);
    exit(1);
}
