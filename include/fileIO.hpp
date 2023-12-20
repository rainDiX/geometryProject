#pragma once

#include <nfd.h>
#include <vtkActor.h>
#include <vtkRenderer.h>

#include <filesystem>
#include <optional>

std::optional<std::filesystem::path> pickModelFile();

void openObjectFile(const std::filesystem::path& path, vtkRenderer* renderer);

void openPLYFile(const std::filesystem::path& path, vtkRenderer* renderer);
