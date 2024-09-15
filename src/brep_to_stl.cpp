/*
    Functionality to convert a topology represented as a B-Rep to a surface
        mesh.
*/

// Standard library.
#include <fstream>

// Third party.
#include "IMeshTools_Parameters.hxx"
#include "BRepMesh_IncrementalMesh.hxx"
#include "TopExp_Explorer.hxx"
#include "TopoDS.hxx"
#include "Poly_Triangulation.hxx"
#include "BRep_Tool.hxx"
#include "BRepTools.hxx"
#include "BRepLib_ToolTriangulatedShape.hxx"
#include "gp_Dir.hxx"

// Library public.
#include "include/brep_to_stl.hxx"

/* 
   ============================================================================
                           File Local Declarations 
   ============================================================================ 
*/ 

static gp_Dir compute_average_vec(const std::vector<gp_Vec>& vecs);

/* 
   ============================================================================
                           File Local Declarations 
   ============================================================================ 
*/ 

/*
    Computes the average vector.
*/
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
   ============================================================================
                           Meshing and STL Generation 
   ============================================================================ 
*/ 

/*
    Generates a surface mesh on a shape of arbitrary topology. Adds vertex surface
        normals to the surface mesh.
    
    Arguments:
        to_mesh:    The BRep to generate a surface mesh on. If this shape already has 
                        an underlying surface mesh, it is removed and the process
                        is started anew.
        angle:      Maximum angular deflection allowed when generating surface mesh. 
        deflection: Maximum linear deflection allowed when generating surface mesh.

    Return:
        None. The resulting surface mesh and vertex surface normals are stored in the
            shape.
*/
void mesh_surface(const TopoDS_Shape& to_mesh, 
                  const double angle, 
                  const double deflection)
{
    // Make sure the faces of the shape haven't already been triangulated. We want
    //     a clean, fresh mesh.
    BRepTools::Clean(to_mesh, true);

    IMeshTools_Parameters mesh_params;
    mesh_params.Angle = angle;
    mesh_params.Deflection = deflection; 
    mesh_params.InParallel = true;

    BRepMesh_IncrementalMesh mesher;
    mesher.SetShape(to_mesh);
    mesher.ChangeParameters() = mesh_params;
    mesher.Perform();

    for (TopExp_Explorer face_iter {to_mesh, TopAbs_FACE}; face_iter.More(); face_iter.Next())
    {
        const TopoDS_Face& face = TopoDS::Face(face_iter.Current());
        TopLoc_Location loc;
        const Handle(Poly_Triangulation)& poly_tri = BRep_Tool::Triangulation(face, loc);
        
        // If a face is not triangulated, something has gone wrong during meshing. 
        assert(!poly_tri.IsNull());
        
        BRepLib_ToolTriangulatedShape::ComputeNormals(face, poly_tri);
    }
}

/*
    Iterates over the triangular faces of a shape and writes the content to a 
        .stl file. Even if the file already exists, it is completely overwritten. 
        Per-face normals are included in the .stl file. Each per-face normal is
        computed by averaging whatever vertex normals are associated with the
        vertices of the face.
    
    Arguments:
        solid_name: The desired name of the solid in the .stl file.
        shape:      The shape to extract the triangular surface mesh from.
        file_path:  Absolute path to the file to write to. Need not already
                        exist.
    
    Returns:
        None.
*/
void shape_to_stl(const std::string solid_name, 
                  const TopoDS_Shape& shape, 
                  const std::string filepath)
{
    std::ofstream f {filepath};  
    assert(f.good());

    f << "solid " << solid_name << std::endl;

    for (TopExp_Explorer face_it(shape, TopAbs_FACE); face_it.More(); face_it.Next())
    {
        const TopoDS_Face& face {TopoDS::Face(face_it.Current())};
        TopLoc_Location loc;
        const Handle(Poly_Triangulation)& poly_tri {BRep_Tool::Triangulation(face, loc)};
        
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
            f << "facet normal " << face_normal.X() << " " << 
                                    face_normal.Y() << " " << 
                                    face_normal.Z() << std::endl;
            f << "outer loop" << std::endl;

            f << "vertex " << (poly_tri->Node(tri(1))).X() << " " 
                           << (poly_tri->Node(tri(1))).Y() << " " 
                           << (poly_tri->Node(tri(1))).Z() << std::endl;

            f << "vertex " << (poly_tri->Node(tri(2))).X() << " " 
                           << (poly_tri->Node(tri(2))).Y() << " " 
                           << (poly_tri->Node(tri(2))).Z() << std::endl;

            f << "vertex " << (poly_tri->Node(tri(3))).X() << " " 
                           << (poly_tri->Node(tri(3))).Y() << " " 
                           << (poly_tri->Node(tri(3))).Z() << std::endl;

            f << "end loop" << std::endl;
            f << "end facet" << std::endl;
        }
    }

    f << "endsolid " << solid_name;
}
