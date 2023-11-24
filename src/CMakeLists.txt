add_subdirectory("vtkImGuiAdapter")

add_executable(geo)

target_link_libraries(geo vtkImGuiAdapter)

vtk_module_autoinit(
  TARGETS geo
  MODULES ${VTK_LIBRARIES}
  )

target_sources(geo PRIVATE draw.cpp)

