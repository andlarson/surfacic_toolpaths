cmake_minimum_required(VERSION 3.28)
project(surfacic_toolpaths LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# ------------------------------------------------------------------------------
#                                   Library
# ------------------------------------------------------------------------------

# DEBUG: Find the OpenCASCADE package that I built from source with debug symbols,
#     extra debug logging, etc.
set(OpenCASCADE_ROOT "/Users/andrewlarson/Desktop/misc/programming/misc_installs/OCCT_debug/")
find_package(OpenCASCADE CONFIG REQUIRED)

find_package(glfw3 CONFIG REQUIRED)

add_library(surfacic_toolpaths SHARED
            "${CMAKE_SOURCE_DIR}/src/brep_builder.cpp"
            "${CMAKE_SOURCE_DIR}/src/brep_to_stl.cpp"
            "${CMAKE_SOURCE_DIR}/src/util.cpp"
            "${CMAKE_SOURCE_DIR}/src/tool_curve.cpp"
            "${CMAKE_SOURCE_DIR}/src/visualization/glfw_occt_view.cpp"
            "${CMAKE_SOURCE_DIR}/src/visualization/glfw_occt_window.cpp"
           )

# Make it possible to include the public headers and private headers within the library,
#     but don't expose non-public headers to users of this CMake target.
target_include_directories(surfacic_toolpaths 
                           PRIVATE
                           ${CMAKE_SOURCE_DIR}
                           INTERFACE
                           "${CMAKE_SOURCE_DIR}/include"
                          )

target_link_libraries(surfacic_toolpaths
                      PUBLIC
                      ${OpenCASCADE_LIBRARIES}
                      glfw
                     )

# ------------------------------------------------------------------------------
#                                   Tests 
# ------------------------------------------------------------------------------

add_executable(test "${CMAKE_SOURCE_DIR}/tests/test.cpp")

target_link_libraries(test PRIVATE
                      surfacic_toolpaths
                     )
