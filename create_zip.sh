# Copies the project contents inside a temporary folder ./temp
cp -r ./project ./temp
# Removes the exes directory and then other things
rm -r ./temp/exes
rm -r ./temp/logfiles
rm ./temp/compile_and_run.sh
rm ./temp/sources/dumb_drone.c
rm ./temp/sources/drone_api/drone_api.o
rm ./temp/sources/logger/logger.o
cd ./temp
# Zips the contents of ./temp
zip -r -q ../project.zip ./*
cd ../
# Removes ./temp
rm -r ./temp
# Zips the project zip files + install.sh
zip -r -q ./assignment.zip ./project.zip ./install.sh
# Removes the temporaty project zip files
rm -r ./project.zip
