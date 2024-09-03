/*
    Showcasing the toolpath to surface mesh conversion process.
*/

#include <fstream>
#include <vector>

#include "brep_to_stl.hxx"
#include "brep_builder.hxx"

int main()
{
    CylindricalTool tool {.5, .1};    
    
    std::vector<Point3D> interpolation_points {{1, 0, 0},
                                               {1, 1, 0},
                                               {0, 1, 0}};
    std::vector<std::pair<uint64_t, Vec3D>> tangents {{0, {0, 1, 0}}, {1, {1, 0, 0}}, {2, {-1, 0, 0}}};
    
    ToolPath tool_path(tool, interpolation_points, tangents);
    std::cout << "Finished B-Rep construction of tool path shape." << std::endl;

    mesh_surface(tool_path.tool_path, .5, .00001); 
    std::cout << "Finished meshing surface." << std::endl;
    
    shape_to_stl("test", tool_path.tool_path, "/Users/andrewlarson/Downloads/test.stl");
}
