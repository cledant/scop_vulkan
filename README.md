# scop
![screenshot](./screenshots/scop_vulkan.png?raw=true)

scop is a 3D model viewer made with Vulkan, C++20 and GLFW.

## Features

scop can load Wavefront .obj files and is controllable with GUI  

## Compiling

Make sure you have the libraries by running `git submodule init && git submodule update`.  
You may compile `scop` binary by running `mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=Release && make -j8`.

## Usage

Run `./scop`.  
There is no CLI options.  