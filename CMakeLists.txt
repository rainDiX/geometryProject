cmake_minimum_required(VERSION 3.25)

project(geometry 
  VERSION 0.1.0
  LANGUAGES C CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
include(GNUInstallDirs)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No CMAKE_BUILD_TYPE specified, default to Release.")
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(vtk_components
  CommonCore
  CommonDataModel
  IOGeometry
  IOPLY
  RenderingCore
  RenderingOpenGL2
  RenderingVolumeOpenGL2
  InteractionStyle
  )

find_package(VTK 9.0 REQUIRED COMPONENTS
             ${vtk_components}
             CONFIG)

message(STATUS "VTK_VERSION: ${VTK_VERSION}")
message(STATUS "VTK_LIBRARIES: ${VTK_LIBRARIES}")

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

add_subdirectory(external)

set(geo_include_dir "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory(src)
