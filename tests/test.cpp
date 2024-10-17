// Standard library.
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <cmath>
#include <tuple>

// Third party.
#include "geometric_primitives.hxx"
#include "toolpath.hxx"

using namespace std;

/* 
   ****************************************************************************
                                    Tests 
   ****************************************************************************
*/ 

struct CylCurveToolpathTest 
{
    string name;
    Curve& path;
    CylindricalTool tool;
    std::pair<double, double> meshing_parameters;
    bool visualize;
    filesystem::path results_directory;
};

struct CylLineToolpathTest 
{
    string name;
    Line& path;
    CylindricalTool tool;
    std::pair<double, double> meshing_parameters;
    bool visualize;
    filesystem::path results_directory;
};

struct CylCompoundToolpathTest 
{
    string name;
    pair<Line&, Curve&> path;
    CylindricalTool tool;
    std::pair<double, double> meshing_parameters;
    bool visualize;
    filesystem::path results_directory;
};

vector<pair<string, InterpolatedCurve>> interpolated_curve_specs { 
                                                                   // {
                                                                   //   "[interpolation]: planar corner",
                                                                   //   {
                                                                   //     {{1, 0, 0}, {1, 1, 0}, {0, 1, 0}}, 
                                                                   //     {{0, {0, 1, 0}}, {1, {-1, 0, 0}}, {2, {-1, 0, 0}}}
                                                                   //   },
                                                                   // },
                                                                   // {
                                                                   //   "[interpolation]: planar straight line",
                                                                   //   {
                                                                   //     {{0, 0, 0}, {1, 0, 0}, {2, 0, 0}}, 
                                                                   //     {{0, {1, 0, 0}}, {1, {1, 0, 0}}, {2, {1, 0, 0}}}
                                                                   //   },
                                                                   // },
                                                                   // TODO: Causes segfault due to bug in OCCT. In general, it appears
                                                                   //     that sweeping along non-axial stright lines causes segfaults.
                                                                   // {
                                                                   //   "[interpolation]: planar non-axial straight line",
                                                                   //   {
                                                                   //     {{0, 0, 0}, {1, 1, 0}, {2, 2, 0}}, 
                                                                   //     {{0, {1, 1, 0}}, {1, {1, 1, 0}}, {2, {1, 1, 0}}}
                                                                   //   },
                                                                   // },
                                                                   // TODO: Causes segfault due to bug in OCCT. In general, it appears
                                                                   //     that sweeping along non-axial stright lines causes segfaults.
                                                                   // {
                                                                   //   "[interpolation]: planar non-axial straight line 2",
                                                                   //   {
                                                                   //     {{1, 0, 0}, {0, 1, 0}, {-1, 2, 0}}, 
                                                                   //     {{0, {-1, 1, 0}}, {1, {-1, 1, 0}}, {2, {-1, 1, 0}}}
                                                                   //   },
                                                                   // },
                                                                   // {
                                                                   //   "[interpolation]: planar zigzag",
                                                                   //   {
                                                                   //     {{0, 0, 0}, {1, 1, 0}, {0, 2, 0}, {3, 3, 0}, {0, 4, 0}}, 
                                                                   //     {{0, {0, 1, 0}}, {1, {0, 1, 0}}, {2, {0, 1, 0}}, {3, {0, 1, 0}}}
                                                                   //   }, 
                                                                   // },
                                                                   // {
                                                                   //   "[interpolation]: planar horseshoe",
                                                                   //   {
                                                                   //     {{-1, 1, 0}, {-1, .4, 0}, {0, 0, 0}, {1, .4, 0}, {1, 1, 0}}, 
                                                                   //     {{0, {0, -1, 0}}, {1, {0, -1, 0}}, {2, {1, 0, 0}}, {3, {0, 1, 0}}, {4, {0, 1, 0}}}
                                                                   //   }, 
                                                                   // },
                                                                   // {
                                                                   //   "[interpolation]: planar one tangent",
                                                                   //   {
                                                                   //     {{0, 0, 0}, {1, 5, 0}, {0, 10, 0}, {1, 15, 0}, {0, 20, 0}}, 
                                                                   //     {{0, {1, 5, 0}}}
                                                                   //   },
                                                                   // },
                                                                   // {
                                                                   //   "[interpolation]: non-planar corner",
                                                                   //   { 
                                                                   //     {{1, 0, 0}, {1, 1, .5}, {0, 1, 5}}, 
                                                                   //     {{0, {0, 1, 0}}, {1, {-1, 0, 0}}, {2, {-1, 0, 0}}}
                                                                   //   },
                                                                   // },
                                                                   // {
                                                                   //   "[interpolation]: non-planar zigzag",
                                                                   //   {
                                                                   //     {{0, 0, 0}, {1, 1, .5}, {0, 2, 1}, {3, 3, 2}, {0, 4, 3}}, 
                                                                   //     {{0, {0, 1, 0}}, {1, {0, 1, 0}}, {2, {0, 1, 0}}, {3, {0, 1, 0}}}
                                                                   //   }, 
                                                                   // },
                                                                   // {
                                                                   //   "[interpolation]: non-planar horseshoe",
                                                                   //   {
                                                                   //     {{-1, 1, 0}, {-1, .4, .5}, {0, 0, 1}, {1, .4, .5}, {1, 1, 0}}, 
                                                                   //     {{0, {0, -1, 0}}, {1, {0, -1, 0}}, {2, {1, 0, 0}}, {3, {0, 1, 0}}, {4, {0, 1, 0}}}
                                                                   //   }, 
                                                                   // }
                                                                };

