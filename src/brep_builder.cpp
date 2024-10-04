/*
    Functionality to build an OCCT B-Rep data structure that occupies
        the space traced by the movement of a machine tool. 
*/

// Standard library.
#include <limits>
#include <cmath>

// Third party.
#include "TopoDS_Wire.hxx"
#include "TopoDS_Face.hxx"
#include "GeomAPI_Interpolate.hxx"
#include "Geom_BSplineCurve.hxx"
#include "Geom_Plane.hxx"
#include "BRepPrimAPI_MakeCylinder.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepOffsetAPI_MakePipe.hxx"
#include "BRepAlgoAPI_Fuse.hxx"

// Library public.
#include "include/brep_builder.hxx"

// Library private.
#include "src/include/tool_curve.hxx"
#include "src/include/glfw_occt_view.hxx"

/* 
   ============================================================================
                            File Local Declarations
   ============================================================================ 
*/ 

static TopoDS_Shape sweep_tool(const ToolCurve& curve,
                               const CylindricalTool& tool_volume);
static TopoDS_Shape make_cylinder(const gp_Pnt& center, 
                                  const double radius, 
                                  const double height);
static TopoDS_Face construct_face(gp_Vec normal, 
                                  const gp_Pnt& bottom_point,
                                  const double width, 
                                  const double height);
static TopoDS_Wire interpolate(const ToolCurve& curve);

/* 
   ============================================================================
                                    ToolPath 
   ============================================================================ 
*/ 

/*
    Defines a toolpath. A toolpath is composed of a tool, occupying some volume,
        moving along a curve in space. 

    Does not allow the tool to be oriented arbitrarily with respect to the curve.
        If full generality was permitted, then:
        (1) There would be no restrictions on the curve.
        (2) The volume occupied by the tool could intersect with the
                the curve describing the path at an arbitrary point within 
                the volume of the tool. 
        (3) The volume could be oriented arbitrarily with respect to curve,
                and the angle could even vary along the curve.

    However, full generality is not permitted. 
    Instead, this function assumes that:
        (1) The curve lives on a plane parallel to the xy axis.
        (2) The rotational axis of symmetry of the tool points in the +z
                direction and the orientation of the tool with respect
                to the curve does not change. 
        (3) The tool sits on top of the first point of the curve. 

    This function guarantees that:
        (1) The resulting curve is G1 continuous.

    Arguments:
        tool                : The shape of the tool.
        interpolation_points: Points to be interpolated. The points must all 
                                  lie on a single plane that has constant z value.
                                  The points must be specified in the order coherent
                                  with the movement of the tool.
        tangents:             A collection of (idx, tangent vector) pairs. Each (idx,
                                  tangent vector) pair specifies the tangent at the point
                                  at index idx in the list of points to be interpolated. 
                              When a curve is interpolated between the points, these tangents
                                  will be honored. 
                              A tangent need not be specified for every point. 
                                  However, a tangent must be specified for the 
                                  first point that composes the curve.
                              All tangent vectors must have zero z component. 
                                  The tool curve must lie on a single plane.
        display_result:       When enabled, causes two windows to be created
                                  showing different intermediate B-Rep constructions.

    Returns:
        None.
*/
ToolPath::ToolPath(const CylindricalTool& tool, 
                   const std::vector<Point3D>& interpolation_points,
                   const std::vector<std::pair<uint64_t, Vec3D>>& tangents,
                   const bool display_result)
{
    const ToolCurve curve {interpolation_points, tangents};

    const TopoDS_Wire interpolation {interpolate(curve)};

    const TopoDS_Face tool_face {construct_face(curve.tangents->First(), 
                                                curve.points_to_interpolate->First(), 
                                                tool.radius * 2, 
                                                tool.height)};

    if (display_result)
    {
        std::vector<TopoDS_Shape> shapes {interpolation, tool_face};
        show_shapes(shapes); 
    }
    
    // Do the sweep.
    BRepOffsetAPI_MakePipe pipe(interpolation, tool_face);
    // assert(!pipe.ErrorOnSurface());
    
    // Build the cylinders that act as the start and end caps of the tool path. 
    // These cylinders have rotation axii of symmetry that point in the +z 
    //     direction.
    TopoDS_Shape start_cap {make_cylinder(curve.points_to_interpolate->First(), 
                                          tool.radius, 
                                          tool.height)};
    TopoDS_Shape end_cap {make_cylinder(curve.points_to_interpolate->Last(), 
                                        tool.radius, 
                                        tool.height)};
    
    // Perform a boolean union between the sweep and the start and end caps of
    //     the tool path. 
    BRepAlgoAPI_Fuse res1(pipe, start_cap);
    assert(res1.HasErrors() == false);
    BRepAlgoAPI_Fuse res2(res1.Shape(), end_cap);
    assert(res2.HasErrors() == false);
    
    // Record the resulting tool path.
    tool_path = res2;
    
    if (display_result)
    {
        std::vector<TopoDS_Shape> shapes {res2};
        show_shapes(shapes);     
    }
};

