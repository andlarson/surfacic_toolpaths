// Standard library.
#include <vector>
#include <fstream>

// Third party.

// OCCT.
#include "gp_Pnt.hxx"
#include "gp_Vec.hxx"
#include "gp.hxx"
#include "gp_Dir.hxx"
#include "gp_Ax2.hxx"
#include "Geom_Plane.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepOffsetAPI_MakePipe.hxx"
#include "BRepAlgoAPI_Fuse.hxx"
#include "BRepPrimAPI_MakeCylinder.hxx"
#include "BRepPrimAPI_MakePrism.hxx"
#include "BRepMesh_IncrementalMesh.hxx"
#include "BRep_Tool.hxx"
#include "BRepTools.hxx"
#include "BRepLib_ToolTriangulatedShape.hxx"
#include "TopoDS_Edge.hxx"
#include "TopoDS_Face.hxx"
#include "TopoDS_Shape.hxx"
#include "TopoDS_Wire.hxx"
#include "TopoDS.hxx"
#include "IMeshTools_Parameters.hxx"
#include "TopExp_Explorer.hxx"
#include "Poly_Triangulation.hxx"

// Library public.
#include "toolpath.hxx"

// Library private.
#include "util_p.hxx"
#include "glfw_occt_view_p.hxx"

/* 
   ****************************************************************************
                           File Local Declarations 
   ****************************************************************************
*/ 

static TopoDS_Face construct_rect_face(const gp_Dir normal, 
                                       const gp_Pnt bottom_point, 
                                       const double width, 
                                       const double height);

static gp_Dir compute_average_vec(const std::vector<gp_Vec>& vecs);

/* **************************************************************************** */


/* 
   ****************************************************************************
                           File Local Definitions 
   ****************************************************************************
*/ 

