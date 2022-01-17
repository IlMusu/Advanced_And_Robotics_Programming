#include "./drone_api.h"

int decode_client_message(int id, int client_fd)
{
    // Before using the file_descriptor, checks if the client is still connected
    int error = 0;
    socklen_t len = sizeof(error);
    if(getsockopt (client_fd, SOL_SOCKET, SO_ERROR, &error, &len) == -1)
        return -1;
    if(error != 0)
        return error;
        
    // Gets the message type
    int message;
    if(read(client_fd, &message, sizeof(int)) == -1)
        perror("Reading message type");
        
    // Decodes message and calls handler
    int result;
    switch(message)
    {
        case SPAWN_MESSAGE:
        {
            int posx, posy;
            if(read(client_fd, &posx, sizeof(int)) == -1)
                perror("Reading x position");
            if(read(client_fd, &posy, sizeof(int)) == -1)
                perror("Reading y position");
                
            result = handle_spawn_message(id, posx, posy);
            break;
        }
        case MOVE_MESSAGE:
        {
            int offx, offy;
            if(read(client_fd, &offx, sizeof(int)) == -1)
                perror("Reading x position");
            if(read(client_fd, &offy, sizeof(int)) == -1)
                perror("Reading y position");
                
            result = handle_move_message(id, offx, offy);
            break;
        }
        case LANDING_MESSAGE:
        {
            int landing;
            if(read(client_fd, &landing, sizeof(int)) == -1)
                perror("Reading x position");
                
            result = handle_landing_message(id, landing);
            break;
        }
        default:
            break;
    }
    
    if(write(client_fd, &result, sizeof(int)) == -1)
        perror("Receiving result from master");
        
    return 0;
}

int send_spawn_message(int master_fd, int posx, int posy)
{
    int message = SPAWN_MESSAGE;
    if(write(master_fd, &message, sizeof(int)) == -1)
        perror("Sending message type");
        
    if(write(master_fd, &posx, sizeof(int)) == -1)
        perror("Sending x position");
    
    if(write(master_fd, &posy, sizeof(int)) == -1)
        perror("Sending y position");
        
    int result;
    if(read(master_fd, &result, sizeof(int)) == -1)
        perror("Receiving result from master");
        
    return result;
}

int send_move_message(int master_fd, int offx, int offy)
{
    int message = MOVE_MESSAGE;
    if(write(master_fd, &message, sizeof(int)) == -1)
        perror("Sending message type");
        
    if(write(master_fd, &offx, sizeof(int)) == -1)
        perror("Sending x offset");
    
    if(write(master_fd, &offy, sizeof(int)) == -1)
        perror("Sending y offset");
        
    int result;
    if(read(master_fd, &result, sizeof(int)) == -1)
        perror("Receiving result from master");
        
    return result;
}

int send_landing_message(int master_fd, int landed)
{
    int message = LANDING_MESSAGE;
    if(write(master_fd, &message, sizeof(int)) == -1)
        perror("Sending message type");
        
    if(write(master_fd, &landed, sizeof(int)) == -1)
        perror("Sending landing info");
        
    int result;
    if(read(master_fd, &result, sizeof(int)) == -1)
        perror("Receiving result from master");
        
    return result;
}

void drone_error(int error)
{
    switch(error)
    {
        case SUCCESS:
            printf("Success\n");
            break;
        case OUT_OF_BOUNDS_POSITION:
            printf("Target position is not inside map\n");
            break;
        case OCCUPIED_POSITION_WALL:
            printf("Target position is occupied by wall\n");
            break;
        case OCCUPIED_POSITION_DRONE:
            printf("Target position is occupied by drone\n");
            break;
        case DRONE_NOT_SPAWNED:
            printf("Drone is not yet spawned in map\n");
            break;
        case DRONE_IS_LANDED:
            printf("Drone is currently landed\n");
            break;
        default:
            printf("Print not implemented for %d\n", error);
    }
}
