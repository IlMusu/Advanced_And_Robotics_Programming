# Creating folder if not existing
mkdir -p $1

# Unzippin into the specified directory
unzip -q project.zip -d $1

# Creating folder to store the executable files
mkdir -p $1/exes/
# Creating folder to store the log files
mkdir -p $1/logfiles

S=$1"/sources"
E=$1"/exes"

DRONE_API=$S/drone_api/drone_api.o
LOGGER=$S/logger/logger.o

#### Compile libraries ####
# -c to create an object file rather than an executable
gcc -c $S/drone_api/drone_api.c -o $DRONE_API
gcc -c $S/logger/logger.c -o $LOGGER

#### Compile sources ####
# (Libraries need to be manually linked)
gcc $S/master/master.c -o $E/master -std=gnu99 $DRONE_API $LOGGER
