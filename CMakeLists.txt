cmake_minimum_required(VERSION 3.28)
project(surfacic_toolpaths LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# ------------------------------------------------------------------------------
#                                   Library
# ------------------------------------------------------------------------------

find_package(OpenCASCADE REQUIRED)

add_library(surfacic_toolpaths SHARED
            "${CMAKE_SOURCE_DIR}/src/brep_builder.cpp"
            "${CMAKE_SOURCE_DIR}/src/brep_to_stl.cpp"
            "${CMAKE_SOURCE_DIR}/src/util.cpp")

target_include_directories(surfacic_toolpaths 
                           PUBLIC
                           "${CMAKE_SOURCE_DIR}"
                          )


target_link_libraries(surfacic_toolpaths
                      PUBLIC
                      ${OpenCASCADE_LIBRARIES}
                     )

# ------------------------------------------------------------------------------
#                                   Tests 
# ------------------------------------------------------------------------------

add_executable(test "${CMAKE_SOURCE_DIR}/tests/test.cpp")

target_link_libraries(test PRIVATE
                      surfacic_toolpaths
                     )
