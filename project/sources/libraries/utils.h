#ifndef ASSIGNEMNT2_UTILS_LIB
#define ASSIGNEMNT2_UTILS_LIB

#include "utils.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>

void fill_randomly(char* array, int length);
int write_time_in_file(struct timespec time, char* file);
int get_time_from_file(struct timespec *time, char* file);

#endif //ASSIGNEMNT2_UTILS_LIBR
