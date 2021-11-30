# Creating folder to store the executable files
mkdir -p ./exes

# Compiling master process
gcc ./sources/master/master.c -o ./exes/master
# Compiling motor process
gcc ./sources/motor/motor.c -o ./exes/motor
# Compiling command_console process
gcc ./sources/command_console/command_console.c -o ./exes/command_console
# Compiling inspection_console process
gcc ./sources/inspection_console/inspection_console.c -o ./exes/inspection_console
# Compiling watchdog process
gcc ./sources/watchdog/watchdog.c -o ./exes/watchdog

# Executing only master process: it will create 
# and control all other processes
./exes/master
