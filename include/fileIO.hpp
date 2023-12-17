#pragma once

#include <nfd.h>
#include <vtkActor.h>
#include <vtkOBJReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>

#include <filesystem>
#include <optional>

std::optional<std::filesystem::path> pickModelFile();

vtkNew<vtkActor> openObjectFile(const std::filesystem::path &path);