vector<pair<string, ArcOfCircle>> arc_of_circle_curve_specs {
                                                              // {
                                                              //   "[arc of circle]: origin centered 1", 
                                                              //   {{{1, 0}, {0, 1}}, {0, 0}, 1}
                                                              // },  
                                                              // {
                                                              //   "[arc of circle]: origin centered 2", 
                                                              //   {{{0, 1}, {1/pow(2, .5), 1/pow(2, .5)}}, {0, 0}, 1},
                                                              // },  
                                                              // {
                                                              //   "[arc of circle]: origin centered 3", 
                                                              //   {{{-1/pow(2, .5), 1/pow(2, .5)}, {1/pow(2, .5), 1/pow(2, .5)}}, {0, 0}, 1},
                                                              // },  
                                                              // {
                                                              //   "[arc of circle]: origin centered 4", 
                                                              //   {{{.75, .6614}, {-1, 0}}, {0, 0}, 1},
                                                              // },

                                                              // // Not origin centered. 
                                                              // {
                                                              //   "[arc of circle]: origin centered 5", 
                                                              //   {{{1 + 10, 0 + 10}, {0 + 10, 1 + 10}}, {10, 10}, 1},
                                                              // },
                                                              // {
                                                              //   "[arc of circle]: origin centered 6", 
                                                              //   {{{0 + 10, 1 + 5}, {1/pow(2, .5) + 10, 1/pow(2, .5) + 5}}, {10, 5}, 1},
                                                              // },
                                                              // {
                                                              //   "[arc of circle]: origin centered 7", 
                                                              //   {{{-1/pow(2, .5) + 10, 1/pow(2, .5) + 5}, {1/pow(2, .5) + 10, 1/pow(2, .5) + 5}}, {10, 5}, 1},
                                                              // },
                                                              // {
                                                              //   "[arc of circle]: origin centered 8", 
                                                              //   {{{.75 - 100, .6614 + 3}, {-1 - 100, 0 + 3}}, {-100, 3}, 1},
                                                              // }
                                                           };

vector<pair<string, Line>> linear_specs {
                                          // {
                                          //   "[linear]: simple1",
                                          //   {
                                          //     {0, 0, 0},
                                          //     {1, 1, 0}
                                          //   }
                                          // },
                                          // {
                                          //   "[linear]: simple2",
                                          //   {
                                          //     {1, 1, 1},
                                          //     {-1, 0, 0}
                                          //   }
                                          // },
                                          // {
                                          //   "[linear]: simple3",
                                          //   {
                                          //     {1, 1, 0},
                                          //     {-1, -1, 0}
                                          //   }
                                          // },
                                          // {
                                          //   "[linear]: simple4",
                                          //   {
                                          //     {-2, -2, -2},
                                          //     {5, 5, 5}
                                          //   }
                                          // },
                                        };

