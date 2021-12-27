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

    // Getting server hostname
 	struct hostent *server; 
	server = gethostbyname("127.0.0.1");
 	if (server == NULL)
 		printf("Could not get local host.\n");
	
 	struct sockaddr_in serv_addr;
 	
	// Setting server data and port number (51234)
	bzero((char *) &serv_addr, sizeof(serv_addr));
 	serv_addr.sin_family = AF_INET;
 	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
 	serv_addr.sin_port = htons(51234); 

	// Making a connection request to the server
	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
 		perror("Connection with server"); 
 		
    // Creates data array
    char* data = (char*)malloc(array_length);
    if(data == NULL)
        printf("Not enough memory");
    fill_randomly(data, array_length);
    
    // Gets the time at the beginning of the transmission
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    write_time_in_file(time, "/tmp/time_1");
    
    for(int i=0; i<array_length; ++i)
    {
        if(write(sockfd, &data[i], sizeof(char)) == -1)
            perror("Writing on socket");
    }
    
    // Freeing memory used for array
    free(data);

 	return 0;
}
