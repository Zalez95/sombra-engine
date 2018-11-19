#!/bin/bash
mkdir ./build
cd ./build

cmake ../ -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DSOMBRA_ENGINE_BUILD_TESTS=On -DSOMBRA_ENGINE_BUILD_DOC=On -DCMAKE_EXPORT_COMPILE_COMMANDS=On
make -j 8

cd ../
