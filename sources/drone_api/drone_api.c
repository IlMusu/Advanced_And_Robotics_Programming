#include "./drone_api.h"

void decode_client_message(int id, int client_fd)
{
    int message;
    if(read(client_fd, &message, sizeof(int)) == -1)
        perror("Reading message type");
        
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

int send_landing_message(int master_fd, int landing)
{
    int message = LANDING_MESSAGE;
    if(write(master_fd, &message, sizeof(int)) == -1)
        perror("Sending message type");
        
    if(write(master_fd, &landing, sizeof(int)) == -1)
        perror("Sending landing info");
        
    int result;
    if(read(master_fd, &result, sizeof(int)) == -1)
        perror("Receiving result from master");
        
    return result;
}
