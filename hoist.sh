# Compiling master process
gcc src/master.c -o master
# Compiling motor process
gcc src/motor.c -o motor
# Compiling command_console process
gcc src/command_console.c -o command_console
# Compiling inspection_console process
gcc src/inspection_console.c -o inspection_console
# Compiling watchdog process
gcc src/watchdog.c -o watchdog

# Executing only master process because
# it will create and control all other
# processes
./master
