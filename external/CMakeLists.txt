file(GLOB imgui_src_common imgui/*.cpp)

set(imgui_sources
        ${imgui_src_common}
        imgui/backends/imgui_impl_opengl3.cpp
        imgui/backends/imgui_impl_sdl2.cpp
        )

file(GLOB imgui_headers_common imgui/*.h)

set(imgui_headers_opengl
        imgui/backends/imgui_impl_opengl3.h
        )
set(imgui_headers_sdl
        imgui/backends/imgui_impl_sdl2.h
        )
set(imgui_headers_all
    ${imgui_headers_common}
    ${imgui_headers_opengl}
    ${imgui_headers_sdl}
    )

find_package(SDL2 CONFIG REQUIRED)
find_package(GLEW REQUIRED)

# opengl
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

add_library(imgui-vtk ${imgui_sources})
add_library(vtkImGuiAdapter::imgui-vtk ALIAS imgui-vtk)
target_link_libraries(imgui-vtk PUBLIC OpenGL::EGL)
target_link_libraries(imgui-vtk PUBLIC SDL2::SDL2)
target_link_libraries(imgui-vtk PUBLIC GLEW::GLEW)
target_include_directories(imgui-vtk PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
