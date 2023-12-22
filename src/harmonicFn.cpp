#include "harmonicFn.hpp"

#include <Eigen/src/Core/Matrix.h>
#include <Eigen/src/Core/util/Constants.h>
#include <Eigen/src/SparseCore/SparseMatrix.h>
#include <vtkCellArray.h>
#include <vtkCellArrayIterator.h>
#include <vtkIdList.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkType.h>

#include <algorithm>
#include <array>
#include <iostream>
#include <utility>

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

vtkSmartPointer<vtkCellArray> getRingTriangles(vtkPolyData* mesh, const std::unordered_map<vtkIdType, long>& ringMap) {
    vtkCellArray* polys = mesh->GetPolys();
    auto triangles = vtkSmartPointer<vtkCellArray>::New();

    auto it = vtk::TakeSmartPointer(polys->NewIterator());
    for (it->GoToFirstCell(); !it->IsDoneWithTraversal(); it->GoToNextCell()) {
        auto cell = it->GetCurrentCell();
        auto withinRing = true;
        for (vtkIdType i = 0; i < cell->GetNumberOfIds(); ++i) {
            auto ptId = cell->GetId(i);
            withinRing = withinRing && ringMap.contains(ptId);
        }
        if (withinRing) {
            triangles->InsertNextCell(cell);
        }
    }
    return triangles;
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

std::function<double(vtkIdType)> laplacianDiffusion(vtkPolyData* mesh, vtkIdType ptId, double alpha, int iterations) {
    auto neighborMap = buildNeighborMap(mesh);
    std::unordered_map<vtkIdType, double> f;  // currentValue
    std::unordered_map<vtkIdType, double> g;  // newValue
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
                g[ptId] = (1.0 - alpha) * weight + alpha * (1.0 / neighbors.size()) * weightOfNeighbors;
            }
        }
        f = g;
    }
    return [=](vtkIdType ptId) {
        if (auto search = f.find(ptId); search != f.end()) {
            return search->second;
        } else {
            return 0.0;
        }
    };
}

Eigen::SparseMatrix<double> laplacianMatrix(vtkPolyData* mesh, vtkIdType ptId,
                                            const std::map<vtkIdType, long>& pointMap,
                                            const std::unordered_map<vtkIdType, long>& ringMap, long lastRingStart) {
    using namespace Eigen;

    auto triangles = getRingTriangles(mesh, ringMap);
    auto nbPoints = ringMap.size();

    SparseMatrix<double> L(nbPoints, nbPoints);
    L.setZero();

    auto it = vtk::TakeSmartPointer(triangles->NewIterator());
    for (it->GoToFirstCell(); !it->IsDoneWithTraversal(); it->GoToNextCell()) {
        auto triangle = it->GetCurrentCell();
        struct Edge {
            vtkIdType i, j, orig;
        };
        const std::array<Edge, 3> edges = {{{0, 1, 2}, {0, 2, 1}, {1, 2, 0}}};
        for (const auto& edge : edges) {
            vtkIdType I = triangle->GetId(edge.i);
            vtkIdType J = triangle->GetId(edge.j);
            vtkIdType Orig = triangle->GetId(edge.orig);

            long i = pointMap.find(I)->second;
            long j = pointMap.find(J)->second;

            Vector3d pi = Map<Vector3d>(mesh->GetPoint(I));
            Vector3d pj = Map<Vector3d>(mesh->GetPoint(J));
            Vector3d porig = Map<Vector3d>(mesh->GetPoint(Orig));

            Vector3d v1 = porig - pi;
            Vector3d v2 = porig - pj;

            double halfCotan = 0.5 * (v1.dot(v2)) / ((v1.cross(v2)).norm());
            if (i < lastRingStart) {
                L.coeffRef(i, j) += halfCotan;
                L.coeffRef(i, i) -= halfCotan;
            }
            if (j < lastRingStart) {
                L.coeffRef(j, j) -= halfCotan;
                L.coeffRef(j, i) += halfCotan;
            }
        }
    }

    for (long i = lastRingStart; i < nbPoints; ++i) {
        L.coeffRef(i, i) = 1;
    }
    return L;
}

std::function<double(vtkIdType)> solveLaplace(vtkPolyData* mesh, vtkIdType ptId, int ringCount) {
    using namespace Eigen;
    auto ringMap = buildRingMap(mesh, ptId, ringCount);
    auto nbPoints = ringMap.size();
    // keep track of the ordering since I did not until now...
    // and also keep track of the borders
    std::map<vtkIdType, long> pointMap;
    long pos = 0;
    long lastRingStart = nbPoints;
    for (long i = 0; i < ringCount; ++i) {
        for (auto pt_ring : ringMap) {
            vtkIdType ptid = pt_ring.first;
            vtkIdType ring = pt_ring.second;
            if (i == ring) {
                pointMap[ptid] = pos;
                if (ring == ringCount - 1) {
                    lastRingStart = std::min(lastRingStart, pos);
                }
                ++pos;
            }
        }
    }
    auto L = laplacianMatrix(mesh, ptId, pointMap, ringMap, lastRingStart);

    std::cout << "Laplacian Mat\n";
    std::cout << L << '\n';

    SparseLU<SparseMatrix<double>> solver;
    solver.compute(L);

    VectorXd rhs = VectorXd::Constant(nbPoints, 0.0);
    rhs(0) = 1.0;

    VectorXd res = solver.solve(rhs);

    if (solver.info() != Success) {
        std::cerr << "Solving failed!" << std::endl;
    }

    std::cout << "Result\n";
    std::cout << res << '\n';

    return [=](vtkIdType ptId) {
        if (auto search = pointMap.find(ptId); search != pointMap.end()) {
            return std::abs(res[search->second]);
        } else {
            return 0.0;
        }
    };
}