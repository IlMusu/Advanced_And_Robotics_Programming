# Defining important directories
SOURCES_DIR="./sources"
EXES_DIR="./exes"
LOGFILES_DIR="./logs"

# Defining libraries object files locations
DRONE_API="$SOURCES_DIR/drone_api/drone_api.o"
LOGGER_LIB="$SOURCES_DIR/logger/logger.o"

# Creats folder to store the executable files
mkdir -p $EXES_DIR
# Creating folder to store the log files
mkdir -p $LOGFILES_DIR

# Compiling libraries
gcc -c $SOURCES_DIR/drone_api/drone_api.c -o $DRONE_API
gcc -c $SOURCES_DIR/logger/logger.c       -o $LOGGER_LIB

# Compiling sources with requires libraries and commands
gcc $SOURCES_DIR/master/master.c       -o $EXES_DIR/master       $DRONE_API $LOGGER_LIB -std=gnu99
gcc $SOURCES_DIR/drone_ms3/drone_ms3.c -o $EXES_DIR/drone_ms3    $DRONE_API $LOGGER_LIB

sh ./run.sh
