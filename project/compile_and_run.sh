# Getting libraries executables locations
DRONE_API="./sources/drone_api/drone_api.o"
LOGGER_LIB="./sources/logger/logger.o"

# Compiling libraries
gcc -c ./sources/drone_api/drone_api.c -o $DRONE_API
gcc -c ./sources/logger/logger.c -o $LOGGER_LIB

# Compiling sources with requires libraries and commands
gcc ./sources/master/master.c -o ./exes/master -std=gnu99 $DRONE_API $LOGGER_LIB
gcc ./sources/dumb_drone.c -o ./exes/dumb_drone $DRONE_API

gnome-terminal -- "./exes/master" "./logfiles/log_master.txt"
gnome-terminal -- "./exes/dumb_drone" "10" "10"
