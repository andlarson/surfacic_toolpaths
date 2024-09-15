/*
    Showcasing the toolpath to surface mesh conversion process.
*/

#include <fstream>
#include <vector>
#include <string>

#include "brep_to_stl.hxx"
#include "brep_builder.hxx"

using namespace std;

/* 
   ============================================================================
                            File Local Declarations
   ============================================================================ 
*/ 

static void run_tests(const string& dir);

/* 
   ============================================================================
                             File Local Definitions
   ============================================================================ 
*/ 

/*
    Runs a suite of totally non-comprehensive tests.
*/
static void run_tests(const string& dir)
{
    // The format of each test is (test name, interpolation points, tangents, tool size).
    const vector<tuple<string, vector<Point3D>, vector<pair<uint64_t, Vec3D>>, pair<double, double>>> test_suite =
    {
        // Ought to succeed.
        {"corner", {{1, 0, 0}, {1, 1, 0}, {0, 1, 0}}, {{0, {0, 1, 0}}, {1, {-1, 0, 0}}, {2, {-1, 0, 0}}}, {1, .2}},
        {"straight_line", {{0, 0, 0}, {1, 0, 0}, {2, 0, 0}}, {{0, {1, 0, 0}}, {1, {1, 0, 0}}, {2, {1, 0, 0}}}, {1, .2}},
        {"zigzag", {{0, 0, 0}, {1, 1, 0}, {0, 2, 0}, {3, 3, 0}, {0, 4, 0}}, {{0, {0, 1, 0}}, {1, {0, 1, 0}}, {2, {0, 1, 0}}, {3, {0, 1, 0}}}, {1, .1}}

        // Ought to fail.
    };

    for (const auto& test : test_suite)
    {
        string test_name {get<0>(test)};
        CylindricalTool tool {get<3>(test).first, get<3>(test).second};
        bool success {true};
                
        try
        {
            ToolPath tool_path(tool, get<1>(test), get<2>(test), true);
            cout << "Finished B-Rep construction for test " << test_name << endl;

            mesh_surface(tool_path.tool_path, .5, .00001); 
            cout << "Finished meshing surface for test " << test_name << endl;
            
            shape_to_stl(test_name, tool_path.tool_path, "/Users/andrewlarson/Downloads/" + test_name + ".stl");
        }
        catch (...)
        {
            success = false;
        }

        if (success)
            cout << "SUCCESS: The test " << test_name << " succeeded!" << endl;
        else
            cout << "FAILURE: The test " << test_name << " failed!" << endl;
    }
}

/* 
   ============================================================================
                                      Main
   ============================================================================ 
*/ 

int main()
{
    run_tests("/Users/andrewlarson/Downloads/");    
}
