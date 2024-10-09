// Standard library.
#include <vector>
#include <fstream>

// Third party.

// OCCT.
#include "gp_Pnt.hxx"
#include "gp_Vec.hxx"
#include "gp.hxx"
#include "gp_Dir.hxx"
#include "Geom_Plane.hxx"
#include "Geom_RectangularTrimmedSurface.hxx"
#include "GeomLib_Tool.hxx"
#include "BRepBuilderAPI_MakeFace.hxx"
#include "BRepBuilderAPI_MakeEdge.hxx"
#include "BRepBuilderAPI_MakeWire.hxx"
#include "BRepOffsetAPI_MakePipe.hxx"
#include "BRepMesh_IncrementalMesh.hxx"
#include "BRep_Tool.hxx"
#include "BRepTools.hxx"
#include "BRepLib_ToolTriangulatedShape.hxx"
#include "TopoDS_Edge.hxx"
#include "TopoDS_Face.hxx"
#include "TopoDS_Shape.hxx"
#include "TopoDS_Wire.hxx"
#include "TopoDS.hxx"
#include "TopoDS_HShape.hxx"
#include "IMeshTools_Parameters.hxx"
#include "TopExp_Explorer.hxx"
#include "Poly_Triangulation.hxx"

// Library public.
#include "toolpath.hxx"
#include "tool_curve.hxx"
#include "tool_profile.hxx"
#include "geometric_primitives.hxx"

// Library private.
#include "util_p.hxx"
#include "tool_curve_p.hxx"
#include "glfw_occt_view_p.hxx"


/* 
   ****************************************************************************
                           File Local Declarations 
   ****************************************************************************
*/ 

static Handle(Geom_RectangularTrimmedSurface) construct_rect_trimmed_surface(gp_Vec normal, 
                                                                     gp_Pnt bottom_point,
                                                                     double width, 
                                                                     double
                                                                     height);

static gp_Dir compute_average_vec(const std::vector<gp_Vec>& vecs);

/* 
   ****************************************************************************
                           File Local Definitions 
   ****************************************************************************
*/ 

