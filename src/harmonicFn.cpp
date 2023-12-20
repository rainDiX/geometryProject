#include "harmonicFn.hpp"

#include <vtkCellArrayIterator.h>
#include <vtkIdList.h>
#include <vtkSmartPointer.h>
#include <vtkType.h>


std::unordered_map<vtkIdType, long> buildRingMap(vtkPolyData* mesh, vtkIdType initPointId, long ringCount) {
    std::unordered_map<vtkIdType, long> ringMap;
    auto currentRing = vtkSmartPointer<vtkIdList>::New();
    auto nextRing = vtkSmartPointer<vtkIdList>::New();
    vtkSmartPointer<vtkIdList> temp;

    vtkCellArray* polys = mesh->GetPolys();

    currentRing->InsertNextId(initPointId);

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