
DRONE_API="./sources/drone_api/drone_api.o"
LOGGER_LIB="./sources/logger/logger.o"

gcc -c ./sources/drone_api/drone_api.c -o $DRONE_API
gcc -c ./sources/logger/logger.c -o $LOGGER_LIB

gcc ./sources/master.c -o ./exes/master -std=gnu99 $DRONE_API $LOGGER_LIB
gcc ./sources/dumb_drone.c -o ./exes/dumb_drone $DRONE_API
