#include "harmonicFn.hpp"

#include <vtkCellArrayIterator.h>
#include <vtkIdList.h>
#include <vtkSmartPointer.h>
#include <vtkType.h>

#include <unordered_map>

std::unordered_map<vtkIdType, long> buildRingMap(vtkPolyData* mesh, vtkIdType initPtId, long ringCount) {
    std::unordered_map<vtkIdType, long> ringMap;
    auto currentRing = vtkSmartPointer<vtkIdList>::New();
    auto nextRing = vtkSmartPointer<vtkIdList>::New();
    vtkSmartPointer<vtkIdList> temp;

    vtkCellArray* polys = mesh->GetPolys();

    currentRing->InsertNextId(initPtId);

    for (int i = 0; i < ringCount; ++i) {
        for (vtkIdType j = 0; j < currentRing->GetNumberOfIds(); ++j) {
            auto pointId = currentRing->GetId(j);
            if (!ringMap.contains(pointId)) {
                ringMap[pointId] = i;

                auto it = vtk::TakeSmartPointer(polys->NewIterator());
                for (it->GoToFirstCell(); !it->IsDoneWithTraversal(); it->GoToNextCell()) {
                    auto cell = it->GetCurrentCell();
                    bool containsPoint = false;
                    for (vtkIdType pt = 0; pt < cell->GetNumberOfIds(); ++pt) {
                        containsPoint = containsPoint || (pointId == cell->GetId(pt));
                    }
                    if (containsPoint) {
                        for (vtkIdType pt = 0; pt < cell->GetNumberOfIds(); ++pt) {
                            auto ptId = cell->GetId(pt);
                            if (pointId != ptId) nextRing->InsertUniqueId(ptId);
                        }
                    }
                }
            }
        }
        currentRing->Reset();
        temp = currentRing;
        currentRing = nextRing;
        nextRing = temp;
    }
    return ringMap;
}

std::function<double(vtkIdType)> simpleHarmonic(vtkPolyData* mesh, vtkIdType pointId, long ringCount) {
    auto ringMap = buildRingMap(mesh, pointId, ringCount);
    return [=](vtkIdType ptId) {
        if (auto search = ringMap.find(ptId); search != ringMap.end()) {
            return static_cast<double>(ringCount - search->second) / static_cast<double>(ringCount);
        } else {
            return 0.0;
        }
    };
}

std::unordered_map<vtkIdType, std::unordered_set<vtkIdType>> buildNeighborMap(vtkPolyData* mesh) {
    std::unordered_map<vtkIdType, std::unordered_set<vtkIdType>> neighborMap;
    vtkCellArray* polys = mesh->GetPolys();

    auto it = vtk::TakeSmartPointer(polys->NewIterator());
    for (it->GoToFirstCell(); !it->IsDoneWithTraversal(); it->GoToNextCell()) {
        auto cell = it->GetCurrentCell();
        for (vtkIdType i = 0; i < cell->GetNumberOfIds(); ++i) {
            auto ptId = cell->GetId(i);

            for (vtkIdType j = 0; j < cell->GetNumberOfIds(); ++j) {
                if (i != j) {
                    if (auto search = neighborMap.find(ptId); search != neighborMap.end()) {
                        search->second.insert(cell->GetId(j));
                    } else {
                        neighborMap[ptId] = {cell->GetId(j)};
                    }
                }
            }
        }
    }
    return neighborMap;
}

std::function<double(vtkIdType)> iterativeHarmonic(vtkPolyData* mesh, vtkIdType ptId, double alpha, int iterations) {
    auto neighborMap = buildNeighborMap(mesh);
    std::unordered_map<vtkIdType, double> f;
    std::unordered_map<vtkIdType, double> fi;
    f[ptId] = 1.0;
    for (int i = 0; i < iterations; ++i) {
        for (vtkIdType ptId = 0; ptId < mesh->GetNumberOfPoints(); ++ptId) {
            auto neighbors = neighborMap[ptId];
            double weight = 0.0;
            double weightOfNeighbors = 0.0;
            if (auto search = f.find(ptId); search != f.end()) {
                weight = search->second;
            }
            for (auto neighbor : neighbors) {
                if (auto search = f.find(neighbor); search != f.end()) {
                    weightOfNeighbors += search->second;
                }
            }
            if (weight > 0.0 || weightOfNeighbors > 0.0) {
                std::cerr << ptId << '\n';
                fi[ptId] = alpha * (1.0 / neighbors.size()) * weightOfNeighbors + (1.0 - alpha) * weight;
            }
        }
        f = fi;
    }
    return [=](vtkIdType ptId) {
        if (auto search = f.find(ptId); search != f.end()) {
            return search->second;
        } else {
            return 0.0;
        }
    };
}