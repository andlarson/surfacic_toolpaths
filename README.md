A tool to convert descriptions of toolpaths in G-Code-esque format to surface
meshes.

Depends on Open Cascade Technology's geometry kernel.

WARNING: The curve interpolation technique does not take the tangents specified
by the user into account. However, the first tangent specified by the user is
used to orient the face to be swept along the curve. When the interpolated curve
is known to be straight, it's possible for the user to guess the tangent of the
interpolated curve at the first point. This means that, right now, this program 
can do straight toolpaths, but it cannot do curved toolpaths.

Building With CMake:

```
mkdir build/
cd build/
cmake ..
cmake --build .
```
