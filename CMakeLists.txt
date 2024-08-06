cmake_minimum_required(VERSION 3.28)
project(surfacic_toolpaths LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenCASCADE REQUIRED)

add_executable(surfacic_toolpaths
               src/main.cpp
               src/brep_builder.cpp
               src/brep_to_stl.cpp
               src/util.cpp)

target_include_directories(surfacic_toolpaths PRIVATE 
                           ${OpenCASCADE_INCLUDE_DIR}
                          )

target_link_libraries(surfacic_toolpaths PRIVATE 
                      ${OpenCASCADE_ModelingAlgorithms_LIBRARIES}
                     )
