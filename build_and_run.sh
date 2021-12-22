#### Compile libraries ####
# -c to create an object file rather than an exetable
gcc -c ./src/libraries/utils.c -o ./src/libraries/utils.o

#### Compile sources ####
# Libraries need to be manually linked
gcc ./src/producer_unpipe.c -o ./exe/producer_unpipe ./src/libraries/utils.o
gcc ./src/consumer_unpipe.c -o ./exe/consumer_unpipe ./src/libraries/utils.o

gcc ./src/producer_napipe.c -o ./exe/producer_napipe ./src/libraries/utils.o
gcc ./src/consumer_napipe.c -o ./exe/consumer_napipe ./src/libraries/utils.o

gcc ./src/producer_cirbuf.c -o ./exe/producer_cirbuf ./src/libraries/utils.o -pthread -lrt
gcc ./src/consumer_cirbuf.c -o ./exe/consumer_cirbuf ./src/libraries/utils.o -pthread -lrt

gcc ./src/producer_socket.c -o ./exe/producer_socket ./src/libraries/utils.o
gcc ./src/consumer_socket.c -o ./exe/consumer_socket ./src/libraries/utils.o

gcc ./src/master.c -o ./exe/master ./src/libraries/utils.o

#### Execute program ####
./exe/master
