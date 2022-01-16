#ifndef ASSIGNEMNT3_DRONE_API
#define ASSIGNEMNT3_DRONE_API

#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define SPAWN_MESSAGE 0
#define MOVE_MESSAGE 1

#define SUCCESS 0
#define OUT_OF_BOUNDS_POSITION 1
#define OCCUPIED_POSITION 2
#define NOT_MOVED_WALL 3
#define NOT_MOVED_DRONE 4

int send_spawn_message(int master_fd, int posx, int posy);
int send_move_message(int master_fd, int offx, int offy);
int send_result(int client_fd, int result);

#endif //ASSIGNEMNT3_DRONE_API
