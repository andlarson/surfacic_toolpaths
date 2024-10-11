// Standard library.
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>

// Third party.
#include "geometric_primitives.hxx"
#include "toolpath.hxx"

using namespace std;

/* 
   ****************************************************************************
                                    Tests 
   ****************************************************************************
*/ 

struct CylToolpathTest 
{
    string name;
    Curve& curve;
    CylindricalTool tool;
    std::pair<double, double> meshing_parameters;
    bool visualize;
    filesystem::path results_directory;
};

string default_results_directory {"/tmp/"};

vector<InterpolatedCurve> interpolated_curve_specs { 
                                                     // Planar.
                                                     {{{1, 0, 0}, {1, 1, 0}, {0, 1, 0}}, {{0, {0, 1, 0}}, {1, {-1, 0, 0}}, {2, {-1, 0, 0}}}},
                                                     {{{0, 0, 0}, {1, 0, 0}, {2, 0, 0}}, {{0, {1, 0, 0}}, {1, {1, 0, 0}}, {2, {1, 0, 0}}}},
                                                     {{{0, 0, 0}, {1, 1, 0}, {0, 2, 0}, {3, 3, 0}, {0, 4, 0}}, {{0, {0, 1, 0}}, {1, {0, 1, 0}}, {2, {0, 1, 0}}, {3, {0, 1, 0}}}}, 
                                                     {{{-1, 1, 0}, {-1, .4, 0}, {0, 0, 0}, {1, .4, 0}, {1, 1, 0}}, {{0, {0, -1, 0}}, {1, {0, -1, 0}}, {2, {1, 0, 0}}, {3, {0, 1, 0}}, {4, {0, 1, 0}}}}, 
                                                     {{{0, 0, 0}, {1, 5, 0}, {0, 10, 0}, {1, 15, 0}, {0, 20, 0}}, {{0, {1, 5, 0}}}},

                                                     // Non-planar. 
                                                     {{{1, 0, 0}, {1, 1, .5}, {0, 1, 5}}, {{0, {0, 1, 0}}, {1, {-1, 0, 0}}, {2, {-1, 0, 0}}}},
                                                     {{{0, 0, 0}, {1, 1, .5}, {0, 2, 1}, {3, 3, 2}, {0, 4, 3}}, {{0, {0, 1, 0}}, {1, {0, 1, 0}}, {2, {0, 1, 0}}, {3, {0, 1, 0}}}}, 
                                                     {{{-1, 1, 0}, {-1, .4, .5}, {0, 0, 1}, {1, .4, .5}, {1, 1, 0}}, {{0, {0, -1, 0}}, {1, {0, -1, 0}}, {2, {1, 0, 0}}, {3, {0, 1, 0}}, {4, {0, 1, 0}}}}, 
                                                   };

vector<ArcOfCircle> arc_of_circle_curve_specs {
                                                {{{1, 0}, {0, 1}}, {0, 0}, 1},
                                                {{{0, 1}, {0, -1}}, {0, 0}, 1}
                                              };

vector<CylToolpathTest> toolpath_tests =
{
    // {
    //     "[interpolation + cylindrical tool]: planar corner", 
    //     interpolated_curve_specs[0],
    //     CylindricalTool {.1, 1}, 
    //     {.5, .00001},
    //     true,
    //     default_results_directory
    // },
    // {
    //     "[interpolation + cylindrical tool]: planar straight_line", 
    //     interpolated_curve_specs[1],
    //     CylindricalTool {.2, 1}, 
    //     {.5, .00001},
    //     true,
    //     default_results_directory
    // },
    // {
    //     "[interpolation + cylindrical tool]: planar zigzag", 
    //     interpolated_curve_specs[2],
    //     CylindricalTool {.05, 1}, 
    //     {.5, .00001},
    //     true,
    //     default_results_directory
    // },
    // {
    //     "[interpolation + cylindrical tool]: planar horseshoe", 
    //     interpolated_curve_specs[3],
    //     CylindricalTool {.1, 2}, 
    //     {.5, .00001},
    //     true,
    //     default_results_directory
    // },
    // {
    //     "[interpolation + cylindrical tool]: planar one_tangent", 
    //     interpolated_curve_specs[4],
    //     CylindricalTool {.5, 10}, 
    //     {.5, .00001},
    //     true,
    //     default_results_directory
    // }

    // {
    //     "[interpolation + cylindrical tool]: non-planar corner", 
    //     interpolated_curve_specs[5],
    //     CylindricalTool {.1, .3}, 
    //     {.5, .00001},
    //     true,
    //     default_results_directory
    // },
    // {
    //     "[interpolation + cylindrical tool]: non-planar zigzag", 
    //     interpolated_curve_specs[6],
    //     CylindricalTool {.05, 1}, 
    //     {.5, .00001},
    //     true,
    //     default_results_directory
    // },
    // {
    //     "[interpolation + cylindrical tool]: non-planar horseshoe", 
    //     interpolated_curve_specs[7],
    //     CylindricalTool {.1, 2}, 
    //     {.5, .00001},
    //     true,
    //     default_results_directory
    // },

    // {
    //     "[arc of circle + cylindrical tool]: quarter circle centered at origin", 
    //     arc_of_circle_curve_specs[0],
    //     CylindricalTool {.1, 2}, 
    //     {.5, .00001},
    //     true,
    //     default_results_directory
    // },
    {
        "[arc of circle + cylindrical tool]: half circle centered at origin", 
        arc_of_circle_curve_specs[1],
        CylindricalTool {.1, 2}, 
        {.5, .00001},
        true,
        default_results_directory
    },
};

/* **************************************************************************** */

/* 
   ****************************************************************************
                            File Local Declarations
   ****************************************************************************
*/ 

static void run_tests();

/* **************************************************************************** */

/* 
   ****************************************************************************
                             File Local Definitions
   ****************************************************************************
*/ 

static void run_tests()
{
    for (const auto& test : toolpath_tests)
    {
        bool success {true};

        try
        {
            cout << endl;
            cout << "Starting to build toolpath for test " << test.name << endl;
            ToolPath tool_path {test.curve, test.tool, test.visualize};
            cout << "Finished B-Rep construction for test " << test.name << endl;
            
            cout << "Starting to mesh surface for test " << test.name << endl;
            tool_path.mesh_surface(test.meshing_parameters.first, test.meshing_parameters.second);
            cout << "Finished meshing surface for test " << test.name << endl;
            
            string stl_path = test.results_directory.string() + test.name + ".stl";
            tool_path.shape_to_stl(test.name, stl_path);
            cout << "Surface mesh written to: " << stl_path << endl;
        }
        catch (...)
        {
            success = false;
        }

        if (success)
            cout << "SUCCESS: The test " << test.name << " succeeded!" << endl;
        else
            cout << "FAILURE: The test " << test.name << " failed!" << endl;
        cout << endl;
    }
}

/* **************************************************************************** */

/* 
   ****************************************************************************
                                        Main 
   ****************************************************************************
*/ 

int main()
{
    run_tests();    
}
