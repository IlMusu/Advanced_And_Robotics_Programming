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
#include "libraries/utils.h"

void error(char *msg);

int main(int argc, char *argv[])
{
	// Gets the array dimension
    int array_length = atoi(argv[1]);
 	
 	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
 	if (sockfd < 0)
 		error("ERROR opening socket");

 	struct hostent *server; 
	server = gethostbyname("127.0.0.1");
 	if (server == NULL) {
 		fprintf(stderr,"ERROR, no such host\n");
 		exit(0);
 	}
	
 	struct sockaddr_in serv_addr;
	// set the fields in serv_addr
	bzero((char *) &serv_addr, sizeof(serv_addr));
 	serv_addr.sin_family = AF_INET;
 	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
 	serv_addr.sin_port = htons(51234); 

	// establish a connection to the server
	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
 		error("ERROR connecting"); 
 		
    // Creates data array
    char data[array_length];
    fill_randomly(data, array_length);
    
    // Gets the time at the beginning of the transmission
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    write_time_in_file(time, "/tmp/time_1");
    
    for(int i=0; i<array_length; ++i)
    {
        if(write(sockfd, &data[i], sizeof(char)) == -1)
            perror("Writing on socket");
        else
        {
            printf("Writing %c on socket\n", data[i]);
            fflush(stdout);
        }
    }

 	return 0;
}

void error(char *msg)
{
	perror(msg);
	exit(0);
}
