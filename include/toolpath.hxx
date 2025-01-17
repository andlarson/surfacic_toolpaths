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
class Line;
class ArcOfCircle;
class InterpolatedCurve;

class ToolPath
{
    TopoDS_Shape toolpath_shape_union;

    void add_shape(const TopoDS_Shape& s);

    TopoDS_Shape curved_toolpath(const Curve& curve,
                                 const CylindricalTool& profile,
                                 const bool display=false) const;

    TopoDS_Shape linear_toolpath(const Line& curve,
                                 const CylindricalTool& profile,
                                 const bool display=false) const;

public:
    ToolPath(const std::tuple<std::vector<Line>, 
                              std::vector<ArcOfCircle>,
                              std::vector<InterpolatedCurve>> compound,
             const CylindricalTool& profile,
             const bool display=false);

    void mesh_surface(const double angle, const double deflection);

    void shape_to_stl(const std::string solid_name, 
                      const std::string filepath) const;
};

struct CylindricalTool
{
    double radius;
    double height;
};

class Path
{
public:
    virtual ~Path() = 0;
};

class Curve : public Path
{
    friend class ToolPath;

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
                const Point3D& interior_point);
};

class Line : public Path
{
    friend class ToolPath; 

    Vec3D line;
    Point3D start_point;

public:
    Line(const Point3D& start_point, const Vec3D& line);
};
