#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include "./drone_api/drone_api.h"

#define MAX_CLIENTS 10

typedef struct{
    int posx;
    int posy;
}drone;

void accept_client();
void on_socket_data(int id, int c_endpoint);
void init_map();
void print_map();
void perror_exit();
void free_resources();

int s_endpoint;
int c_endpoints[MAX_CLIENTS];
int connections = 0;

char map[40][80];
drone drones[MAX_CLIENTS];

int main(int argc, char *argv[])
{
    // Inits the map to have walls
    init_map();
    
    struct sockaddr_in serv_addr;
    
    // Setting server data
    bzero((char *) &serv_addr, sizeof(serv_addr));  
    serv_addr.sin_family = AF_INET;             // Using protocol IPv4
    serv_addr.sin_port = htons(51234);          // Setting server port
    serv_addr.sin_addr.s_addr = INADDR_ANY;     // Server listens on all available interfaces
    
    // Opening socket
    if((s_endpoint = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        perror_exit("Opening socket");
    
    // Binding server address to socket
    if (bind(s_endpoint, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
        perror_exit("Binding socket");
        
    // Makes the server listen for clients with a maximum queue of 5
    if(listen(s_endpoint, 5) == -1)
        perror_exit("Listening on socket");
    
    // Set of file descriptors
    fd_set fds;
    
    while(1)
    {
        // Sets the server and clients fds to the set
        FD_ZERO(&fds);
        FD_SET(s_endpoint, &fds);
        for(int i=0; i<connections; ++i)
            FD_SET(c_endpoints[i], &fds);
                
        // Waits for one of the file descriptors to become ready
        int activity = select(FD_SETSIZE+1, &fds, NULL, NULL, NULL);
        if(activity == -1)
            perror_exit("Selecting a file descriptor");
            
        // If activity was on s_endpoint, a new client wants to connect
        if(FD_ISSET(s_endpoint, &fds))
            accept_client();
        
        // If activity was on one of the the c_endpoints, new data is available
        for(int i=0; i<connections; ++i)
            if(FD_ISSET(c_endpoints[i], &fds))
                on_socket_data(i, c_endpoints[i]);
                
        // Prints the new map
        print_map();
    }
        
    return 0;
}

void accept_client()
{
    struct sockaddr_in cli_addr;
    
    int clilen = sizeof(cli_addr);
    int c_endpoint = accept(s_endpoint, (struct sockaddr *) &cli_addr, &clilen);
    if(c_endpoint == -1)
        perror_exit("Accepting client connection");
        
    c_endpoints[connections++] = c_endpoint;
}

void on_socket_data(int id, int c_endpoint)
{
    // Reading message type
    int message;
    if(read(c_endpoint, &message, sizeof(int)) == -1)
        perror("Reading message type");
        
    switch(message)
    {
        case SPAWN_MESSAGE:
        {
            // Reading positions from the message
            int posx, posy;
            if(read(c_endpoint, &posx, sizeof(int)) == -1)
                perror("Reading x position");
            if(read(c_endpoint, &posy, sizeof(int)) == -1)
                perror("Reading y position");
            
            if(posx < 0 || posx >= 40 || posy < 0 || posy >= 80)
            {
                send_result(c_endpoint, OUT_OF_BOUNDS_POSITION);
                return;
            }
            if(map[posx][posy] != ' ')
            {
                send_result(c_endpoint, OCCUPIED_POSITION);
                return;
            }
            
            // Creating drone
            drones[id] = (drone){posx, posy};
            map[posx][posy] = 'X';
            
            send_result(c_endpoint, SUCCESS);
            break;
        }
        case MOVE_MESSAGE:
        {
            // Reading positions from the message
            int offx, offy;
            if(read(c_endpoint, &offx, sizeof(int)) == -1)
                perror("Reading x position");
            if(read(c_endpoint, &offy, sizeof(int)) == -1)
                perror("Reading y position");
                
            drone d = drones[id];
            int posx = d.posx + offx;
            int posy = d.posy + offy;
                
            if(posx < 0 || posx >= 40 || posy < 0 || posy >= 80)
            {
                send_result(c_endpoint, OUT_OF_BOUNDS_POSITION);
                return;
            }
            if(map[posx][posy] == '*')
            {
                send_result(c_endpoint, NOT_MOVED_WALL);
                return;
            }
            if(map[posx][posy] == '*')
            {
                send_result(c_endpoint, NOT_MOVED_DRONE);
                return;
            }
            
            map[d.posx][d.posy] = ' ';
            map[posx][posy] = 'X';
            
            drones[id].posx = posx;
            drones[id].posy = posy;
            
            send_result(c_endpoint, SUCCESS);
            break;
        }
        default:
            break;
    }
}

void init_map()
{
    for(int i=0; i<40; ++i)
    {
        for(int j=0; j<80; ++j)
        {
            if(i==0 || i==39 || j==0 || j==79)
                map[i][j] = '*';
            else
                map[i][j] = ' ';
        }
    }
}

void print_map()
{
    for(int i=39; i>=0; --i)
    {
        for(int j=79; j>=0; --j)
            printf("%c", map[i][j]);
        printf("\n");
    }
    
    fflush(stdout);
}

void perror_exit(char* s)
{
    free_resources();
    perror(s);
    exit(0);
}

void free_resources()
{
    // Closing connections with client
    for(int i=0; i<connections; ++i)
        if(close(c_endpoints[i]) == -1)
            perror("Closing client connection");
        
    // Closing socket
    if(close(s_endpoint) == -1)
        perror("Closing socket");
}
