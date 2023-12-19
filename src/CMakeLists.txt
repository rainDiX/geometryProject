add_executable(geo)

target_link_libraries(geo PRIVATE vtkImGuiAdapter nfd)

vtk_module_autoinit(
  TARGETS geo
  MODULES ${VTK_LIBRARIES}
  )

target_sources(geo PRIVATE 
  main.cpp
  Tools.cpp
  Application.cpp
  fileIO.cpp
  MouseInteractorStylePP.cpp
)

target_include_directories(geo
    PRIVATE
    ${geo_include_dir}
)