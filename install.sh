# Unzippin into the specified directory
unzip -q project.zip -d $1

# Creating folder to store the executable files
mkdir $1/exes/

#### Compile libraries ####
# -c to create an object file rather than an executable
gcc -c $1/sources/libraries/utils.c -o $1/sources/libraries/utils.o

#### Compile sources ####
# (Libraries need to be manually linked)
# Compiling sources for the unnamed pipe comunication
gcc $1/sources/unnamed_pipe/producer_unpipe.c -o $1/exes/producer_unpipe $1/sources/libraries/utils.o
gcc $1/sources/unnamed_pipe/consumer_unpipe.c -o $1/exes/consumer_unpipe $1/sources/libraries/utils.o

# Compiling sources for the named pipe comunication
gcc $1/sources/named_pipe/producer_napipe.c -o $1/exes/producer_napipe $1/sources/libraries/utils.o
gcc $1/sources/named_pipe/consumer_napipe.c -o $1/exes/consumer_napipe $1/sources/libraries/utils.o

# Compiling sources for the circular buffer comunication
gcc $1/sources/circular_buffer/producer_cirbuf.c -o $1/exes/producer_cirbuf $1/sources/libraries/utils.o -pthread -lrt
gcc $1/sources/circular_buffer/consumer_cirbuf.c -o $1/exes/consumer_cirbuf $1/sources/libraries/utils.o -pthread -lrt

# Compiling sources for the socket comunication
gcc $1/sources/socket/producer_socket.c -o $1/exes/producer_socket $1/sources/libraries/utils.o
gcc $1/sources/socket/consumer_socket.c -o $1/exes/consumer_socket $1/sources/libraries/utils.o

# Compiling sources for the master process
gcc $1/sources/master/master.c -o $1/exes/master $1/sources/libraries/utils.o
