# cmake_demo
a demo project showing basic cmake usage

## Usage
` cmake -DCMAKE_BUILD_TYPE=Debug ../ `
` cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32 ../ `
` CFLAGS=-m32 CXXFLAGS=-m32 cmake -DCMAKE_BUILD_TYPE=Debug ../ `
