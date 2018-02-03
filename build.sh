mkdir ./build
cd ./build

cmake ../ -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DFAZE_ENGINE_BUILD_TESTS=On -DCMAKE_EXPORT_COMPILE_COMMANDS=On
make -j 8

cd ../
