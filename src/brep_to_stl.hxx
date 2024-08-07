#include "TopoDS_Shape.hxx"

void mesh_surface(TopoDS_Shape& to_mesh, 
                  const double angle=.5, 
                  const double deflection=.01);

void shape_to_stl(const std::string solid_name, 
                  const TopoDS_Shape& shape, 
                  const std::string filepath);
