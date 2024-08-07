/*
    Showcasing the toolpath to surface mesh conversion process.
*/

#include <fstream>
#include <vector>

#include "include/brep_to_stl.hxx"
#include "include/brep_builder.hxx"

int main(void)
{
    CylindricalTool tool {1, .2};    
    
    std::vector<Point3D> interpolation_points {{1, 0, 3},
                                               {2, 0, 3},
                                               {3, 0, 3}};

    std::vector<std::pair<uint64_t, Vec3D>> tangents {{{0}, {1, 0, 0}}};

    ToolCurve curve(interpolation_points, tangents);
    
    ToolPath tool_path(tool, curve);
    std::cout << "Finished B-Rep construction of tool path shape." << std::endl;

    mesh_surface(tool_path.tool_path, .5, .00001); 
    std::cout << "Finished meshing surface." << std::endl;
    
    shape_to_stl("test", tool_path.tool_path, "/Users/andrewlarson/Downloads/test.stl");
}
