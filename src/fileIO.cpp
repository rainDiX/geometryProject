#include "fileIO.hpp"

#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataMapper.h>

#include <format>
#include <iostream>

#include "nfd.h"
#ifdef _WIN32
#include <string>
#endif  // _WIN32

std::optional<std::filesystem::path> pickModelFile() {
    nfdchar_t *outPath;
    nfdfilteritem_t filterItem[2] = {{"Wavefront OBJ (.obj)", "obj"}, {"Polygon File Format (.ply)", "gltf"}};
    nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 2, NULL);

    if (result != NFD_OKAY && result != NFD_CANCEL) {
        std::cerr << std::format("Error: {}\n", NFD_GetError());
    }
    if (result == NFD_OKAY) {
#ifdef _WIN32
        std::wstring ws(outPath);
        NFD_FreePath(outPath);
        std::string path();
        std::filesystem::path p{ws.begin(), ws.end()};
#else
        std::filesystem::path path{std::move(outPath)};
        return path;
#endif  // _WIN32
    }
    return std::nullopt;
}

template <typename Reader>
vtkNew<vtkActor> openFile(const std::filesystem::path &path) {
    vtkNew<Reader> reader;
    reader->SetFileName(path.c_str());
    reader->Update();

    vtkNew<vtkPolyDataMapper> meshMapper;
    meshMapper->SetInputConnection(reader->GetOutputPort());

    vtkNew<vtkActor> meshActor;
    meshActor->SetMapper(meshMapper);

    meshActor->SetObjectName(path.filename());

    return meshActor;
}

vtkNew<vtkActor> openObjectFile(const std::filesystem::path &path) { return openFile<vtkOBJReader>(path); };

vtkNew<vtkActor> openPLYFile(const std::filesystem::path &path) { return openFile<vtkPLYReader>(path); };
