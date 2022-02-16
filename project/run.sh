SOURCES_DIR="./sources"
EXES_DIR="./exes"
LOGFILES_DIR="./logs"

rm -r $LOGFILES_DIR
mkdir -p $LOGFILES_DIR

# Terminals are launched for each process
gnome-terminal -- "$EXES_DIR/master" "$LOGFILES_DIR/log.txt"
gnome-terminal -- "$EXES_DIR/drone_ms3" "$LOGFILES_DIR/log.txt"