/*
    Constructs a rectangular trimmed surface with the center point of the bottom
        edge at a passed point in space, with normal lying on the xy-plane, and
        with centerline extending in the +Z direction.

    Arguments:
        normal:       Normal to the face. 
        bottom_point: Location of the bottom point of the rectangular face in
                          space.
        width:        Width of the face. 
        height:       Height of the face.
*/
static Handle(Geom_RectangularTrimmedSurface) construct_rect_trimmed_surface(gp_Vec normal, 
                                                                             const gp_Pnt bottom_point,
                                                                             const double width, 
                                                                             const double height)
{
    // An infinite plane. 
    Handle(Geom_Plane) plane {new Geom_Plane(bottom_point, normal)};
    
    // Construct the basis on the plane.
    normal.Normalize();
    double divisor {sqrt(pow(normal.X(), 2) + pow(normal.Y(), 2))};
    gp_Vec v1 {-normal.Y()/divisor, normal.X()/divisor, 0};
    gp_Vec v2 {gp_Vec(gp::DZ())};

    // Compute the four points that make up the face.
    // These four points lie on the infinite plane.
    // The points are ordered cyclically. 
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
    std::array<gp_Pnt, VERTICES_PER_RECTANGLE> points {p1, p2, p3, p4};

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

    // DEBUG!?
    /*
    Handle(Geom_RectangularTrimmedSurface) rect_trimmed_surface {new Geom_RectangularTrimmedSurface(plane, )}
    */
    Handle(Geom_RectangularTrimmedSurface) test {new
    Geom_RectangularTrimmedSurface(plane, 5, -.1, -.5, 2, true, true)};

    return test;
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
   ****************************************************************************
                                    ToolPath 
   ****************************************************************************
*/ 

/*
    Defines a toolpath. A toolpath is just a tool profile swept through space. 

    This function assumes that:
        (1) The rotational axis of symmetry of the tool profile points in the +z
                direction along the entire curve. The normal to the tool profile
                is parallel to the tangent of the curve along the entire curve.
        (2) The curve is G1 continuous.
        (3) The curve describes the path taken in space by the center point of
                the bottom of the tool.
        (4) The toolpath does not intersect itself. If it does, the behavior is
                undefined.

    Arguments:
        curve:          Curve describing the path that the center point of the
                            bottom face of the tool takes in space.
        profile:        The cross section of the tool.
        display_result: Causes windows to be created showing the results of
                            toolpath creation. 
*/
ToolPath::ToolPath(const ToolCurve& curve,
                   const CylindricalTool& profile,
                   const bool display_result)
{
    const Handle(Geom_BSplineCurve) bspline {curve.pimpl->curve};

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
    gp_Vec tangent;
    gp_Pnt unused;
    bspline->D1(start_parameter, unused, tangent);

    const auto profile_surface {construct_rect_trimmed_surface(tangent, start, profile.radius * 2, profile.height)};
    
    // Extract the parameteric bounds of the rectangle.
    std::array<double, RECTANGLE_PARAMETRIC_BND_CNT> parameters;
    profile_surface->Bounds(parameters[0], parameters[1], parameters[2],
                            parameters[3]);

    // Build topology from the geometry. 
    const BRepBuilderAPI_MakeFace face_topology_builder {profile_surface,
                                                         parameters[0],
                                                         parameters[1],
                                                         parameters[2],
                                                         parameters[3],
                                                         FP_EQUALS_TOLERANCE};
    assert(face_topology_builder.IsDone());
    const TopoDS_Face profile_topology {face_topology_builder.Face()};

    if (display_result)
    {
        std::vector<TopoDS_Shape> shapes {profile_topology, curve_wire_topology};
        show_shapes(shapes); 
    }

    // Do the sweep.
    const BRepOffsetAPI_MakePipe pipe_topology_builder {curve_wire_topology, profile_topology};
    const TopoDS_Shape pipe_topology {pipe_topology_builder.Pipe().Shape()}; 

    /* TODO: Add Caps.

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
    */

    if (display_result)
    {
        std::vector<TopoDS_Shape> shapes {pipe_topology};
        show_shapes(shapes);     
    }

    this->tool_path = pipe_topology;
}

/*
    Generates a surface mesh on the toolpath topology.

    If the input topology is weird, then this function can fail/take a long
        time. There is no formal definition of weird and therefore this
        function cannot test for weirdness, so to be safe, you should visualize
        the topology you are trying to mesh before calling this function.
        For example, if the topology contains self intersections, mesh
        generation may not fail, but it will take tons of time.
    
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
    BRepTools::Clean(this->tool_path, true);

    IMeshTools_Parameters mesh_params;
    mesh_params.Angle = angle;
    mesh_params.Deflection = deflection; 
    mesh_params.InParallel = true;

    BRepMesh_IncrementalMesh mesher;
    mesher.SetShape(this->tool_path);
    mesher.ChangeParameters() = mesh_params;
    mesher.Perform();

    for (TopExp_Explorer face_iter {this->tool_path, TopAbs_FACE}; face_iter.More(); face_iter.Next())
    {
        const TopoDS_Face face {TopoDS::Face(face_iter.Current())};
        TopLoc_Location loc;
        const Handle(Poly_Triangulation) poly_tri {BRep_Tool::Triangulation(face, loc)};
        
        // If a face could not be triangulated, something has gone wrong during meshing. 
        assert(!poly_tri.IsNull());
        
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
    If the toolpath hasn't already been meshed, then the behavior of this
        function is undefined.
    
    Arguments:
        solid_name: The desired name of the solid in the .stl file.
        file_path:  Absolute path to the file to write to. Need not already
                        exist.
    
    Returns:
        None.
*/
void ToolPath::shape_to_stl(const std::string solid_name, 
                            const std::string filepath)
{
    std::ofstream f {filepath};  
    assert(f.good());

    f << "solid " << solid_name << std::endl;

    for (TopExp_Explorer face_it {this->tool_path, TopAbs_FACE}; face_it.More(); face_it.Next())
    {
        const TopoDS_Face face {TopoDS::Face(face_it.Current())};
        TopLoc_Location loc;
        const Handle(Poly_Triangulation) poly_tri {BRep_Tool::Triangulation(face, loc)};
        
        assert(!poly_tri.IsNull());

        for (int tri_it {1}; tri_it <= poly_tri->NbTriangles(); ++tri_it)
        {
            const Poly_Triangle& tri = poly_tri->Triangle(tri_it);
            
            // Average the vertex normals to compute the face normal. 
            int v1_idx, v2_idx, v3_idx;
            tri.Get(v1_idx, v2_idx, v3_idx);
            const gp_Vec face_normal {compute_average_vec({poly_tri->Normal(v1_idx), 
                                                           poly_tri->Normal(v2_idx), 
                                                           poly_tri->Normal(v3_idx)})};
            f << "    " << "facet normal " << face_normal.X() << " " << 
                                              face_normal.Y() << " " << 
                                              face_normal.Z() << std::endl;
            f << "        " << "outer loop" << std::endl;

            f << "            " << "vertex " << (poly_tri->Node(tri(1))).X() << " " 
                                << (poly_tri->Node(tri(1))).Y() << " " 
                                << (poly_tri->Node(tri(1))).Z() << std::endl;

            f << "            " << "vertex " << (poly_tri->Node(tri(2))).X() << " " 
                                << (poly_tri->Node(tri(2))).Y() << " " 
                                << (poly_tri->Node(tri(2))).Z() << std::endl;

            f << "            " << "vertex " << (poly_tri->Node(tri(3))).X() << " " 
                                << (poly_tri->Node(tri(3))).Y() << " " 
                                << (poly_tri->Node(tri(3))).Z() << std::endl;

            f << "        " << "endloop" << std::endl;
            f << "    " << "endfacet" << std::endl;
        }
    }

    f << "endsolid " << solid_name;
}
