set(vtkImGuiAdapter_headers
    vtkImGuiSDL2RenderWindowInteractor.h
    vtkImGuiSDL2OpenGLRenderWindow.h
  )

set(vtkImGuiAdapter_sources
    vtkImGuiSDL2RenderWindowInteractor.cxx
    vtkImGuiSDL2OpenGLRenderWindow.cxx
  )

add_library(vtkImGuiAdapter ${vtkImGuiAdapter_sources})

target_link_libraries(vtkImGuiAdapter PUBLIC
        ${EXTRA_LIBS}
        ${VTK_LIBRARIES}
        imgui-vtk
)

add_library(vtkImGuiAdapter::vtkImGuiAdapter ALIAS vtkImGuiAdapter)

target_include_directories(vtkImGuiAdapter PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )