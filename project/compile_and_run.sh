#### Compile libraries ####
# -c to create an object file rather than an executable
gcc -c ./sources/libraries/utils.c -o ./sources/libraries/utils.o

#### Compile sources ####
# Libraries need to be manually linked
gcc ./sources/unnamed_pipe/producer_unpipe.c -o ./exes/producer_unpipe ./sources/libraries/utils.o
gcc ./sources/unnamed_pipe/consumer_unpipe.c -o ./exes/consumer_unpipe ./sources/libraries/utils.o

gcc ./sources/named_pipe/producer_napipe.c -o ./exes/producer_napipe ./sources/libraries/utils.o
gcc ./sources/named_pipe/consumer_napipe.c -o ./exes/consumer_napipe ./sources/libraries/utils.o

gcc ./sources/circular_buffer/producer_cirbuf.c -o ./exes/producer_cirbuf ./sources/libraries/utils.o -pthread -lrt
gcc ./sources/circular_buffer/consumer_cirbuf.c -o ./exes/consumer_cirbuf ./sources/libraries/utils.o -pthread -lrt

gcc ./sources/socket/producer_socket.c -o ./exes/producer_socket ./sources/libraries/utils.o
gcc ./sources/socket/consumer_socket.c -o ./exes/consumer_socket ./sources/libraries/utils.o

gcc ./sources/master/master.c -o ./exes/master ./sources/libraries/utils.o

#### execute program ####
./exes/master
