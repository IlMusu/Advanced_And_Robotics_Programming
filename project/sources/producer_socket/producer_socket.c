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
char* data;

int main(int argc, char *argv[])
{
    // Creating logger
    logger = (Logger){"producer_socket", atoi(argv[1])};
    
    // So that process can release resources in case of error
    signal(SIGTERM, on_error);
    
	// Gets the array dimension
    int array_length = atoi(argv[2]);
 	
 	// Opening socket
 	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
 		perror_exit(&logger, "Opening socket");

    // Getting server hostname
 	struct hostent *server; 
	server = gethostbyname("127.0.0.1");
 	if (server == NULL)
 		error_exit(&logger, "Getting local host");
	
 	struct sockaddr_in serv_addr;
 	
	// Setting server data and port number (51234)
	bzero((char *) &serv_addr, sizeof(serv_addr));
 	serv_addr.sin_family = AF_INET;
 	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
 	serv_addr.sin_port = htons(51234); 

	// Making a connection request to the server
	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) == -1)
 		perror_exit(&logger, "Connection with server"); 
 		
    // Creates data array
    if((data = (char*)malloc(array_length)) == NULL)
        error_exit(&logger, "Creating array with malloc");
    fill_randomly(data, array_length);
    
    // Gets the time at the beginning of the transmission
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    write_time_in_file(time, "/tmp/time_1");
    
    for(int i=0; i<array_length; ++i)
    {
        if(write(sockfd, &data[i], sizeof(char)) == -1)
            perror_exit(&logger, "Writing on socket");
    }
    
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
}
