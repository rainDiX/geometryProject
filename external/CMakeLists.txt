add_subdirectory(imgui)

#cmake_policy(SET CMP0077 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

if(UNIX AND NOT APPLE)
  set(NFD_PORTAL ON)
endif()
add_subdirectory(nativefiledialog-extended)

add_subdirectory(vtkImGuiAdapter)