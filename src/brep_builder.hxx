#include <array>
#include <vector>
#include <stdint.h>

#include "TColgp_HArray1OfPnt.hxx"
#include "TColStd_HArray1OfBoolean.hxx"
#include "TColgp_HArray1OfVec.hxx"

#include "TopoDS_Shape.hxx"

typedef std::array<double, 3> Point3D;
typedef std::array<double, 3> Vec3D;
typedef std::array<double, 2> Point2D;

/*
    Doesn't orient the cylinder in space. Simply specifies the geometry of the
        tool.
*/
class CylindricalTool
{
public:

    int height;
    int radius;
};

/*
    The ToolCurve simply describes the curve that the tool moves along in space.
        Assumptions are not made about the orientation and position of the tool
        with respect to the curve.
*/
class ToolCurve
{
public:

    Handle(TColgp_HArray1OfPnt) points_to_interpolate;
    Handle(TColStd_HArray1OfBoolean) tangent_bools;
    Handle(TColgp_HArray1OfVec) tangents;

    ToolCurve(const std::vector<Point3D>& points,
              const std::vector<std::pair<uint64_t, Vec3D>>& tangents);
};

class ToolPath
{
public:
    TopoDS_Shape tool_path;

    ToolPath(const CylindricalTool& tool, 
             const ToolCurve& curve);
};
