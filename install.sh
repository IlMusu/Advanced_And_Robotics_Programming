# Unzippin into the specified directory
unzip -q project.zip -d $1

# Creating folder to store the executable files
mkdir $1/exes/

# Compiling master process
gcc $1/sources/master/master.c -o $1/exes/master
# Compiling motor process
gcc $1/sources/motor/motor.c -o $1/exes/motor
# Compiling command_console process
gcc $1/sources/command_console/command_console.c -o $1/exes/command_console
# Compiling inspection_console process
gcc $1/sources/inspection_console/inspection_console.c -o $1/exes/inspection_console
# Compiling watchdog process
gcc $1/sources/watchdog/watchdog.c -o $1/exes/watchdog
