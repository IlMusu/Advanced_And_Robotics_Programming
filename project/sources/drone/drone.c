#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h> 
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "./../drone_api/drone_api.h"
#include "./../logger/logger.h"

// Some console colors
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMGN  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

// The map sizes
#define MAP_SIZE_X 80
#define MAP_SIZE_Y 40

// The map can contain only these elements
#define WALL 'X'
#define DRONE 'O'
#define VISITED 'V'
#define EMPTY '-'

// Drone parameters
#define BATTERY_MAX 100
#define BATTERY_RECHARGE_VEL 500
#define MOVEMENT_VEL 200
#define STEPS_MAX 10
#define STEPS_MIN 4

// A Vector representation
typedef struct
{
    int x;
    int y;
    int z;
}Vec3;

// The Drone representation
typedef struct
{
    char* name;     // The name of the drone
    Vec3 pos;       // The drone current position
    Vec3 dir;       // The drone desired motion vector
    int steps;      // Remaining steps to perform in that direction
    int spawned;    // True if the drone spawned
    int landed;     // True if the drone is landed
    int battery;    // Value to identify the battery
    int goal;       // Current goal being executed by the drone
}DroneSim;

// A Goal representation
typedef struct
{
    int (*can_activate)(DroneSim drone);  // If the Goal can be activated
    int (*activate)(DroneSim* drone);     // Perform the goal action (1 when no error)
    int simulation_usec;                  // The time used to simulate the action 
}Goal;

int connect_to_master(int max_attempts);

// Spawn goal
int can_activate_spawn_goal(DroneSim drone);
int activate_spawn_goal(DroneSim* drone);
// Update direction goal
int can_activate_update_dir_goal(DroneSim drone);
int activate_update_dir_goal(DroneSim* drone);
// Move goal
int can_activate_move_goal(DroneSim drone);
int activate_move_goal(DroneSim* drone);
// Emergency land goal
int can_activate_em_land_goal(DroneSim drone);
int activate_em_land_goal(DroneSim* drone);
// Recharge goal
int can_activate_recharge_goal(DroneSim d);
int activate_recharge_goal(DroneSim* d);
// Takeoff goal
int can_activate_takeoff_goal(DroneSim d);
int activate_takeoff_goal(DroneSim* d);

void draw_screen();
int analyze_and_print_map();
void print_map_element(char element);
void log_drone(DroneSim drone);

int random_between(int min, int max);
void validate_response_or_exit(int result);
void on_error(int signo);
void free_resources();

Logger logger = {"", "", -1};
char map[MAP_SIZE_X][MAP_SIZE_Y];

int socket_fd;
int last_result = -1;

// The main function takes the following parameters:
// 1. The log file position
// 2. The drone name
int main(int argc, char *argv[])
{
    // ####################### PRE INIT #######################
    
    if(argv[1] == NULL)
        printf("The first parameter should be the log file position.\n");
    if(argv[2] == NULL)
        printf("The second parameter should be the drone name.\n");
    if(argv[1] == NULL || argv[2] == NULL)
        return 1;
    
    // Initializing the seed for random function
    srand(time(NULL));
    
    // Creating a logger for managing the log file
    create_logger(&logger, argv[2], argv[1]);
    
    // Initializing signals that the process will handle
    signal(SIGTERM, on_error);
    
    info(&logger, "Started now.", 0);
    
    // ####################### INIT #######################
    
    // Initializing map
    for(int y = 0; y < MAP_SIZE_Y ; y++)
        for(int x = 0; x < MAP_SIZE_X ; x++)
            map[x][y] = EMPTY;
            
    // Initializing drone
    DroneSim drone = { argv[2], {-1, -1, -1}, {0, 0, 0,}, 0, 0, 0, BATTERY_MAX };
    
    // Initializing some goals
    Goal spawn_goal = {can_activate_spawn_goal, activate_spawn_goal,                2000000};
    Goal update_dir_goal = {can_activate_update_dir_goal, activate_update_dir_goal, 1000};
    Goal move_goal = {can_activate_move_goal, activate_move_goal,                   10000000 / MOVEMENT_VEL};
    Goal em_land_goal = {can_activate_em_land_goal, activate_em_land_goal,          2000000};
    Goal recharge_goal = {can_activate_recharge_goal, activate_recharge_goal,       10000000 / BATTERY_RECHARGE_VEL};
    Goal takeoff_goal = {can_activate_takeoff_goal, activate_takeoff_goal,          2000000};
    
    // Initializing the drone goals: the order in which the goals are stored in the array is the priority
    // the have (if two goals can be activated at the same time, it is choosen the one with the highest priority).
    // This is the reason because "em_land_goal" is before "move_goal"
    int goals_size = 6;
    Goal goals[] = { spawn_goal, update_dir_goal, em_land_goal, move_goal, recharge_goal, takeoff_goal };

    info(&logger, "Parameters and Goals initialized.", 0);
    
    // ####################### SERVER CONNECTION #######################
    
    // Connecting to master process
    if(connect_to_master(10) == -1)
        perror_exit(&logger, "All connection attempts failed");
        
    info(&logger, "Connected to server.", 0);
        
    // ####################### PERFORMING GOALS #######################
    
    while(1)
    {
        // Iterating all goals in priority order
        for(int i=0; i<goals_size; ++i)
        {
            // Checking a goal can be activated
            Goal goal = goals[i];
            if(goal.can_activate(drone) != 1)
                continue;
                
            drone.goal = i;
            log_drone(drone);
            
            // Activating that goal and then breaking
            if(goal.activate(&drone))
            {   
                draw_screen(drone);
                usleep(goal.simulation_usec);
            }
            break;
        }
    }
}

