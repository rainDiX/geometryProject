#include "deformations.hpp"

#include <vtkPointData.h>
#include <vtkPolyDataNormals.h>

#include "harmonicFn.hpp"

void laplacianSmoothing(vtkPolyData* mesh, int numIterations) {
    auto neighbors = buildNeighborMap(mesh);

    auto smoothedMesh = vtkSmartPointer<vtkPolyData>::New();
    smoothedMesh->DeepCopy(mesh);
    auto points = smoothedMesh->GetPoints();

    for (int i = 0; i < numIterations; ++i) {
        vtkSmartPointer<vtkPoints> newPoints = vtkSmartPointer<vtkPoints>::New();
        newPoints->DeepCopy(points);

        for (vtkIdType ptId = 0; ptId < points->GetNumberOfPoints(); ++ptId) {
            double laplacian[3] = {0.0, 0.0, 0.0};

            if (auto search = neighbors.find(ptId); search != neighbors.end()) {
                long numNeighbors = search->second.size();
                double neighborPosition[3] = {0.0, 0.0, 0.0};
                for (auto neighbor : search->second) {
                    points->GetPoint(neighbor, neighborPosition);
                    for (int i = 0; i < 3; ++i) {
                        laplacian[i] += neighborPosition[i];
                    }
                }
                double currentPosition[3];
                points->GetPoint(ptId, currentPosition);

                double newPosition[3];
                for (int i = 0; i < 3; ++i) {
                    newPosition[i] = (currentPosition[i] + laplacian[i]) / (numNeighbors + 1);
                }
                newPoints->SetPoint(ptId, newPosition);
            }
        }
        points->DeepCopy(newPoints);
    }
    mesh->DeepCopy(smoothedMesh);
}

void weightedTranslate(vtkPolyData* mesh, vtkIdType ptId, double dist, std::function<double(vtkIdType)> weightFn) {
    auto normals = vtkSmartPointer<vtkPolyDataNormals>::New();
    normals->SetInputData(mesh);
    normals->ComputeCellNormalsOff();
    normals->ComputePointNormalsOn();
    normals->Update();
    auto out = normals->GetOutput();
    double* normal = out->GetPointData()->GetNormals()->GetTuple(ptId);
    double max = weightFn(ptId);

    auto newMesh = vtkSmartPointer<vtkPolyData>::New();
    newMesh->DeepCopy(mesh);

    for (vtkIdType p = 0; p < mesh->GetNumberOfPoints(); ++p) {
        double weight = weightFn(p);
        double normalized = weight / max;
        double translation[3] = {dist * normalized * normal[0], dist * normalized * normal[1],
                                 dist * normalized * normal[2]};
        double* currentPosition = mesh->GetPoints()->GetPoint(p);
        double newPosition[3];
        for (int i = 0; i < 3; ++i) {
            newPosition[i] = currentPosition[i] + translation[i];
        }
        newMesh->GetPoints()->SetPoint(p, newPosition);
    }
    mesh->DeepCopy(newMesh);
}