/* 
   ============================================================================
                             File Local Definitions
   ============================================================================ 
*/ 

/*
    Converts specification of curve to wire via interpolation. The result is
        guaranteed to be G1 continuous.
*/
static TopoDS_Wire interpolate(const ToolCurve& curve)
{
    GeomAPI_Interpolate interpolation(curve.points_to_interpolate, false, std::numeric_limits<double>::min());
    
    interpolation.Load(*(curve.tangents), curve.tangent_bools);

    interpolation.Perform();
    interpolation.IsDone();
    Handle(Geom_BSplineCurve) interpolated_curve {interpolation.Curve()};

    assert(interpolated_curve->IsCN(1));
    
    BRepBuilderAPI_MakeEdge edge(interpolated_curve);
    BRepBuilderAPI_MakeWire wire(edge.Edge());

    return wire.Wire();
}

/*
    Constructs a rectangular face under some assumptions. This function is not
        capable of constructing a rectangular face with arbitrary orientation
        in space.

    See toolpath.xopp for more details.

    Arguments:
        normal:       Normal to the face. Must be parallel to the xy plane.
        bottom_point: Location of the bottom point of the rectangular face in
                          space.
        width:        Width of the face. 
        height:       Height of the face.
*/
static TopoDS_Face construct_face(gp_Vec normal, 
                                  const gp_Pnt& bottom_point,
                                  const double width, 
                                  const double height)
{
    // An infinite plane. 
    Handle(Geom_Plane) plane = new Geom_Plane(bottom_point, normal);
    
    // Construct the basis for the face. 
    normal.Normalize();
    double divisor {sqrt(pow(normal.X(), 2) + pow(normal.Y(), 2))};
    gp_Vec v1(-normal.Y()/divisor, normal.X()/divisor, 0);
    gp_Vec v2(0, 0, 1);

    // Compute the four points that make up the face.
    gp_Pnt p1(bottom_point.X() + (width / 2) * v1.X(),
              bottom_point.Y() + (width / 2) * v1.Y(),
              bottom_point.Z() + (width / 2) * v1.Z());
    gp_Pnt p2(p1.X() + height * v2.X(),
              p1.Y() + height * v2.Y(),
              p1.Z() + height * v2.Z());
    gp_Pnt p3(p2.X() - width * v1.X(),
              p2.Y() - width * v1.Y(),
              p2.Z() - width * v1.Z());
    gp_Pnt p4(p3.X() - height * v2.X(),
              p3.Y() - height * v2.Y(),
              p3.Z() - height * v2.Z());
    
    // Use the four points to construct a wire to bound the face.
    const std::vector<gp_Pnt> face_points {p1, p2, p3, p4};
    
    std::vector<BRepBuilderAPI_MakeEdge> face_edges;
    for (auto it {face_points.begin()}; it != face_points.end(); ++it)
    {
        if (it == face_points.end() - 1)
        {
            BRepBuilderAPI_MakeEdge edge (*it, *(face_points.begin()));
            assert(edge.IsDone());
            face_edges.push_back(edge);
        }
        else
        {
            BRepBuilderAPI_MakeEdge edge (*it, *(it + 1));
            assert(edge.IsDone());
            face_edges.push_back(edge);
        }
    }

    BRepBuilderAPI_MakeWire wire_for_face;
    for (auto it {face_edges.begin()}; it != face_edges.end(); ++it)
    {
        wire_for_face.Add(*it);
        assert(wire_for_face.IsDone());
    }

    BRepBuilderAPI_MakeFace face(plane, wire_for_face);
    assert(face.IsDone());

    return face.Face();
}

/*
    Builds a cylinder on top of an arbitrary point in space, oriented in the 
        +z direction.
*/
static TopoDS_Shape make_cylinder(const gp_Pnt& center, 
                                  const double radius, 
                                  const double height)
{
    gp_Ax2 axis(center, gp::DZ());
    BRepPrimAPI_MakeCylinder cylinder(axis, radius, height);
    return cylinder.Shape();
}