vector<pair<string, pair<Line, ArcOfCircle>>> compound_specs1 {
                                                                // {
                                                                //   "[linear + arc of circle]: simple_touching1",
                                                                //   {
                                                                //     {
                                                                //       {0, 0, 0},
                                                                //       {1, 1, 0}
                                                                //     },
                                                                //     {{{1, 0}, {0, 1}}, {0, 0}, 1}                                                                   
                                                                //   }
                                                                // },
                                                                // {
                                                                //   "[linear + arc of circle]: simple_touching2",
                                                                //   {
                                                                //     {
                                                                //       {0, 0, 0},
                                                                //       {0, 1, 0}
                                                                //     },
                                                                //     {{{0, 1}, {-1, 0}}, {0, 0}, 1}                                                                   
                                                                //   }
                                                                // },
                                                                // {
                                                                //   "[linear + arc of circle]: simple_touching3",
                                                                //   {
                                                                //     {
                                                                //       {1, 1, 0},
                                                                //       {2, 1, 0}
                                                                //     },
                                                                //     {{{1.5, 0}, {0, 1.5}}, {0, 0}, 1.5}                                                                   
                                                                //   }
                                                                // },
                                                                // {
                                                                //   "[linear + arc of circle]: simply_touching4",
                                                                //   {
                                                                //     {
                                                                //       {1, 1, 0},
                                                                //       {2, 1, 0}
                                                                //     },
                                                                //     {{{1.5, 0}, {0, 1.5}}, {0, 0}, 1.5}                                                                   
                                                                //   }
                                                                // },
                                                                {
                                                                  "[linear + arc of circle]: not_touching1",
                                                                  {
                                                                    {
                                                                      {-1, 0, 0},
                                                                      {1, 0, 0}
                                                                    },
                                                                    {{{6, 5}, {5, 6}}, {5, 5}, 1}                                                                   
                                                                  }
                                                                },
                                                              };

/* **************************************************************************** */

/* 
   ****************************************************************************
                            File Local Declarations
   ****************************************************************************
*/ 

using Tests = tuple<vector<CylCurveToolpathTest>, vector<CylLineToolpathTest>, vector<CylCompoundToolpathTest>>;
static Tests gen_tests();
template <class T> static void run_tests(vector<T>& tests);

/* **************************************************************************** */

/* 
   ****************************************************************************
                             File Local Definitions
   ****************************************************************************
*/ 

static Tests gen_tests()
{
    Tests tests;
    
    const CylindricalTool default_cylindrical_tool {.1, .3};
    const pair<double, double> default_mesh_options {.5, .00001};
    const filesystem::path default_results_directory {"/tmp/"};
    const bool visualize {true};

    for (auto& interpolated_curve_spec : interpolated_curve_specs)
        get<0>(tests).push_back(
                                 {
                                   interpolated_curve_spec.first,
                                   interpolated_curve_spec.second,
                                   default_cylindrical_tool,
                                   default_mesh_options,
                                   visualize,
                                   default_results_directory
                                 }
                               ); 

    for (auto& arc_of_circle_curve_spec : arc_of_circle_curve_specs)
        get<0>(tests).push_back(
                                 {
                                   arc_of_circle_curve_spec.first,
                                   arc_of_circle_curve_spec.second,
                                   default_cylindrical_tool,
                                   default_mesh_options,
                                   visualize,
                                   default_results_directory
                                 }
                               ); 

    for (auto& linear_spec : linear_specs) 
        get<1>(tests).push_back(
                                 {
                                   linear_spec.first,
                                   linear_spec.second,
                                   default_cylindrical_tool,
                                   default_mesh_options,
                                   visualize,
                                   default_results_directory
                                 }
                               ); 

    for (auto& compound_spec : compound_specs1) 
        get<2>(tests).push_back(
                                 {
                                   compound_spec.first,
                                   compound_spec.second,
                                   default_cylindrical_tool,
                                   default_mesh_options,
                                   visualize,
                                   default_results_directory
                                 }
                               ); 

    return tests;
}

template <class T>
static void run_tests(vector<T>& tests)
{ 
    for (const auto& test : tests)
    {
        bool success {true};

        cout << endl;
        cout << "********* TEST: " << test.name << " **********" << endl;

        cout << "Starting to build toolpath for test " << test.name << endl;
        ToolPath tool_path {test.path, test.tool, test.visualize};
        cout << "Finished B-Rep construction for test " << test.name << endl;
        
        cout << "Starting to mesh surface for test " << test.name << endl;
        tool_path.mesh_surface(test.meshing_parameters.first, test.meshing_parameters.second);
        cout << "Finished meshing surface for test " << test.name << endl;
        
        string stl_path = test.results_directory.string() + test.name + ".stl";
        tool_path.shape_to_stl(test.name, stl_path);
        cout << "Surface mesh written to: " << stl_path << endl;

        if (success)
            cout << "SUCCESS: The test " << test.name << " succeeded!" << endl;
        else
            cout << "FAILURE: The test " << test.name << " failed!" << endl;
        cout << "********* FINISH TEST: " << test.name << " **********" << endl;
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
    vector<CylCurveToolpathTest> cyl_curve_tests; 
    vector<CylLineToolpathTest> cyl_line_tests;
    vector<CylCompoundToolpathTest> cyl_compound_tests;
    tie(cyl_curve_tests, cyl_line_tests, cyl_compound_tests) = gen_tests();
    run_tests(cyl_curve_tests);
    run_tests(cyl_line_tests);
    run_tests(cyl_compound_tests);
}
