#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>
#include <malloc.h>
#include "../libraries/utils.h"

int main(int argc, char *argv[])
{
    // Gets the array dimension
    int array_length = atoi(argv[1]);
    
    // Opening socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        perror("Opening socket");
        
    struct sockaddr_in serv_addr, cli_addr;

    // Setting server data and port number (51234)  
    bzero((char *) &serv_addr, sizeof(serv_addr));  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(51234);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    // Binding server address to socket
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        perror("Binding socket");
        
    // Makes the server listen for clients with a maximum queue of 5
    listen(sockfd, 5);
    
    // Waiting for a client request
    int clilen = sizeof(cli_addr);
    int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
        perror("Accepting client request");
        
    // Creates data array
    char* data = (char*)malloc(array_length);
    if(data == NULL)
        printf("Not enough memory");
    
    int last_percent = 0;
    for(int i=0; i<array_length; ++i)
    {
        if(read(newsockfd, &data[i], sizeof(char)) == -1)
            perror("Reading from pipe");
            
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
    write_time_in_file(time, "/tmp/time_2");
    
    // Freeing memory used for array
    free(data);
        
    return 0;
}
