cp -r ./project ./temp
rm -r ./temp/exes
rm ./temp/compile_and_run.sh
cd ./temp
zip -r -q ../project.zip ./*
cd ../
rm -r ./temp
zip -r -q ./assignment.zip ./project.zip ./install.sh
rm -r ./project.zip
