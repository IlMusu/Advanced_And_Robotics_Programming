cat \
project_description.txt <(echo) \
src/master/master.txt <(echo) \
src/libraries/libraries.txt <(echo) \
src/unnamed_pipe/unnamed_pipe.txt <(echo) \
src/named_pipe/named_pipe.txt <(echo) \
src/socket/socket.txt <(echo) \
src/circular_buffer/circular_buffer.txt <(echo) \
| more -d -c -p
