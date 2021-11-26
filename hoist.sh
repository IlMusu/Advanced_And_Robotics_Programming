# Compiling master process
gcc src/master.c -o exe/master
# Compiling motor process
gcc src/motor.c -o exe/motor
# Compiling command_console process
gcc src/command_console.c -o exe/command_console
# Compiling inspection_console process
gcc src/inspection_console.c -o exe/inspection_console
# Compiling watchdog process
gcc src/watchdog.c -o exe/watchdog

# Executing only master process because
# it will create and control all other
# processes
./exe/master
