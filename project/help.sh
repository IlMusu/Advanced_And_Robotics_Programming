cat \
project_description.txt <(echo) \
sources/master/master.txt <(echo) \
sources/libraries/utils.txt <(echo) \
sources/libraries/logger.txt <(echo) \
sources/producer_unpipe/producer_unpipe.txt <(echo) \
sources/consumer_unpipe/consumer_unpipe.txt <(echo) \
sources/producer_napipe/producer_napipe.txt <(echo) \
sources/consumer_napipe/consumer_napipe.txt <(echo) \
sources/producer_socket/producer_socket.txt <(echo) \
sources/consumer_socket/consumer_socket.txt <(echo) \
sources/producer_cirbuf/producer_cirbuf.txt <(echo) \
sources/consumer_cirbuf/consumer_cirbuf.txt <(echo) \
| more -d -c -p
