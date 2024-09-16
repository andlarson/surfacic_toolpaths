A tool to convert descriptions of toolpaths in G-Code-esque format to surface
meshes.

Depends on Open Cascade Technology's geometric kernel. GLFW is also a dependency
if visualization is desired.

### WARNING

This tool has only been tested on macOS and currently includes some machine-
specific behavior in the build system. This machine-specific behavior will be
removed soon.

### Building With CMake:

```
mkdir build/
cd build/
cmake ..
cmake --build .
```
