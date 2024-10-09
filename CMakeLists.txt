cmake_minimum_required(VERSION 3.28)
project(surfacic_toolpaths LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ------------------------------------------------------------------------------
#                              Building the Library 
# ------------------------------------------------------------------------------

find_package(OpenCASCADE CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

# All source files with relative paths. 
set(source_files_relative_path
    "util.cpp"
    "tool_curve.cpp"
    "toolpath.cpp"
    "visualization/glfw_occt_view.cpp"
    "visualization/glfw_occt_window.cpp"
   )

# All source files with absolute paths.
list(TRANSFORM source_files_relative_path 
     PREPEND "${CMAKE_SOURCE_DIR}/src/" 
     OUTPUT_VARIABLE source_files_absolute_path
    )

add_library(${PROJECT_NAME} SHARED ${source_files_absolute_path})

target_include_directories(${PROJECT_NAME} 
                           PRIVATE
                           "${CMAKE_SOURCE_DIR}/src/include"
                           PUBLIC 
                           $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/>
                           $<INSTALL_INTERFACE:include/>
                          )

# All header files that are public. 
set(public_header_files_relative_path
    "geometric_primitives.hxx"
    "tool_curve.hxx"
    "tool_profile.hxx"
    "toolpath.hxx"
   )

# All header files with absolute paths.
list(TRANSFORM public_header_files_relative_path
     PREPEND "${CMAKE_SOURCE_DIR}/include/" 
     OUTPUT_VARIABLE public_header_files_absolute_path
    )

target_link_libraries(${PROJECT_NAME}
                      PUBLIC
                      ${OpenCASCADE_LIBRARIES}
                      glfw
                     )

# ------------------------------------------------------------------------------
#                             Installing the Library 
# ------------------------------------------------------------------------------

# Prepare the export set. In this case, the set has just one element - a library.
set(lib_export_set "${PROJECT_NAME}Targets")
install(TARGETS ${PROJECT_NAME}
        EXPORT ${lib_export_set} 
       )

# Install all the public headers. This technique is isn't fully modern, but
#     target_sources(FILE_SET) is...confusing...
install(FILES ${public_header_files_absolute_path} TYPE INCLUDE)

set(cmake_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}/")

# Specify where the CMake file that creates imported targets should be placed. 
install(EXPORT ${lib_export_set} 
        DESTINATION ${cmake_install_dir}  
       )

# Produce the Config file in the build directory. 
configure_package_config_file("${CMAKE_SOURCE_DIR}/Config.cmake.in"
                              "${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${cmake_install_dir} 
                             )

# Install the Config file. 
install(FILES "${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        DESTINATION ${cmake_install_dir}
       )

# ------------------------------------------------------------------------------
#                             Testing the Library
# ------------------------------------------------------------------------------

add_executable(test "${CMAKE_SOURCE_DIR}/tests/test.cpp")

target_link_libraries(test PRIVATE
                      ${PROJECT_NAME}
                     )
