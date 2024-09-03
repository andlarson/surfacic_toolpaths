// ***** LIBRARY PUBLIC *****

#pragma once 

// Standard library.
#include <array>
#include <vector>
#include <stdint.h>

// Third party.
#include "TopoDS_Shape.hxx"

typedef std::array<double, 3> Point3D;
typedef std::array<double, 3> Vec3D;
typedef std::array<double, 2> Point2D;

class CylindricalTool
{
public:
    double height;
    double radius;
};

class ToolPath
{
public:
    TopoDS_Shape tool_path;

    ToolPath(const CylindricalTool& tool, 
             const std::vector<Point3D>& points,
             const std::vector<std::pair<uint64_t, Vec3D>>& tangents);
};
