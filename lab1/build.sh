#!/bin/bash

cd ..

mkdir -p build
cd build

cmake ..
make

if [ $? -eq 0 ]; then
    echo "Сборка завершена."

    ./daemon_example ../lab1/config.txt
else
    echo "Сборка завершилась с ошибкой."
fi