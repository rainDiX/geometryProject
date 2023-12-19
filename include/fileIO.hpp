#pragma once

#include <nfd.h>
#include <vtkActor.h>

#include <filesystem>
#include <optional>

std::optional<std::filesystem::path> pickModelFile();

vtkNew<vtkActor> openObjectFile(const std::filesystem::path &path);

vtkNew<vtkActor> openPLYFile(const std::filesystem::path &path);
