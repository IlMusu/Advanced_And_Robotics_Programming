#include "./drone_api.h"

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

int send_result(int client_fd, int result)
{
    if(write(client_fd, &result, sizeof(int)) == -1)
        perror("Sending result");
}
