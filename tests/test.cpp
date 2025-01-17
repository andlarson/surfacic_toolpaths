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

// Some test defaults. Not all tests use these defaults.
const CylindricalTool default_cylindrical_tool {.2, 1.5};
const pair<double, double> default_mesh_options {.5, .00001};
const filesystem::path default_results_directory {"/tmp/"};
const bool default_visualize {true};

struct CylCompoundToolpathTest 
{
    const string name;
    const tuple<vector<Line>, 
                vector<ArcOfCircle>, 
                vector<InterpolatedCurve>,
                vector<Circle>> path;
    const CylindricalTool tool;
    const std::pair<double, double> meshing_parameters;
    const bool visualize;
    const filesystem::path results_directory;
};

const vector<CylCompoundToolpathTest> tests 
{
  // // Test Class: Single interpolated curve.
  // {
  //   "[single interpolated curve]: planar corner",
  //   {
  //     // Lines.
  //     {},
  //     // Arcs of circles.
  //     {},
  //     // Interpolated curves.
  //     {
  //       {
  //         {{1, 0, 0}, {1, 1, 0}, {0, 1, 0}}, 
  //         {{0, {0, 1, 0}}, {1, {-1, 0, 0}}, {2, {-1, 0, 0}}}
  //       }
  //     },
  //     // Circles
  //     {}
  //   },
  //   // Very small tool radius necessary to avoid problems due to self
  //   //     intersection around sharp corner.
  //   {.05, 1.5}, 
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[single interpolated curve]: planar straight line",
  //   {
  //     // Lines.
  //     {},
  //     // Arcs of circles.
  //     {},
  //     // Interpolated curves.
  //     {
  //       {
  //         {{0, 0, 0}, {1, 0, 0}, {2, 0, 0}}, 
  //         {{0, {1, 0, 0}}, {1, {1, 0, 0}}, {2, {1, 0, 0}}}
  //       }
  //     },
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // // Causes segfault. In general, it appears that sweeping along non-axial 
  // //     straight lines causes segfaults. This si surprising because some
  // //     tests that sweep along non-axial straight lines, without using
  // //     interpolation, do work.
  // // {
  // //   "[single interpolated curve]: planar non-axial straight line",
  // //   {
  // //     // Lines.
  // //     {},
  // //     // Arcs of circles.
  // //     {},
  // //     // Interpolated curves.
  // //     {
  // //       {
  // //         {{0, 0, 0}, {1, 1, 0}, {2, 2, 0}}, 
  // //         {{0, {1, 1, 0}}, {1, {1, 1, 0}}, {2, {1, 1, 0}}}
  // //       }
  // //     },
  // //     // Circles.
  // //     {}
  // //   },
  // //   default_cylindrical_tool,
  // //   default_mesh_options,
  // //   default_visualize,
  // //   default_results_directory
  // // },
  // // Causes segfault. In general, it appears that sweeping along non-axial 
  // //     straight lines causes segfaults. This is surprising because some
  // //     tests that sweep along non-axial straight lines, without using
  // //     interpolation, do work.
  // // {
  // //   "[single interpolated curve]: planar non-axial straight line 2",
  // //   {
  // //     // Lines.
  // //     {},
  // //     // Arcs of circles.
  // //     {},
  // //     // Interpolated curves.
  // //     {
  // //       {
  // //         {{1, 0, 0}, {0, 1, 0}, {-1, 2, 0}}, 
  // //         {{0, {-1, 1, 0}}, {1, {-1, 1, 0}}, {2, {-1, 1, 0}}}
  // //       }
  // //     },
  // //     // Circles.
  // //     {}
  // //   },
  // //   default_cylindrical_tool,
  // //   default_mesh_options,
  // //   default_visualize,
  // //   default_results_directory
  // // },
  // {
  //   "[single interpolated curve]: planar zigzag",
  //   {
  //     // Lines.
  //     {},
  //     // Arcs of circles.
  //     {},
  //     // Interpolated curves.
  //     {
  //       {
  //         {{0, 0, 0}, {1, 1, 0}, {0, 2, 0}, {3, 3, 0}, {0, 4, 0}}, 
  //         {{0, {0, 1, 0}}, {1, {0, 1, 0}}, {2, {0, 1, 0}}, {3, {0, 1, 0}}}
  //       }, 
  //     },
  //     // Circles
  //     {}
  //   },
  //   // Very small tool radius necessary to avoid problems due to self
  //   //     intersection around sharp corner.
  //   {.05, 1},
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[single interpolated curve]: planar horseshoe",
  //   {
  //     // Lines.
  //     {},
  //     // Arcs of circles.
  //     {},
  //     // Interpolated curves.
  //     {
  //       {
  //         {{-1, 1, 0}, {-1, .4, 0}, {0, 0, 0}, {1, .4, 0}, {1, 1, 0}}, 
  //         {{0, {0, -1, 0}}, {1, {0, -1, 0}}, {2, {1, 0, 0}}, {3, {0, 1, 0}}, {4, {0, 1, 0}}}
  //       }
  //     },
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[single interpolated curve]: planar one tangent",
  //   {
  //     // Lines.
  //     {},
  //     // Arcs of circles.
  //     {},
  //     // Interpolated curves.
  //     {
  //       {
  //         {{0, 0, 0}, {1, 5, 0}, {0, 10, 0}, {1, 15, 0}, {0, 20, 0}}, 
  //         {{0, {1, 5, 0}}}
  //       }
  //     },
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // // This generates a crazy geometry due to the way that sweeping is done.
  // // {
  // //   "[single interpolated curve]: non-planar corner",
  // //   {
  // //     // Lines.
  // //     {},
  // //     // Arcs of circles.
  // //     {},
  // //     // Interpolated curves.
  // //     {
  // //       {
  // //         {{1, 0, 0}, {1, 1, .5}, {0, 1, 5}}, 
  // //         {{0, {0, 1, 0}}, {1, {-1, 0, 0}}, {2, {-1, 0, 0}}}
  // //       }
  // //     },
  // //     // Circles.
  // //     {}
  // //   },
  // //   default_cylindrical_tool,
  // //   default_mesh_options,
  // //   default_visualize,
  // //   default_results_directory
  // // },
  // // This results in a crazy geometry due to the way that sweeping works. 
  // // {
  // //   "[single interpolated curve]: non-planar zigzag",
  // //   {
  // //     // Lines.
  // //     {},
  // //     // Arcs of circles.
  // //     {},
  // //     // Interpolated curves.
  // //     {
  // //       {
  // //         {{0, 0, 0}, {1, 1, .5}, {0, 2, 1}, {3, 3, 2}, {0, 4, 3}}, 
  // //         {{0, {0, 1, 0}}, {1, {0, 1, 0}}, {2, {0, 1, 0}}, {3, {0, 1, 0}}}
  // //       }
  // //     },
  // //     // Circles.
  // //     {}
  // //   },
  // //   // Very small tool radius necessary to avoid problems due to self
  // //   //     intersection around sharp corner.
  // //   {.05, 1},
  // //   default_mesh_options,
  // //   default_visualize,
  // //   default_results_directory
  // // },
  // // This results in crazy geometry due to sharp corners.
  // // {
  // //   "[single interpolated curve]: non-planar horseshoe",
  // //   {
  // //     // Lines.
  // //     {},
  // //     // Arcs of circles.
  // //     {},
  // //     // Interpolated curves.
  // //     {
  // //       {
  // //         {{-1, 1, 0}, {-1, .4, .5}, {0, 0, 1}, {1, .4, .5}, {1, 1, 0}}, 
  // //         {{0, {0, -1, 0}}, {1, {0, -1, 0}}, {2, {1, 0, 0}}, {3, {0, 1, 0}}, {4, {0, 1, 0}}}
  // //       }
  // //     },
  // //     // Circles.
  // //     {}
  // //   },
  // //   // Very small tool radius necessary to avoid problems due to self
  // //   //     intersection around sharp corner.
  // //   {.05, 1},
  // //   default_mesh_options,
  // //   default_visualize,
  // //   default_results_directory
  // // },

  // // Test Class: Single Line.
  // {
  //   "[single line]: simple1",
  //   {
  //     // Lines.
  //     {
  //       {
  //         {0, 0, 0},
  //         {1, 1, 0}
  //       }
  //     },
  //     // Arcs of circles.
  //     {},
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[single line]: simple2",
  //   {
  //     // Lines.
  //     {
  //       {
  //         {1, 1, 1},
  //         {-1, 0, 0}
  //       }
  //     },
  //     // Arcs of circles.
  //     {},
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[single line]: simple3",
  //   {
  //     // Lines.
  //     {
  //       {
  //         {1, 1, 0},
  //         {-1, -1, 0}
  //       }
  //     },
  //     // Arcs of circles.
  //     {},
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // // This results in crazy geometry due to the way that sweeping works.
  // // {
  // //   "[single line]: simple4",
  // //   {
  // //     // Lines.
  // //     {
  // //       {
  // //         {-2, -2, -2},
  // //         {5, 5, 5}
  // //       }
  // //     },
  // //     // Arcs of circles.
  // //     {},
  // //     // Interpolated curves.
  // //     {},
  // //     // Circles.
  // //     {}
  // //   },
  // //   default_cylindrical_tool,
  // //   default_mesh_options,
  // //   default_visualize,
  // //   default_results_directory
  // // },

  // // Test Class: Single Arc of Circle.
  // {
  //   "[single arc of circle]: origin centered 1",
  //   {
  //     // Lines.
  //     {},
  //     // Arcs of circles.
  //     {
  //       {
  //         {{1, 0, 0}, {0, 1, 0}}, 
  //         {.5, sqrt(1 - pow(.5, 2)), 0}, 
  //       }  
  //     },
  //     // Interpolated curves.
  //     {},
  //     // Circles.
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[single arc of circle]: origin centered 2",
  //   {
  //     // Lines.
  //     {},
  //     // Arcs of circles.
  //     {
  //       { 
  //         {{-1/pow(2, .5), 1/pow(2, .5), 0}, {1/pow(2, .5), 1/pow(2, .5), 0}}, 
  //         {0, 1, 0}
  //       }
  //     },
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[single arc of circle]: origin centered 3",
  //   {
  //     // Lines.
  //     {},
  //     // Arcs of circles.
  //     {
  //       {
  //         {{.75, .6614, 0}, {-1, 0, 0}}, 
  //         {0, 1, 0}
  //       }
  //     },
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[single arc of circle]: origin centered 4, almost full circle",
  //   {
  //     // Lines.
  //     {},
  //     // Arcs of circles.
  //     {
  //       {
  //         {{1, 0, 0}, {.995, -sqrt(1 - pow(.995, 2)), 0}}, 
  //         {0, 1, 0}
  //       }
  //     },
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[single arc of circle]: not origin centered 1",
  //   {
  //     // Lines.
  //     {},
  //     // Arcs of circles.
  //     {
  //       {
  //         {{1 + 10, 0 + 10, 0}, {0 + 10, 1 + 10, 0}}, 
  //         {10.5, sqrt(1 - pow(.5, 2)) + 10, 0}, 
  //       }
  //     },
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[single arc of circle]: not origin centered 2",
  //   {
  //     // Lines.
  //     {},
  //     // Arcs of circles.
  //     {
  //       {
  //         {{0 + 10, 1 + 5, 0}, {1/pow(2, .5) + 10, 1/pow(2, .5) + 5, 0}}, 
  //         {-1 + 10, 5, 0}, 
  //       }
  //     },
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[single arc of circle]: not origin centered 3",
  //   {
  //     // Lines.
  //     {},
  //     // Arcs of circles.
  //     {
  //       {
  //         {{-1/pow(2, .5) + 10, 1/pow(2, .5) + 5, 0}, {1/pow(2, .5) + 10, 1/pow(2, .5) + 5, 0}}, 
  //         {10, 5 - 1, 0}, 
  //       }
  //     },
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // 
  // // Test Class: Single Circle. 
  // {
  //   "[single circle]: origin centered 1",
  //   {
  //     // Lines.
  //     {},
  //     // Arcs of circles.
  //     {},
  //     // Interpolated curves.
  //     {},
  //     // Circles.
  //     {
  //       {
  //         {1, 0, 0},
  //         {0, 1, 0},
  //         {-1, 0, 0}
  //       }
  //     }
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[single circle]: origin centered 2",
  //   {
  //     // Lines.
  //     {},
  //     // Arcs of circles.
  //     {},
  //     // Interpolated curves.
  //     {},
  //     // Circles.
  //     {
  //       {
  //         {1/sqrt(2), 1/sqrt(2), 0},
  //         {-1/sqrt(2), -1/sqrt(2), 0},
  //         {-1/sqrt(2), 1/sqrt(2), 0}
  //       }
  //     }
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // // This results in crazy geometry because the tool is oriented in the +Z
  // //     direction.
  // // {
  // //   "[single circle]: non origin centered 3",
  // //   {
  // //     // Lines.
  // //     {},
  // //     // Arcs of circles.
  // //     {},
  // //     // Interpolated curves.
  // //     {},
  // //     // Circles.
  // //     {
  // //       {
  // //         {0, 0, 0},
  // //         {1, 0, 0},
  // //         {0, 0, 1}
  // //       }
  // //     }
  // //   },
  // //   default_cylindrical_tool,
  // //   default_mesh_options,
  // //   default_visualize,
  // //   default_results_directory
  // // },
  // {
  //   "[single circle]: non origin centered 1",
  //   {
  //     // Lines.
  //     {},
  //     // Arcs of circles.
  //     {},
  //     // Interpolated curves.
  //     {},
  //     // Circles.
  //     {
  //       {
  //         {1 + 2, 1, 0},
  //         {1, 1 + 2, 0},
  //         {1 - 2, 1, 0}
  //       }
  //     }
  //   },
  //   // For some reason I don't understand, using a larger tool radius results
  //   //     in self intersection and crazy geometry.
  //   {.1, 1},
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },

  // // Test Class: Single Line + Single Arc of Circle.
  // {
  //   "[single line + single arc of circle]: simple touching1",
  //   {
  //     // Lines.
  //     {
  //       {
  //         {0, 0, 0},
  //         {1, 1, 0}
  //       },
  //     },
  //     // Arcs of circles.
  //     {
  //       {
  //         {{1, 0, 0}, {0, 1, 0}},
  //         {.5, sqrt(1 - pow(.5, 2)), 0},
  //       }
  //     },
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[single line + single arc of circle]: simple touching2",
  //   {
  //     // Lines.
  //     {
  //       {
  //         {0, 0, 0},
  //         {0, 1, 0}
  //       }
  //     },
  //     // Arcs of circles.
  //     {
  //       {
  //         {{0, 1, 0}, {-1, 0, 0}},
  //         {1, 0, 0},
  //       }
  //     },
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[single line + single arc of circle]: simple touching3",
  //   {
  //     // Lines.
  //     {
  //       {
  //         {1, 1, 0},
  //         {2, 1, 0}
  //       }
  //     },
  //     // Arcs of circles.
  //     {
  //       {
  //         {{1.5, 0, 0}, {0, 1.5, 0}},
  //         {.5, sqrt(pow(1.5, 2) - pow(.5, 2)), 0},
  //       }
  //     },
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[single line + single arc of circle]: simply touching4",
  //   {
  //     // Lines.
  //     {
  //       {
  //         {1, 1, 0},
  //         {2, 1, 0}
  //       }
  //     },
  //     // Arcs of circles.
  //     {
  //       {
  //         {{1.5, 0, 0}, {0, 1.5, 0}}, 
  //         {.5, sqrt(pow(1.5, 2) - pow(.5, 2)), 0}, 
  //       }
  //     },
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[single line + single arc of circle]: not touching1",
  //   {
  //     // Lines.
  //     {
  //       {
  //         {-1, 0, 0},
  //         {1, 0, 0}
  //       }
  //     },
  //     // Arcs of circles.
  //     {
  //       {
  //         {{6, 5, 0}, {5, 6, 0}},
  //         {5, 4, 0},
  //       }
  //     },
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[single line + single arc of circle]: realistic touching",
  //   {
  //     // Lines.
  //     {
  //       {
  //         {6.044, -.888, -.3},
  //         {-.014, .553, 0}
  //       }
  //     },
  //     // Arcs of circles.
  //     {
  //       {
  //         {{6.030, -.335, -.3}, {3.669, -.381, -.3}}, 
  //         {4, -sqrt(pow(249.72, 2) - pow((4 + .014885), 2)) + 249.31, -.3}, 
  //       }
  //     },
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },

  // // Test Class: Multiple lines. 
  // {
  //   "[multiple lines]: two touching lines",
  //   {
  //     // Lines.
  //     {
  //       {
  //         {0, 0, 0},
  //         {1, 0, 0}
  //       },
  //       {
  //         {1, 0, 0},
  //         {0, 1, 0}
  //       }
  //     },
  //     // Arcs of circles.
  //     {},
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[multiple lines]: two parallel lines",
  //   {
  //     // Lines.
  //     {
  //       {
  //         {-1, 0, 0},
  //         {0, -1, 0}
  //       },
  //       {
  //         {-1, -1, 0},
  //         {0, -1, 0}
  //       }
  //     },
  //     // Arcs of circles.
  //     {},
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[multiple lines]: fours a square",
  //   {
  //     // Lines.
  //     {
  //       {
  //         {0, 0, 0},
  //         {1, 0, 0}
  //       },
  //       {
  //         {1, 0, 0},
  //         {0, 1, 0}
  //       },
  //       {
  //         {1, 1, 0},
  //         {-1, 0, 0}
  //       },
  //       {
  //         {0, 1, 0},
  //         {0, -1, 0}
  //       }
  //     },
  //     // Arcs of circles.
  //     {},
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // // Because the axis of symmetry of the tool is always assumed to point in the +Z
  // //     direction, this test results in non-sensical geometry. 
  // // {
  // //   "[multiple lines]: craziness",
  // //   {
  // //     // Lines.
  // //     {
  // //       {
  // //         {0, 0, 0},
  // //         {1, 1, 1}
  // //       },
  // //       {
  // //         {0, 0, 0},
  // //         {-1, -1, -1}
  // //       },
  // //       {
  // //         {0, 0, 0},
  // //         {1, -1, -1}
  // //       },
  // //       {
  // //         {0, 0, 0},
  // //         {-1, 1, 1}
  // //       }
  // //     },
  // //     // Arcs of circles.
  // //     {},
  // //     // Interpolated curves.
  // //     {},
  // //     // Circles.
  // //     {}
  // //   },
  // //   default_cylindrical_tool,
  // //   default_mesh_options,
  // //   default_visualize,
  // //   default_results_directory
  // // },
  // {
  //   "[multiple lines]: many paths, one region",
  //   {
  //     // Lines.
  //     {
  //       {
  //         {0, 0, 0},
  //         {1, 0, 0}
  //       },
  //       {
  //         {1, 0, 0},
  //         {0, 1, 0}
  //       },
  //       {
  //         {1, 1, 0},
  //         {-1, 0, 0}
  //       },
  //       {
  //         {0, 1, 0},
  //         {0, -1, 0}
  //       }
  //     },
  //     // Arcs of circles.
  //     {},
  //     // Interpolated curves.
  //     {},
  //     // Circles
  //     {}
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },

  // // Test Class: Multiple Circles.  
  // {
  //   "[multiple circles]: not touching",
  //   {
  //     // Lines.
  //     {},
  //     // Arcs of circles.
  //     {},
  //     // Interpolated curves.
  //     {},
  //     // Circles.
  //     {
  //       {
  //         {1, 0, 0},
  //         {0, 1, 0},
  //         {-1, 0, 0}
  //       },
  //       {
  //         {5 + 1, 5, 0},
  //         {5, 5 + 1, 0},
  //         {5 - 1, 5, 0}
  //       }
  //     }
  //   },
  //   default_cylindrical_tool,
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[multiple circles]: overlapping, concentric 1",
  //   {
  //     // Lines.
  //     {},
  //     // Arcs of circles.
  //     {},
  //     // Interpolated curves.
  //     {},
  //     // Circles.
  //     {
  //       {
  //         {1, 0, 0},
  //         {0, 1, 0},
  //         {-1, 0, 0}
  //       },
  //       {
  //         {.9, 0, 0},
  //         {0, .9, 0},
  //         {-.9, 0, 0}
  //       },
  //       {
  //         {.8, 0, 0},
  //         {0, .8, 0},
  //         {-.8, 0, 0}
  //       }
  //     }
  //   },
  //   // Small tool radius necessary to avoid self intersection and crazy
  //   //     geometry...
  //   {.1, 1},
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // {
  //   "[multiple circles]: overlapping, concentric 2",
  //   {
  //     // Lines.
  //     {},
  //     // Arcs of circles.
  //     {},
  //     // Interpolated curves.
  //     {},
  //     // Circles.
  //     {
  //       {
  //         {1, 0, 0},
  //         {0, 1, 0},
  //         {-1, 0, 0}
  //       },
  //       {
  //         {.8, 0, 0},
  //         {0, .8, 0},
  //         {-.8, 0, 0}
  //       },
  //     }
  //   },
  //   {.2, 1},
  //   default_mesh_options,
  //   default_visualize,
  //   default_results_directory
  // },
  // // For some reason I don't understand, this results in an incorrect geometry!
  // //     The inner circle, when generated is wrong. And the final geometry just
  // //     does not include the inner circle.
  // // {
  // //   "[multiple circles]: overlapping, concentric 3",
  // //   {
  // //     // Lines.
  // //     {},
  // //     // Arcs of circles.
  // //     {},
  // //     // Interpolated curves.
  // //     {},
  // //     // Circles.
  // //     {
  // //       {
  // //         {1, 0, 0},
  // //         {0, 1, 0},
  // //         {-1, 0, 0}
  // //       },
  // //       {
  // //         {.8, 0, 0},
  // //         {0, .8, 0},
  // //         {-.8, 0, 0}
  // //       },
  // //     }
  // //   },
  // //   {.4, 1},
  // //   default_mesh_options,
  // //   default_visualize,
  // //   default_results_directory
  // // },
  // // For some reason I don't understand, this results in an incorrect geometry!
  // //     The inner circle, when generated is wrong. And the final geometry just
  // //     does not include the inner circle.
  // // {
  // //   "[multiple circles]: overlapping, concentric, full circle",
  // //   {
  // //     // Lines.
  // //     {},
  // //     // Arcs of circles.
  // //     {},
  // //     // Interpolated curves.
  // //     {},
  // //     // Circles.
  // //     {
  // //       {
  // //         {1, 0, 0},
  // //         {0, 1, 0},
  // //         {-1, 0, 0}
  // //       },
  // //       {
  // //         {.9, 0, 0},
  // //         {0, .9, 0},
  // //         {-.9, 0, 0}
  // //       },
  // //       {
  // //         {.8, 0, 0},
  // //         {0, .8, 0},
  // //         {-.8, 0, 0}
  // //       },
  // //       {
  // //         {.7, 0, 0},
  // //         {0, .7, 0},
  // //         {-.7, 0, 0}
  // //       },
  // //       {
  // //         {.6, 0, 0},
  // //         {0, .6, 0},
  // //         {-.6, 0, 0}
  // //       },
  // //       {
  // //         {.5, 0, 0},
  // //         {0, .5, 0},
  // //         {-.5, 0, 0}
  // //       },
  // //       {
  // //         {.4, 0, 0},
  // //         {0, .4, 0},
  // //         {-.4, 0, 0}
  // //       },
  // //       {
  // //         {.3, 0, 0},
  // //         {0, .3, 0},
  // //         {-.3, 0, 0}
  // //       },
  // //       {
  // //         {.2, 0, 0},
  // //         {0, .2, 0},
  // //         {-.2, 0, 0}
  // //       },
  // //       {
  // //         {.1, 0, 0},
  // //         {0, .1, 0},
  // //         {-.1, 0, 0}
  // //       },
  // //     }
  // //   },
  // //   {.1, 1},
  // //   default_mesh_options,
  // //   default_visualize,
  // //   default_results_directory
  // // }
};

/* 
   ****************************************************************************
                            File Local Declarations
   ****************************************************************************
*/ 

using Tests = vector<CylCompoundToolpathTest>;
template <class T> static void run_tests(vector<T>& tests);

/* 
   ****************************************************************************
                             File Local Definitions
   ****************************************************************************
*/ 

template <class T>
static void run_tests(const vector<T>& tests)
{ 
    for (const auto& test : tests)
    {
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

        cout << "SUCCESS: The test " << test.name << " succeeded!" << endl;

        cout << "********* FINISH TEST: " << test.name << " **********" << endl;
    }
}

int main()
{
    run_tests(tests);
    return EXIT_SUCCESS;
}
