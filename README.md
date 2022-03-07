# Level Renderer
A graphics renderer made from scratch in vulkan.

## Installation
1. Download and install CMake and Vulkan API.
2. Download and unzip the repository files or clone using the command ```git clone https://github.com/AdamStagg/Level-Renderer.git```
3. Open command prompt and navigate to the directory with the files.
4. Run the CMake command: ```cmake -S ./ -B ./build```
5. Open the solution generated in the build folder
6. Change the start-up project from ALL-BUILD to Level-Renderer

## Features
Scene loading from text file (generated in blender)\
Hot-swapping scenes by using Control + O\
Camera movement with keyboard (WASD, Left Shift, Space), mouse, and controller (left and right sticks, left and right bumpers)\
Directional and ambient lighting with specular reflections\
Fully instanced draw calls to optimize the GPU\
Camera collision with the environment by using a bounding volume hierarchy tree

## Assets
```https://opengameart.org/content/modular-terrain```
