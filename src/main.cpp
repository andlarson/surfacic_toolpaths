/*
    Showcasing the toolpath to surface mesh conversion process.
*/

#include "brep_to_stl.hxx"
#include "brep_builder.hxx"

int main(void)
{
    CylindricalTool tool {3, 1};    
    
    std::vector<Point3D> interpolation_points {{0, 0, 0},
                                               {0, 1, 0},
                                               {1, 1, 0}};

    std::vector<std::pair<uint64_t, Vec3D>> tangents {{{0}, {0, -1, 0}}};

    ToolCurve curve(interpolation_points, tangents);

    ToolPath tool_path(tool, curve);

    mesh_surface(tool_path.tool_path); 
    
    shape_to_stl("test", tool_path.tool_path, "/tmp/test.stl");
}
