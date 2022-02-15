SOURCES_DIR="./sources"
EXES_DIR="./exes"
LOGFILES_DIR="./logs"

# Terminals are launched for each process
gnome-terminal -- "$EXES_DIR/master" "$LOGFILES_DIR/log_master.txt"
gnome-terminal -- "$EXES_DIR/drone_ms3" "$LOGFILES_DIR/log_ms3.txt"

