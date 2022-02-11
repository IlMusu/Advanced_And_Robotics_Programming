# Creating folder if not existing
mkdir -p $1

# Unzippin into the specified directory
unzip -q project.zip -d $1

# Creating folder to store the executable files
mkdir -p $1/exes/

S=$1"/sources"
E=$1"/exes"

#### Compile libraries ####
# -c to create an object file rather than an executable
gcc -c $S/libraries/utils.c -o $S/libraries/utils.o
gcc -c $S/libraries/logger.c -o $S/libraries/logger.o

#### Compile sources ####
# (Libraries need to be manually linked)
# Compiling sources for the unnamed pipe comunication
gcc $S/producer_unpipe/producer_unpipe.c -o $E/producer_unpipe $S/libraries/utils.o $S/libraries/logger.o
gcc $S/consumer_unpipe/consumer_unpipe.c -o $E/consumer_unpipe $S/libraries/utils.o $S/libraries/logger.o

# Compiling sources for the named pipe comunication
gcc $S/producer_napipe/producer_napipe.c -o $E/producer_napipe $S/libraries/utils.o $S/libraries/logger.o
gcc $S/consumer_napipe/consumer_napipe.c -o $E/consumer_napipe $S/libraries/utils.o $S/libraries/logger.o

# Compiling sources for the circular buffer comunication
gcc $S/producer_cirbuf/producer_cirbuf.c -o $E/producer_cirbuf $S/libraries/utils.o $S/libraries/logger.o -pthread -lrt
gcc $S/consumer_cirbuf/consumer_cirbuf.c -o $E/consumer_cirbuf $S/libraries/utils.o $S/libraries/logger.o -pthread -lrt

# Compiling sources for the socket comunication
gcc $S/producer_socket/producer_socket.c -o $E/producer_socket $S/libraries/utils.o $S/libraries/logger.o
gcc $S/consumer_socket/consumer_socket.c -o $E/consumer_socket $S/libraries/utils.o $S/libraries/logger.o

# Compiling sources for the master process
gcc $S/master/master.c -o $E/master $S/libraries/utils.o $S/libraries/logger.o
