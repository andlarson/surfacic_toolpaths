#pragma once

// Standard library.
#include <vector>

// Third party.
#include "TopoDS_Shape.hxx"
#include "Geom_BSplineCurve.hxx"

// Library public.
#include "geometric_primitives.hxx"

class Curve;
class CylindricalTool;

class ToolPath
{
    TopoDS_Shape tool_path;

public:
    ToolPath(const Curve& curve,
             const CylindricalTool& profile,
             const bool display_result);

    void mesh_surface(const double angle, const double deflection);

    void shape_to_stl(const std::string solid_name, 
                      const std::string filepath);
};

struct CylindricalTool
{
    double radius;
    double height;
};

class Curve
{
    friend ToolPath::ToolPath(const Curve& curve,
                              const CylindricalTool& profile,
                              const bool display_result);
protected:
    Handle(Geom_BSplineCurve) representation; 

public:
    virtual ~Curve() = 0;
};

class InterpolatedCurve : public Curve
{
public:
    InterpolatedCurve(const std::vector<Point3D>& interpolation_points,
                      const std::vector<std::pair<uint64_t, Vec3D>>& tangents);
};

class ArcOfCircle : public Curve
{
public:
    ArcOfCircle(const std::pair<Point3D, Point3D>& arc_endpoints,
                const Point3D& center, const double radius);
};

