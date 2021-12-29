#### Compile libraries ####
# -c to create an object file rather than an executable
gcc -c ./sources/libraries/utils.c -o ./sources/libraries/utils.o
gcc -c ./sources/libraries/logger.c -o ./sources/libraries/logger.o

#### Compile sources ####
# Libraries need to be manually linked
gcc ./sources/producer_unpipe/producer_unpipe.c -o ./exes/producer_unpipe ./sources/libraries/utils.o ./sources/libraries/logger.o
gcc ./sources/consumer_unpipe/consumer_unpipe.c -o ./exes/consumer_unpipe ./sources/libraries/utils.o ./sources/libraries/logger.o

gcc ./sources/producer_napipe/producer_napipe.c -o ./exes/producer_napipe ./sources/libraries/utils.o ./sources/libraries/logger.o
gcc ./sources/consumer_napipe/consumer_napipe.c -o ./exes/consumer_napipe ./sources/libraries/utils.o ./sources/libraries/logger.o

gcc ./sources/producer_cirbuf/producer_cirbuf.c -o ./exes/producer_cirbuf ./sources/libraries/utils.o ./sources/libraries/logger.o -pthread -lrt
gcc ./sources/consumer_cirbuf/consumer_cirbuf.c -o ./exes/consumer_cirbuf ./sources/libraries/utils.o ./sources/libraries/logger.o -pthread -lrt

gcc ./sources/producer_socket/producer_socket.c -o ./exes/producer_socket ./sources/libraries/utils.o ./sources/libraries/logger.o
gcc ./sources/consumer_socket/consumer_socket.c -o ./exes/consumer_socket ./sources/libraries/utils.o ./sources/libraries/logger.o

gcc ./sources/master/master.c -o ./exes/master ./sources/libraries/utils.o ./sources/libraries/logger.o

#### execute program ####
./exes/master
