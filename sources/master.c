#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include "./drone_api/drone_api.h"

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"

#define MAX_CLIENTS 6

typedef struct{
    int posx;
    int posy;
    int is_landed;
}drone;

void free_resources();
void accept_client();
int on_spawn_message(int id, int posx, int posy);
int on_move_message(int id, int offx, int offy);
int on_landing_message(int id, int landing);
void init_map();
void print_map();
void perror_exit();

char* colors[] = {RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN};

int s_endpoint;
int c_endpoints[MAX_CLIENTS];
int connections = 0;

char map[40][80];
drone drones[MAX_CLIENTS];

int main(int argc, char *argv[])
{
    // Inits the map to have walls
    init_map();
    
    handle_spawn_message = on_spawn_message;
    handle_move_message = on_move_message;
    handle_landing_message = on_landing_message;
    
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
                decode_client_message(i, c_endpoints[i]);
                
        // Prints the new map
        print_map();
    }
        
    return 0;
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

void accept_client()
{
    struct sockaddr_in cli_addr;
    
    int clilen = sizeof(cli_addr);
    int c_endpoint = accept(s_endpoint, (struct sockaddr *) &cli_addr, &clilen);
    if(c_endpoint == -1)
        perror_exit("Accepting client connection");
        
    c_endpoints[connections++] = c_endpoint;
}

int on_spawn_message(int id, int posx, int posy)
{
    if(posx < 0 || posx >= 40 || posy < 0 || posy >= 80)
        return OUT_OF_BOUNDS_POSITION;
    if(map[posx][posy] != ' ')
        return OCCUPIED_POSITION;
    
    // Creating drone
    drones[id] = (drone){posx, posy};
    map[posx][posy] = 'X';
    
    return SUCCESS;
}

int on_landing_message(int id, int landing)
{
    drone d = drones[id];
    drones[id].is_landed = landing;
    map[d.posx][d.posy] = 'O';
    return SUCCESS;
}

int on_move_message(int id, int offx, int offy)
{
    drone d = drones[id];
    int posx = d.posx + offx;
    int posy = d.posy + offy;
        
    if(posx < 0 || posx >= 40 || posy < 0 || posy >= 80)
        return OUT_OF_BOUNDS_POSITION;
    if(map[posx][posy] == '*')
        return NOT_MOVED_WALL;
    if(map[posx][posy] == 'X')
        return NOT_MOVED_DRONE;
    
    map[d.posx][d.posy] = ' ';
    map[posx][posy] = 'X';
    
    drones[id].posx = posx;
    drones[id].posy = posy;
    
    return SUCCESS;
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
        for(int j=0; j<=79; ++j)
        {
            int is_drone = 0;
            for(int c=0; c<connections; ++c)
                if(drones[c].posx == i && drones[c].posy == j)
                {
                    printf("%s%c" RESET, colors[c], map[i][j]);
                    is_drone = 1;
                    break;
                }
                
            if(is_drone)
                continue;
                
            printf("%c", map[i][j]);
        }
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
