/*
    Functionality to convert a topology represented as a B-Rep to a surface
        mesh.
*/

#include <fstream>

#include "IMeshTools_Parameters.hxx"
#include "BRepMesh_IncrementalMesh.hxx"
#include "TopExp_Explorer.hxx"
#include "TopoDS.hxx"
#include "Poly_Triangulation.hxx"
#include "BRep_Tool.hxx"

#include "brep_to_stl.hxx"

/*
    Generates a surface mesh on a shape of arbitrary topology. 

    TODO: Under what conditions will this not work?
*/
void mesh_surface(TopoDS_Shape& to_mesh, const double angle, const double deflection)
{
    IMeshTools_Parameters a_mesh_params;
    a_mesh_params.Angle = angle;
    a_mesh_params.Deflection = deflection; 
    a_mesh_params.InParallel = true;

    BRepMesh_IncrementalMesh a_mesher;
    a_mesher.SetShape(to_mesh);
    a_mesher.ChangeParameters() = a_mesh_params;
    a_mesher.Perform();
}

/*
    Iterates over the faces of a shape and writes the content to a .stl file.
    
    Even if the file already exists, it is completely overwritten. 

    Does not write normals to the .stl file.
*/
void shape_to_stl(const std::string solid_name, 
                  const TopoDS_Shape& shape, 
                  const std::string filepath)
{
    std::ofstream f {filepath};  
    assert(f.good());

    f << "solid " << solid_name << std::endl;

    for (TopExp_Explorer aFaceIter(shape, TopAbs_FACE); aFaceIter.More(); aFaceIter.Next())
    {
        const TopoDS_Face& aFace {TopoDS::Face(aFaceIter.Current())};
        TopLoc_Location aLoc;
        const Handle(Poly_Triangulation)& aPolyTri {BRep_Tool::Triangulation(aFace, aLoc)};
        
        if (aPolyTri.IsNull())
            continue;

        for (int aTriIter = 1; aTriIter <= aPolyTri->NbTriangles(); ++aTriIter)
        {
            const Poly_Triangle& aTri = aPolyTri->Triangle(aTriIter);

            // Don't produce normals in the .stl file. Right now, we only care about
            //     shape.
            f << "facet normal 0, 0, 0" << std::endl;
            f << "outer loop" << std::endl;

            f << "vertex " << (aPolyTri->Node(aTri(1))).X() << " " 
                           << (aPolyTri->Node(aTri(1))).Y() << " " 
                           << (aPolyTri->Node(aTri(1))).Z() << std::endl;

            f << "vertex " << (aPolyTri->Node(aTri(2))).X() << " " 
                           << (aPolyTri->Node(aTri(2))).Y() << " " 
                           << (aPolyTri->Node(aTri(2))).Z() << std::endl;

            f << "vertex " << (aPolyTri->Node(aTri(3))).X() << " " 
                           << (aPolyTri->Node(aTri(3))).Y() << " " 
                           << (aPolyTri->Node(aTri(3))).Z() << std::endl;

            f << "end loop" << std::endl;
            f << "end facet" << std::endl;
        }
    }

    f << "endsolid " << solid_name;
}