int connect_to_master(int max_attempts)
{
    // Opening socket
 	if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
 		perror_exit(&logger,"Opening socket");

    // Getting server hostname
 	struct hostent *server;
    // specify the IP address
	server = gethostbyname("127.0.0.1");
 	if (server == NULL)
 		perror_exit(&logger, "Getting local host");
	
    // define struct
 	struct sockaddr_in serv_addr;
 	
	// Setting server data and port number (51234)
	bzero((char *) &serv_addr, sizeof(serv_addr));
 	serv_addr.sin_family = AF_INET;
 	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
 	serv_addr.sin_port = htons(51234); 

    for(int attempt=0; attempt<max_attempts; ++attempt)
    {
        // Making a connection request to the server
	    if (connect(socket_fd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
	    {
	        // Prints a log info
            char* text; asprintf(&text, "Failed attempt [%d] to connect to master", attempt);
            perror_cont(&logger, text);
            free(text);
        
 		    // Waiting 10 seconds before attempting a new connection
 		    sleep(10);
 		    continue;
	    }
	    
	    return 0;
    }
    
    return -1;
}

int can_activate_spawn_goal(DroneSim d)
{
    return d.spawned != 1;
}

int activate_spawn_goal(DroneSim* d)
{
    while(1)
    {
        // Getting spawn positions from console
        printf("Insert spawn coordinates:\n");
        printf("spawn x: "); scanf("%d", &(d->pos.x));
        printf("spawn y: "); scanf("%d", &(d->pos.y));
        printf("spawn z: "); scanf("%d", &(d->pos.z));
        
        // Trying to spawn the drone in the map
        last_result = send_spawn_message(socket_fd, d->pos.x, d->pos.y, d->pos.z);
        validate_response_or_exit(last_result);
        
        // Check if the drone spawned
        if(last_result == SUCCESS)
        {
            d->spawned = 1;
            map[d->pos.x][d->pos.y] = DRONE;
            return 1;
        }
            
        // Otherwise inform the user that the spawn coordinates are not valid
        printf("Invalid spawn coordinates. "); drone_error(last_result); printf("\n");
    }
}

int can_activate_update_dir_goal(DroneSim d)
{
    // Drone must be not landed
    if(d.landed == 1)
        return 0;
    // If the battery is enough, when the steps are finished    
    if(d.battery-1 > d.pos.z)
        return d.steps <= 0;
    // If the battery is not enough, when not going down
    return d.dir.z >= 0 || d.steps <= 0;
}

int activate_update_dir_goal(DroneSim* d)
{
    // If battery is almost finished, go down
    if(d->battery <= d->pos.z)
    {
        d->dir = (Vec3){0, 0, -1};
        d->steps = 10;
        return 1;
    }
    
    // Otherwise, update direction randomly
    int dir_x, dir_y, dir_z;
    do
    {
        dir_x = random_between(-1, 1);
        dir_y = random_between(-1, 1);
        dir_z = random_between(-1, 1);
    }
    while(dir_x == 0 && dir_y == 0 && dir_z == 0);
    
    d->dir = (Vec3){dir_x, dir_y, dir_z};
    d->steps = random_between(STEPS_MIN, STEPS_MAX);
    return 1;
}

int can_activate_move_goal(DroneSim d)
{
    // When not landed, battery is not 0 and there are steps
    return d.landed == 0 && d.battery > 0 && d.steps > 0;
}

int activate_move_goal(DroneSim* d)
{
    // Asks the master to move in the direction
    last_result = send_move_message(socket_fd, d->dir.x, d->dir.y, d->dir.z);
    validate_response_or_exit(last_result);
    
    // If there is wall marks the wall in the map
    if(last_result == OCCUPIED_POSITION_WALL)
        map[d->pos.x + d->dir.x][d->pos.y + d->dir.y] = WALL;
    
    // If some error occurred, try another direction
    if(last_result != SUCCESS)
    {
        // Resets steps so that another direction is choosen
        d->steps = 0;
        return 0;
    }
    
    // Marks the old position as visited
    map[d->pos.x][d->pos.y] = VISITED;

    // Compute the new position
    d->pos.x += d->dir.x;
    d->pos.y += d->dir.y;
    d->pos.z += d->dir.z;
    // Updates steps and battery
    d->steps--;
    d->battery--;
    
    // Updates map
    map[d->pos.x][d->pos.y] = DRONE;
    return 1;
}

int can_activate_recharge_goal(DroneSim d)
{
    // When landed and the battery is not fully recharged
    return d.landed == 1 && d.battery < BATTERY_MAX;
}

int activate_recharge_goal(DroneSim* d)
{
    d->battery++;
    return 1;
}

int can_activate_em_land_goal(DroneSim d)
{
    // When not landed, battery is almost finished and drone is at ground level
    return d.landed == 0 && d.battery <= 2 && d.pos.z == 0;
}

int activate_em_land_goal(DroneSim* d)
{
    // Asks the master to land at position
    last_result = send_land_message(socket_fd, 1);
    validate_response_or_exit(last_result);
    // Marks the drone as landed
    d->landed = 1;
    d->steps = 0;
    return 1;
}

int can_activate_takeoff_goal(DroneSim d)
{
    // When landed and battery is fully recharged
    return d.landed == 1 && d.battery >= BATTERY_MAX;
}

int activate_takeoff_goal(DroneSim* d)
{
    // Asks the master to take off 
    last_result = send_land_message(socket_fd, 0);
    validate_response_or_exit(last_result);
    // Marks the drone as not landed
    d->landed = 0;
    d->steps = 0;
    return 1;
}

void draw_screen(DroneSim d)
{
    // Clearing the console before drawing it again
    system("clear");
    
    // Writing drone informations on top
    printf("[%s]\n", d.name);
    printf("Executing goal at index: %d\n", d.goal);
    printf("Current position: (%d, %d, %d)\n", d.pos.x, d.pos.y, d.pos.z);
    printf("Current battery: %.1f%%\n", 100*(float)d.battery/BATTERY_MAX);
    printf("Last requested direction: (%d, %d, %d)\n", d.dir.x, d.dir.y, d.dir.z);
    printf("Last request result: "); drone_error(last_result);
    printf("\n");
    
    // Prints the map on the bottom
    int discovered_cells = analyze_and_print_map();
    printf("\n");
    
    // Prints discovered percentage on bottom
    float percent = 100*((float)discovered_cells)/(MAP_SIZE_X*MAP_SIZE_Y);
    printf("Map discovered: %s %.3f%% %s\n", KGRN, percent, KNRM);
    
    fflush(stdout);
}

int analyze_and_print_map()
{
    // How many cells of the map have been discovered
    int discovered_cells = 0;
    
    for(int i = MAP_SIZE_Y - 1; i >= 0; i--)
    {
        for(int j = 0; j < MAP_SIZE_X ; j++)
        {
            char element = map[j][i];
            print_map_element(element);
            
            if(element != EMPTY)
                ++discovered_cells;
        }
        printf("\n");
    }
    
    return discovered_cells;
}

void print_map_element(char element)
{
    if(element == DRONE)
        printf(KBLU "%c" KNRM, DRONE);
    else if(element == VISITED)
        printf(KGRN "%c" KNRM, VISITED);
    else if(element == WALL)
        printf("%c", WALL);
    else
        printf(KYEL "%c" KNRM, EMPTY);
}

void log_drone(DroneSim drone)
{
    char* text;
    asprintf(&text, 
        "Drone: P(%d, %d, %d), D(%d, %d, %d), B: %d, L: %d. Activated task at %d.", 
        drone.pos.x, drone.pos.y, drone.pos.z, drone.dir.x, drone.dir.y, drone.dir.z, 
        drone.battery, drone.landed, drone.goal);
    info(&logger, text, 0); 
    free(text);
}

int random_between(int min, int max)
{
    return min + rand()%(max+1-min);
}

void validate_response_or_exit(int result)
{
    if(result == CONNECTION_ABORTED || result == -1)
        perror_exit(&logger, "Master disconnected. Terminating.");
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
    // Closing socket
    if(close(socket_fd) == -1)
        perror_cont(&logger, "Closing socket");
}
