
DRONE_API="./sources/drone_api/drone_api.o"

gcc -c ./sources/drone_api/drone_api.c -o $DRONE_API

gcc ./sources/master.c -o ./exes/master $DRONE_API
gcc ./sources/dumb_drone.c -o ./exes/dumb_drone $DRONE_API
