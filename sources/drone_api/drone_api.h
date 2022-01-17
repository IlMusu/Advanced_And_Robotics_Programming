#ifndef ASSIGNEMNT3_DRONE_API
#define ASSIGNEMNT3_DRONE_API

#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#define SPAWN_MESSAGE 0
#define MOVE_MESSAGE 1
#define LANDING_MESSAGE 2

#define SUCCESS 0
#define OUT_OF_BOUNDS_POSITION 1
#define OCCUPIED_POSITION 2
#define NOT_MOVED_WALL 3
#define NOT_MOVED_DRONE 4

void decode_client_message(int id, int client_fd);

int send_spawn_message(int master_fd, int posx, int posy);
int (*handle_spawn_message)(int, int, int);

int send_move_message(int master_fd, int offx, int offy);
int (*handle_move_message)(int, int, int);

int send_landing_message(int master_fd, int landing);
int (*handle_landing_message)(int, int);

#endif //ASSIGNEMNT3_DRONE_API
