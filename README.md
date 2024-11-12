A tool to convert a description of a toolpath in G-Code-esque format to a
surface mesh.

### Depends on:
```
OpenCascade Geometry Kernel
GLFW3
```
The CMake build system generator must be able to find these dependencies using
the config mode of `find_package()`.

### Building and Installing With CMake:
```
mkdir build/
cd build/
cmake ..
ccmake .
cmake --build .
cmake --install .
```