/*
    Constructs a face with the center point of the bottom edge at a passed point
        in space and with normal lying in some direction. 

    Requires that:
        (1) The normal lies in the XY-plane.

    Assumes that:
        (1) The face extends in the +Z direction.

    Notes for future improvement:
        A point, a direction, a width and a height, taken together, are
            insufficient to describe the location of a rectangle in space.
            In particular, the rectangle (with dimensions width and height)
            could lie anywhere on the plane described by the point and direction.
            As such, in order to make this function more general (i.e. get rid
            of the assumption about the normal lying in XY-plane and get rid of
            the assumption about the face extending in +Z direction), the caller 
            would need to provide more information - potentially the location
            of each vertex in space.
        Ideally it would be possible to construct the face as a geometry, and
            convert it to a topology exactly when necessary. Turns out that's not
            so easy. Some code below attempts to make a face via a rectangular
            trimmed surface. This isn't viable because (as best I understand it)
            rectangular trimmed surfaces only permit rectangles that are
            axis-aligned to be represented. This isn't sufficiently general.

    Arguments:
        normal:       Normal to the face. Must lie in the xy-plane.
        bottom_point: Location of the bottom point of the rectangular face in
                          space.
        width:        Width of the face. 
        height:       Height of the face.
*/
static TopoDS_Face construct_rect_face(const gp_Dir normal, 
                                       const gp_Pnt bottom_point, 
                                       const double width, 
                                       const double height)
{
    // An infinite plane. 
    const Handle(Geom_Plane) plane {new Geom_Plane(bottom_point, normal)};
    
    // Construct the two dimensional basis on the plane.
    // The second axis is hardcoded to point in the +Z direction.
    // The first axis is orthogonal to the passed normal and the +Z direction.
    //     Therefore, it lies in the XY-plane.
    const double divisor {sqrt(pow(normal.X(), 2) + pow(normal.Y(), 2))};
    const gp_Dir v1 {-normal.Y()/divisor, normal.X()/divisor, 0};
    const gp_Dir v2 {gp_Vec(gp::DZ())};

    // Compute the four points that make up the face.
    // These four points lie on the infinite plane.
    // The points are ordered cyclically. 
    const gp_Pnt p1 {bottom_point.X() + (width / 2) * v1.X(),
                     bottom_point.Y() + (width / 2) * v1.Y(),
                     bottom_point.Z() + (width / 2) * v1.Z()};
    const gp_Pnt p2 {p1.X() + height * v2.X(),
                     p1.Y() + height * v2.Y(),
                     p1.Z() + height * v2.Z()};
    const gp_Pnt p3 {p2.X() - width * v1.X(),
                     p2.Y() - width * v1.Y(),
                     p2.Z() - width * v1.Z()};
    const gp_Pnt p4 {p3.X() - height * v2.X(),
                     p3.Y() - height * v2.Y(),
                     p3.Z() - height * v2.Z()};
    const std::array<gp_Pnt, VERTICES_PER_RECTANGLE> points {p1, p2, p3, p4};

    std::vector<BRepBuilderAPI_MakeEdge> face_edges;
    for (auto it {points.begin()}; it != points.end(); ++it)
    {
        if (it == points.end() - 1)
        {
            BRepBuilderAPI_MakeEdge edge {*it, *(points.begin())};
            assert(edge.IsDone());
            face_edges.push_back(edge);
        }
        else
        {
            BRepBuilderAPI_MakeEdge edge {*it, *(it + 1)};
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

    const BRepBuilderAPI_MakeFace face {plane, wire_for_face};
    assert(face.IsDone());
    
    /* 
    // Compute the parameterization at the four points. 
    std::array<std::pair<double, double>, VERTICES_PER_RECTANGLE> point_parameters;
    for (int i {0}; i < points.size(); ++i)
    {
        std::pair<double, double> parameters;
        assert(GeomLib_Tool::Parameters(plane, points[i], FP_EQUALS_TOLERANCE, 
                                        parameters.first, parameters.second));
        point_parameters[i] = parameters;
    }
    
    // The four points each have parameters (u, v) associated with them.
    //     However, because the points are arranged in a rectangle, there
    //     are only 2 distinct u values and 2 distinct v values. This step
    //     extracts those 4 distinct u and v values so they can be used
    //     for trimming.
    std::array<double, 2> u_values; 
    std::array<double, 2> v_values; 

    Handle(Geom_RectangularTrimmedSurface) rect_trimmed_surface {new Geom_RectangularTrimmedSurface(plane, )}
    */

    return face.Face();
}

static gp_Dir compute_average_vec(const std::vector<gp_Vec>& vecs)
{
    assert(vecs.size() > 0);
    
    gp_Vec res {0, 0, 0};
    for (const gp_Dir& vec : vecs)
    {
        res += vec;
    }
    res *= (1. / vecs.size());

    return res;
}

/*
    Builds a cylinder at a point with axis of rotation in the +Z direction. 
    
    Assumes that:
        (1) The axis of rotation of the cylinder should be in the +Z direction.
    
    Arguments:
        loc:    The location of the center point of the bottom face of the 
                    cylinder.
        radius: Radius of the cylinder.
        height: Height of the cylinder.
*/
static TopoDS_Shape build_vertical_cylinder(const gp_Pnt& loc,
                                            const double radius,
                                            const double height)
{
    gp_Ax2 start_point_axis {};
    start_point_axis.SetLocation(loc);
    BRepPrimAPI_MakeCylinder start_cap_topology_builder {start_point_axis, radius, height};
    const TopoDS_Shape cyl {start_cap_topology_builder.Shape()};
    return cyl;
}

/* **************************************************************************** */

/*
    See callees for documentation.
*/
ToolPath::ToolPath(const Curve& curve,
                   const CylindricalTool& profile,
                   const bool display)
{
    const TopoDS_Shape curved {curved_toolpath(curve, profile, display)};
    add_shape(curved);
}

/*
    See callees for documentation.
*/
ToolPath::ToolPath(const Line& line,
                   const CylindricalTool& profile,
                   const bool display)
{
    const TopoDS_Shape linear {linear_toolpath(line, profile, display)};
    add_shape(linear);
}

/*
    See callees for documentation.
*/
ToolPath::ToolPath(const std::pair<const Line&, const Curve&> compound,
                   const CylindricalTool& profile,
                   const bool display)
{
    const TopoDS_Shape linear {linear_toolpath(compound.first, profile, display)};
    add_shape(linear);

    const TopoDS_Shape curved {curved_toolpath(compound.second, profile, display)};
    add_shape(curved);

    if (display)
    {
        const std::vector<TopoDS_Shape> shapes {this->toolpath_shape_union};
        show_shapes(shapes);     
    }
}

/*
    Generates a surface mesh on the toolpath topology.

    If the input topology is weird, then this function can fail/take a long
        time. There is no formal definition of weird and therefore this
        function cannot test for weirdness, so to be safe, you should visualize
        the topology you are trying to mesh before calling this function.
        For example, if the topology contains self intersections, mesh
        generation may not fail, but it will take tons of time.
    There is no guarantee that the produced surface mesh is any good. The caller
        is responsible for checking its quality. This function is best effort.
    
    Arguments:
        angle:      Maximum angular deflection allowed when generating surface mesh. 
        deflection: Maximum linear deflection allowed when generating surface mesh.
    
    Return:
        None.
*/
void ToolPath::mesh_surface(const double angle, 
                            const double deflection)
{
    // Get rid of any previous mesh associated with this toolpath.
    BRepTools::Clean(this->toolpath_shape_union, true);

    IMeshTools_Parameters mesh_params;
    mesh_params.Angle = angle;
    mesh_params.Deflection = deflection; 
    mesh_params.InParallel = true;

    BRepMesh_IncrementalMesh mesher;
    mesher.SetShape(this->toolpath_shape_union);
    mesher.ChangeParameters() = mesh_params;
    mesher.Perform();

    for (TopExp_Explorer face_iter {this->toolpath_shape_union, TopAbs_FACE}; face_iter.More(); face_iter.Next())
    {
        const TopoDS_Face face {TopoDS::Face(face_iter.Current())};
        TopLoc_Location loc;
        const Handle(Poly_Triangulation) poly_tri {BRep_Tool::Triangulation(face, loc)};

        if (!poly_tri.IsNull())
            BRepLib_ToolTriangulatedShape::ComputeNormals(face, poly_tri);
    }
}

/*
    Writes the meshed toolpath to a file. Even if the file already exists, it is 
        completely overwritten. Per-face normals are included in the .stl file.
        Each per-face normal is computed by averaging whatever vertex normals
        are associated with the vertices of the face.
    See https://www.fabbers.com/tech/STL_Format for the closest thing to a
        standardization of the STL format.

    Assumes:
        (1) The caller is OK with the file being overwritten if it already exists.
        (2) The toolpath has already been meshed in a satisfactory way.
        
    Arguments:
        solid_name: The desired name of the solid in the .stl file.
        file_path:  Absolute path to the file to write to. 
    
    Returns:
        None.
*/
void ToolPath::shape_to_stl(const std::string solid_name, 
                            const std::string filepath) const
{
    std::ofstream f {filepath};  

    // Ensure that ample precision is used when writing to the .stl. 
    f.precision(FP_WRITE_PRECISION);

    assert(f.good());

    f << "solid " << solid_name << std::endl;

    for (TopExp_Explorer face_it {this->toolpath_shape_union, TopAbs_FACE}; face_it.More(); face_it.Next())
    {
        const TopoDS_Face face {TopoDS::Face(face_it.Current())};
        TopLoc_Location loc;
        const Handle(Poly_Triangulation) poly_tri {BRep_Tool::Triangulation(face, loc)};
        
        // It's not this function's resposibility to deal with faces that are
        //     not triangulated.
        if (!poly_tri.IsNull())
            for (int tri_it {1}; tri_it <= poly_tri->NbTriangles(); ++tri_it)
            {
                const Poly_Triangle& tri {poly_tri->Triangle(tri_it)};
                
                // Average the vertex normals to compute the face normal. 
                int v1_idx, v2_idx, v3_idx;
                tri.Get(v1_idx, v2_idx, v3_idx);
                const gp_Vec face_normal {compute_average_vec({poly_tri->Normal(v1_idx), poly_tri->Normal(v2_idx), poly_tri->Normal(v3_idx)})};

                // Write the face normals.
                f << FOUR_SPACES << "facet normal " << face_normal.X() << " " <<  face_normal.Y() << " " << face_normal.Z() << std::endl;
                
                // Write the vertices.
                f << EIGHT_SPACES << "outer loop" << std::endl;
                for (int i {1}; i <= VERTICES_PER_TRIANGLE; ++i)
                {
                    f << TWELVE_SPACES << "vertex " << (poly_tri->Node(tri(i))).X() << " " << (poly_tri->Node(tri(i))).Y() << " " << (poly_tri->Node(tri(i))).Z() << std::endl;
                }
                f << EIGHT_SPACES << "endloop" << std::endl;
                f << FOUR_SPACES << "endfacet" << std::endl;
            }
    }

    f << "endsolid " << solid_name;
}

/*
    Adds a shape to the shape compound that makes up this toolpath.

    Arguments:
        s: The shape to add to the compound.
    
    Returns:
        None.
*/
void ToolPath::add_shape(const TopoDS_Shape& s)
{
    if (this->toolpath_shape_union.IsNull())
    {
        this->toolpath_shape_union = s;
    }
    else
    {
        BRepAlgoAPI_Fuse toolpath_union {this->toolpath_shape_union, s};
        assert(!toolpath_union.HasErrors());
        this->toolpath_shape_union = toolpath_union.Shape();
    }
}

/*
    Sweeps a profile along a curve and adds caps, forming a curved toolpath.
    
    Requires that:
        (1) The tangent at the first point on the curve lies on a plane parallel
                to the XY-plane. 
        (2) The tangent at the last point on the curve lies on a plane parallel
                to the XY-plane.
        (3) The curve is G1 continuous.
        (4) The toolpath does not intersect itself. 

    Assumes that:
        (1) The rotational axis of symmetry of the tool profile points in the +Z
                direction at the first point on the curve. This affects how the
                caps are built.
        (2) The curve describes the path taken in space by the center point of
                the bottom of the tool.
    
    Notes for future improvement:
        The angle between the tool profile and the curve is maintained along
            the entirety of the curve. This does not mean that the rotational axis
            of symmetry of the tool points in the +Z direction along the
            entirety of the curve. TODO: Not clear to me how the angle evolves
            along the curve. Question submitted on OCCT forum.
        The axii of rotation should not necessarily be in the +Z direction. To
            make this work for more general curves, it would be necessary to figure
            out the correct axii of rotation based the topology produced by the
            sweep. One approach would be to query every face part of the sweep 
            topology, find the faces that intersect the start and end points,
            and use the geometry of the faces (their shape and normal) to compute
            the correct axis of rotation and the correct sweep angles.
        It would be awfully nice to be able to check if the result of sweeping
            the profile is a closed topology. Unfortunately, the IsClosed()
            member function returns false even when the topology is closed.
            Maybe BRepOffsetAPI_MakePipe doesn't update the Closed flag?  Also,
            checking if the topology is a solid via TopoDS's ShapeType member
            function is also ineffective.

    Arguments:
        curve:   Curve describing the path that the center point of the
                     bottom face of the tool takes in space.
        profile: The cross section of the tool.
        display: Causes windows to be created showing the results of
                     toolpath creation. 
    
    Return:
        The shape resulting from extruding the profile along the curve.
*/
TopoDS_Shape ToolPath::curved_toolpath(const Curve& curve,
                                       const CylindricalTool& profile,
                                       const bool display) const
{
    const Handle(Geom_BSplineCurve) bspline {curve.representation};

    // Build topology from the geometry.
    BRepBuilderAPI_MakeEdge edge_topology_builder {bspline};
    assert(edge_topology_builder.IsDone());
    const TopoDS_Edge curve_edge_topology {edge_topology_builder.Edge()};

    BRepBuilderAPI_MakeWire wire_topology_builder {curve_edge_topology};
    assert(wire_topology_builder.IsDone());
    const TopoDS_Wire curve_wire_topology {wire_topology_builder.Wire()};
    
    // Extract the start and end points to build the caps. 
    const gp_Pnt start {bspline->StartPoint()};
    const gp_Pnt end {bspline->EndPoint()};
    
    // Figure out the vector tangent to the curve at the start point of the
    //     curve.
    const double start_parameter {bspline->FirstParameter()};
    gp_Vec tangent_start;
    gp_Pnt unused;
    bspline->D1(start_parameter, unused, tangent_start);

    // The tangent vector must lie in the XY-plane.
    assert(tangent_start.Z() < FP_EQUALS_TOLERANCE);

    const TopoDS_Face profile_topology {construct_rect_face(tangent_start, start, profile.radius * 2, profile.height)};
    
    if (display)
    {
        const std::vector<TopoDS_Shape> shapes {profile_topology, curve_wire_topology};
        show_shapes(shapes); 
    }

    // Do the sweep.
    const BRepOffsetAPI_MakePipe pipe_topology_builder {curve_wire_topology, profile_topology};
    const TopoDS_Shape pipe_topology {pipe_topology_builder.Pipe().Shape()}; 
    
    // Build the cylinders that act as the start and end caps of the tool path. 
    // Assumes that caps should have axis of rotation in +Z direction.
    TopoDS_Shape start_cap {build_vertical_cylinder(start, profile.radius, profile.height)};
    TopoDS_Shape end_cap {build_vertical_cylinder(end, profile.radius, profile.height)};

    BRepAlgoAPI_Fuse pipe_topology_with_start_cap {pipe_topology, start_cap};
    assert(!pipe_topology_with_start_cap.HasErrors());
    BRepAlgoAPI_Fuse pipe_topology_with_both_caps {pipe_topology_with_start_cap.Shape(), end_cap};
    assert(!pipe_topology_with_both_caps.HasErrors());

    if (display)
    {
        const std::vector<TopoDS_Shape> shapes {pipe_topology_with_both_caps};
        show_shapes(shapes);     
    }

    return pipe_topology_with_both_caps;
}

/*
    Sweeps a profile along a line and adds caps, forming a linear toolpath.
    
    Assumes that:
        (1) The rotational axis of symmetry of the tool profile points in the +Z
                direction. This affects how the caps are built.
        (2) The curve describes the path taken in space by the center point of
                the bottom of the tool.

    Arguments:
        line:    Line describing the path that the center point of the
                     bottom face of the tool takes in space.
        profile: The cross section of the tool.
        display: Causes windows to be created showing the results of
                     toolpath creation. 

    Return:
        The shape resulting from extruding the profile along the line.
*/
TopoDS_Shape ToolPath::linear_toolpath(const Line& line,
                                       const CylindricalTool& profile,
                                       const bool display) const
{
    const gp_Vec path {line.line[0], line.line[1], line.line[2]}; 
    const gp_Pnt start {line.start_point[0], line.start_point[1], line.start_point[2]};
    const gp_Pnt end {start.Translated(path)};
    const TopoDS_Face profile_topology {construct_rect_face(path, start, profile.radius * 2, profile.height)};

    if (display)
    {
        // Need to build the topology for the line here. 
        BRepBuilderAPI_MakeEdge edge_topology_builder {start, end};
        assert(edge_topology_builder.IsDone());

        const std::vector<TopoDS_Shape> shapes {profile_topology, edge_topology_builder.Edge()};
        show_shapes(shapes); 
    }
    
    // Do the sweep.
    BRepPrimAPI_MakePrism prism_topology_builder {profile_topology, path};
    assert(prism_topology_builder.IsDone());
    const TopoDS_Shape& prism_topology {prism_topology_builder.Shape()};

    // Build the cylinders that act as the start and end caps of the tool path. 
    // Assumes that caps should have axis of rotation in +Z direction.
    TopoDS_Shape start_cap {build_vertical_cylinder(start, profile.radius, profile.height)};
    TopoDS_Shape end_cap {build_vertical_cylinder(end, profile.radius, profile.height)};

    BRepAlgoAPI_Fuse prism_topology_with_start_cap {prism_topology, start_cap};
    assert(!prism_topology_with_start_cap.HasErrors());
    BRepAlgoAPI_Fuse prism_topology_with_both_caps {prism_topology_with_start_cap.Shape(), end_cap};
    assert(!prism_topology_with_both_caps.HasErrors());

    if (display)
    {
        const std::vector<TopoDS_Shape> shapes {prism_topology_with_both_caps};
        show_shapes(shapes); 
    }
    
    return prism_topology_with_both_caps;
}
