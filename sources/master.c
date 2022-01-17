#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <netinet/in.h>
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
    int spawned;
    int posx;
    int posy;
    int landed;
}Drone;

void free_resources();
void accept_client();
void disconnect_client(int index);
int on_spawn_message(int id, int posx, int posy);
int on_move_message(int id, int offx, int offy);
int on_landing_message(int id, int landing);
int get_drone(int posx, int posy);
void init_map();
void print_map();
void perror_exit();

char* colors[] = {RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN};

int s_endpoint;
int c_endpoints[MAX_CLIENTS];

char map[40][80];
Drone drones[MAX_CLIENTS];

int main(int argc, char *argv[])
{
    // Inits the map to have walls
    init_map();
    
    // Init drones and endpoints
    for(int i=0; i<MAX_CLIENTS; ++i)
    {
        c_endpoints[i] = -1;
        drones[i] = (Drone){0, -1, -1, 0};
    }
    
    // Init drone_api library
    handle_spawn_message = on_spawn_message;
    handle_move_message = on_move_message;
    handle_landing_message = on_landing_message;
    
    // Prints an empty map
    print_map();
    
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
        for(int i=0; i<MAX_CLIENTS; ++i)
            if(c_endpoints[i] != -1)
                FD_SET(c_endpoints[i], &fds);
        
        // Waits for one of the file descriptors to become ready
        int activity = select(FD_SETSIZE+1, &fds, NULL, NULL, NULL);
        if(activity == -1)
            perror_exit("Selecting a file descriptor");
            
        // If activity was on s_endpoint, a new client wants to connect
        if(FD_ISSET(s_endpoint, &fds))
            accept_client();
        
        // If activity was on one of the the c_endpoints, new data is available
        for(int i=0; i<MAX_CLIENTS; ++i)
            if(FD_ISSET(c_endpoints[i], &fds))
                if(decode_client_message(i, c_endpoints[i]) == EPIPE)
                    disconnect_client(i);
                
        // Prints the new map
        print_map();
    }
        
    return 0;
}

void free_resources()
{
    // Closing connections with client
    for(int i=0; i<MAX_CLIENTS; ++i)
        if(c_endpoints[i] != -1 && close(c_endpoints[i]) == -1)
            perror("Closing client connection");
        
    // Closing socket
    if(close(s_endpoint) == -1)
        perror("Closing socket");
}

void accept_client()
{
    struct sockaddr_in cli_addr;
    
    // Accepting client connection
    int clilen = sizeof(cli_addr);
    int c_endpoint = accept(s_endpoint, (struct sockaddr *) &cli_addr, &clilen);
    if(c_endpoint == -1)
        perror_exit("Accepting client connection");
        
    // Saving client file descriptor in array
    for(int i=0; i<MAX_CLIENTS; ++i)
        if(c_endpoints[i] == -1)
        {
            c_endpoints[i] = c_endpoint;
            break;
        }
}

void disconnect_client(int id)
{
    // Removing client file descriptor
    c_endpoints[id] = -1;
    
    // Removing drone from map
    Drone drone = drones[id];
    map[drone.posx][drone.posy] = ' ';
    // Reinitializing drone
    drones[id] = (Drone){0, -1, -1, 0};    
}

int on_spawn_message(int id, int posx, int posy)
{
    // Basic checks
    if(posx < 0 || posx >= 40 || posy < 0 || posy >= 80)
        return OUT_OF_BOUNDS_POSITION;
    if(map[posx][posy] == '*')
        return OCCUPIED_POSITION_WALL;
    if(map[posx][posy] == 'X')
        return OCCUPIED_POSITION_DRONE;
    
    // Creating drone
    drones[id] = (Drone){1, posx, posy, 0};
    map[posx][posy] = 'X';
    
    return SUCCESS;
}

int on_move_message(int id, int offx, int offy)
{
    Drone drone = drones[id];
    
    // Basic checks
    if(drone.spawned == 0)
        return DRONE_NOT_SPAWNED;
    if(drone.landed == 1)
        return DRONE_IS_LANDED;
        
    // Getting target position
    int posx = drone.posx + offx;
    int posy = drone.posy + offy;
    
    // Basic checks
    if(posx < 0 || posx >= 40 || posy < 0 || posy >= 80)
        return OUT_OF_BOUNDS_POSITION;
    if(map[posx][posy] == '*')
        return OCCUPIED_POSITION_WALL;
    if(map[posx][posy] == 'X')
        return OCCUPIED_POSITION_DRONE;
    
    // Moving drone
    map[drone.posx][drone.posy] = ' ';
    map[posx][posy] = 'X';
    drones[id].posx = posx;
    drones[id].posy = posy;
    
    return SUCCESS;
}


int on_landing_message(int id, int landed)
{
    Drone drone = drones[id];
    
    // A not spawned drone cannot land
    if(drone.spawned == 0)
        return DRONE_NOT_SPAWNED;
        
    drones[id].landed = landed;
    map[drone.posx][drone.posy] = 'O';
    
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
            int id = get_drone(i, j);
                
            if(id != -1)
                printf("%s%c" RESET, colors[id], map[i][j]);
            else
                printf("%c", map[i][j]);
        }
        printf("\n");
    }
    
    fflush(stdout);
}

int get_drone(int posx, int posy)
{
    for(int i=0; i<MAX_CLIENTS; ++i)
    {
        Drone drone = drones[i];
        if(drone.spawned == 1 && drone.posx == posx && drone.posy == posy)
            return i;
    }
    return -1;
}

void perror_exit(char* s)
{
    free_resources();
    perror(s);
    fflush(stdout);
    exit(0);
}
