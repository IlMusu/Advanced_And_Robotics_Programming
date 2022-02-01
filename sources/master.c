#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include "./drone_api/drone_api.h"
#include "./logger/logger.h"

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"

#define MAX_CLIENTS 6
#define MAP_SIZE_X 40
#define MAP_SIZE_Y 80
#define MAP_SIZE_Z 10

void free_resources();
void on_error(int signo);

void accept_client();
void disconnect_client(int index);

void recreate_map();
void print_map();
int is_inside_map(int posx, int posy, int posz);
int get_drone(int posx, int posy, int posz);
int get_visible_drone(int posx, int posy);
char drone_to_char(Drone drone);

int on_spawn_message(int id, int posx, int posy, int posz);
int on_move_message(int id, int offx, int offy, int offz);
int on_landing_message(int id, int landing);

char* colors[] = {RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN};

Logger logger = {"", "", -1};

int s_endpoint;
int c_endpoints[MAX_CLIENTS];

char map[MAP_SIZE_X][MAP_SIZE_Y];
Drone drones[MAX_CLIENTS];

int main(int argc, char *argv[])
{
    // Init logger
    create_logger(&logger, "MASTER", argv[1]);
    
    // Printing start time in logger
    char text[100];
    strftime(text, 100, "Started on %x at %X.", localtime(&(time_t){time(NULL)}));
    info(&logger, text, 0);
  
    // So that process can release resources in case of error
    signal(SIGTERM, on_error);
    
    // Init drones and endpoints
    for(int i=0; i<MAX_CLIENTS; ++i)
    {
        c_endpoints[i] = -1;
        drones[i] = (Drone){0, -1, -1, -1, 0};
    }
    
    // Init drone_api library
    handle_spawn_message = on_spawn_message;
    handle_move_message = on_move_message;
    handle_landing_message = on_landing_message;
    
    // Prints an empty map
    recreate_map();
    print_map();
    
    // Setting server data
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));  
    serv_addr.sin_family = AF_INET;             // Using protocol IPv4
    serv_addr.sin_port = htons(51234);          // Setting server port
    serv_addr.sin_addr.s_addr = INADDR_ANY;     // Server listens on all available interfaces
    
    // Opening socket
    if((s_endpoint = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        perror_exit(&logger, "Opening socket");
    
    // Binding server address to socket
    if (bind(s_endpoint, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
        perror_exit(&logger, "Binding socket");
        
    // Makes the server listen for clients with a maximum queue of 5
    if(listen(s_endpoint, 5) == -1)
        perror_exit(&logger, "Listening on socket");
    
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
            perror_exit(&logger, "Selecting a file descriptor");
            
        // If activity was on s_endpoint, a new client wants to connect
        if(FD_ISSET(s_endpoint, &fds))
            accept_client();
        
        // If activity was on one of the the c_endpoints, new data is available
        for(int i=0; i<MAX_CLIENTS; ++i)
            if(FD_ISSET(c_endpoints[i], &fds))
            {
                int error = decode_client_message(i, c_endpoints[i]);
                
                if(error == EPIPE)
                    disconnect_client(i);
                else if(error == -1)
                    perror_exit(&logger, "Decoding client message");
            }
                
        // Prints the new map
        recreate_map();
        print_map();
    }
        
    return 0;
}

void free_resources()
{
    // Closing connections with client
    for(int i=0; i<MAX_CLIENTS; ++i)
        if(c_endpoints[i] != -1 && close(c_endpoints[i]) == -1)
            perror_cont(&logger, "Closing client connection");
        
    // Closing socket
    if(close(s_endpoint) == -1)
        perror_cont(&logger, "Closing socket");
}

void on_error(int signo)
{
    if(signo == SIGTERM)
    {
        free_resources();
        exit(1);
    }
}

void accept_client()
{
    struct sockaddr_in cli_addr;
    
    // Accepting client connection
    int clilen = sizeof(cli_addr);
    int c_endpoint = accept(s_endpoint, (struct sockaddr *) &cli_addr, &clilen);
    if(c_endpoint == -1)
        perror_exit(&logger, "Accepting client connection");
        
    // Checking if server has a index for the client
    int index = -1;
    for(int i=0; i<MAX_CLIENTS; ++i)
        if(c_endpoints[i] == -1)
        {
            index = i;
            break;
        }
        
    // Getting client address
    char address[INET_ADDRSTRLEN];
    inet_ntop(cli_addr.sin_family, &(cli_addr.sin_addr), address, INET_ADDRSTRLEN);
        
    if(index != -1)
    {
        c_endpoints[index] = c_endpoint;
        char* text; asprintf(&text, "Client [%s] connected at index %d.", address, index);
        info(&logger, text, 0);
    }
    else
    {
        close(c_endpoint);
        info(&logger, "Client [%s] cannot connect: maximum capacity reached.", 0);
    }
}

void disconnect_client(int id)
{
    // Removing client file descriptor
    c_endpoints[id] = -1;
    
    // Reinitializing drone
    drones[id] = (Drone){0, -1, -1, 0};    
    
    char* text; asprintf(&text, "Client at index %d disconnected.", id);
    info(&logger, text, 0);
}

int on_spawn_message(int id, int posx, int posy, int posz)
{
    // Basic checks
    if(!is_inside_map(posx, posy, posz))
        return OUT_OF_BOUNDS_POSITION;
    if(map[posx][posy] == '*')
        return OCCUPIED_POSITION_WALL;
    if(get_drone(posx, posy, posz) != -1)
        return OCCUPIED_POSITION_DRONE;
    
    // Creating drone
    drones[id] = (Drone){1, posx, posy, posz, 0};
    
    return SUCCESS;
}

int on_move_message(int id, int offx, int offy, int offz)
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
    int posz = drone.posz + offz;

    // Basic checks
    if(!is_inside_map(posx, posy, posz))
        return OUT_OF_BOUNDS_POSITION;
    if(map[posx][posy] == '*')
        return OCCUPIED_POSITION_WALL;
    if(get_drone(posx, posy, posz) != -1)
        return OCCUPIED_POSITION_DRONE;
    
    // Moving drone
    drones[id].posx = posx;
    drones[id].posy = posy;
    drones[id].posz = posz;
    
    return SUCCESS;
}


int on_landing_message(int id, int landed)
{
    Drone drone = drones[id];
    
    // A not spawned drone cannot land
    if(drone.spawned == 0)
        return DRONE_NOT_SPAWNED;
        
    drones[id].landed = landed;
    
    return SUCCESS;
}

int is_inside_map(int posx, int posy, int posz)
{
    return  posx >= 0 && posx < MAP_SIZE_X && 
            posy >= 0 && posy < MAP_SIZE_Y &&
            posz >= 0 && posz < MAP_SIZE_Z;
}

int get_drone(int posx, int posy, int posz)
{
    for(int i=0; i<MAX_CLIENTS; ++i)
    {
        Drone d = drones[i];
        if(d.posx == posx && d.posy == posy && d.posz == posz)
            return i;
    }
    
    return -1;
}

int get_visible_drone(int posx, int posy)
{
    int posz = -1;
    int index = -1;
    
    for(int i=0; i<MAX_CLIENTS; ++i)
    {
        Drone d = drones[i];
        if(d.posx == posx && d.posy == posy && d.posz > posz)
            index = i;
    }
    
    return index;
}

void recreate_map()
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
    
    for(int i=0; i<10; ++i)
    {        
        map[i+03][54] = '*';
        map[i+24][43] = '*';
        map[13][i+06] = '*';
        map[33][i+38] = '*';
    }
    
    for(int i=0; i<MAX_CLIENTS; ++i)
    {
        Drone d = drones[i];
        if(d.spawned)
            map[d.posx][d.posy] = drone_to_char(d);
    }
}

void print_map()
{    
    for(int i=39; i>=0; --i)
    {
        for(int j=0; j<=79; ++j)
        {
            int id = get_visible_drone(i, j);
                
            if(id != -1)
                printf("%s%c" RESET, colors[id], map[i][j]);
            else
                printf("%c", map[i][j]);
        }
        printf("\n");
    }
    
    fflush(stdout);
}

char drone_to_char(Drone drone)
{
    if(!drone.spawned)
        return ' ';
    if(drone.landed)
        return 'X';
    return (char)(drone.posz+48);
}
