// Standard library.
#include <fstream>
#include <vector>
#include <string>

// Third party.
#include "geometric_primitives.hxx"
#include "tool_curve.hxx"
#include "tool_profile.hxx"
#include "toolpath.hxx"

using namespace std;

/* 
   ****************************************************************************
                                Global Variables 
   ****************************************************************************
*/ 

// Testing curve defined by interpolation and a cylindrical tool.
// The format of each test is (test name, interpolation points, tangents, tool size, meshing parameters, visualization).
using interp_curve_cyl_tool_test = tuple<string, 
                                         vector<Point3D>, 
                                         vector<pair<uint64_t, Vec3D>>, 
                                         pair<double, double>,
                                         pair<double, double>,
                                         bool>;

const vector<interp_curve_cyl_tool_test> interp_curve_cyl_tool_tests =
{
    // On a single plane orthogonal to z axis. 
    // {
    //     "[interpolation + cylindrical tool]: corner", 
    //     {{1, 0, 0}, {1, 1, 0}, {0, 1, 0}}, 
    //     {{0, {0, 1, 0}}, {1, {-1, 0, 0}}, {2, {-1, 0, 0}}}, 
    //     {1, .1}, 
    //     {.5, .00001},
    //     true
    // },
    {
        "[interpolation + cylindrical tool]: straight_line", 
        {{0, 0, 0}, {1, 0, 0}, {2, 0, 0}}, 
        {{0, {1, 0, 0}}, {1, {1, 0, 0}}, {2, {1, 0, 0}}}, 
        {1, .2}, 
        {.5, .00001},
        true
    },
    {
        "[interpolation + cylindrical tool]: zigzag", 
        {{0, 0, 0}, {1, 1, 0}, {0, 2, 0}, {3, 3, 0}, {0, 4, 0}}, 
        {{0, {0, 1, 0}}, {1, {0, 1, 0}}, {2, {0, 1, 0}}, {3, {0, 1, 0}}}, 
        {1, .05}, 
        {.5, .00001},
        true
    },
    // {
    //     "[interpolation + cylindrical tool]: horseshoe", 
    //     {{-1, 1, 0}, {-1, .4, 0}, {0, 0, 0}, {1, .4, 0}, {1, 1, 0}}, 
    //     {{0, {0, -1, 0}}, {1, {0, -1, 0}}, {2, {1, 0, 0}}, {3, {0, 1, 0}}, {4, {0, 1, 0}}}, 
    //     {2, .1}, 
    //     {.5, .00001},
    //     true
    // },
    // {
    //     "[interpolation + cylindrical tool]: one_tangent", 
    //     {{0, 0, 0}, {1, 5, 0}, {0, 10, 0}, {1, 15, 0}, {0, 20, 0}}, 
    //     {{0, {1, 5, 0}}}, 
    //     {10, .5}, 
    //     {.5, .00001},
    //     true
    // }
};

/* 
   ****************************************************************************
                            File Local Declarations
   ****************************************************************************
*/ 

static void run_tests(const string& dir);

/* 
   ****************************************************************************
                             File Local Definitions
   ****************************************************************************
*/ 

static void run_tests(const string& dir)
{
    for (const auto& test : interp_curve_cyl_tool_tests)
    {
        bool success {true};
        string test_name {get<0>(test)};

        try
        {
            CylindricalTool tool {get<3>(test).first, get<3>(test).second};

            cout << endl;
            cout << "Starting to build tool curve for test " << test_name << endl;
            InterpolatedToolCurve curve {get<1>(test), get<2>(test)};  
            cout << "Finished building tool curve for test " << test_name << endl;

            cout << "Starting to build toolpath for test " << test_name << endl;
            ToolPath tool_path {curve, tool, get<5>(test)};
            cout << "Finished B-Rep construction for test " << test_name << endl;
            
            cout << "Starting to mesh surface for test " << test_name << endl;
            tool_path.mesh_surface(get<4>(test).first, get<4>(test).second);
            cout << "Finished meshing surface for test " << test_name << endl;
            
            string stl_path = "/Users/andrewlarson/Downloads/" + test_name + ".stl";
            tool_path.shape_to_stl(test_name, stl_path);
            cout << "Surface mesh written to: " << stl_path << endl;
        }
        catch (...)
        {
            success = false;
        }

        if (success)
            cout << "SUCCESS: The test " << test_name << " succeeded!" << endl;
        else
            cout << "FAILURE: The test " << test_name << " failed!" << endl;
        cout << endl;
    }
}

/* 
   ****************************************************************************
                                        Main 
   ****************************************************************************
*/ 

int main()
{
    run_tests("/Users/andrewlarson/Downloads/");    
}
