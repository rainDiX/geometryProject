#pragma once

#include <vtkPolyData.h>
#include <vtkType.h>

#include <Eigen/Eigen>
#include <functional>
#include <unordered_map>
#include <unordered_set>

/**
 * Utility function that builds a ring map for a mesh given an initial point , and the number of rings.
 * read the entire mesh ringCount times
 *
 * @param mesh The vtkPolyData mesh to build the ring map from.
 * @param initPointId The initial point ID to start building the ring map from.
 * @param ringCount The number of rings to build in the ring map.
 *
 * @return An unordered map of vtkIdType to double representing the ring map.
 */
std::unordered_map<vtkIdType, long> buildRingMap(vtkPolyData* mesh, vtkIdType initPointId, long ringCount);

/**
 * Builds a neighbor map for a given vtkPolyData triangle mesh.
 *
 * @param mesh A pointer to the vtkPolyData mesh.
 *
 * @return An unordered map with vtkIdType keys and unordered sets of vtkIdType values,
 *         representing the neighbor map.
 *
 */
std::unordered_map<vtkIdType, std::unordered_set<vtkIdType>> buildNeighborMap(vtkPolyData* mesh);

/**
 * Generates an Harmonic function inversely proportial to the ring id.
 *
 * @param mesh The pointer to the vtkPolyData object.
 * @param pointId The ID of the point.
 * @param ringCount The number of rings.
 *
 * @return The harmonic function that calculates the weight of the point, f(v) = (ringCount - ring(v)) / ringCount.
 *
 */
std::function<double(vtkIdType)> simpleHarmonic(vtkPolyData* mesh, vtkIdType pointId, long ringCount);

/**
 * Generates the diffusion process of the laplacian.
 * Initially Value_i = 1 if i == ptId else 0
 * then NewValue_i​ = (1 − alpha) * OldValue_i ​ + alpha * (1/Degree_i) ​* \sum_{j\in Neighbors_i}
 * ​​OldValue_j​
 * @param mesh The pointer to the vtkPolyData object.
 * @param ptId The ID of the point.
 * @param alpha diffusion parameter (between 0 and 1/2)
 * @param iterations the number of iterations
 *
 * @return the laplacian diffusion
 *
 */
std::function<double(vtkIdType)> laplacianDiffusion(vtkPolyData* mesh, vtkIdType ptId, double alpha,
                                                    int iterations = 0);

/**
 * Generates a Laplacian matrix for a given mesh and point ID.
 *
 * @param mesh A pointer to the vtkPolyData mesh.
 * @param ptId The ID of the point.
 * @param pointMap
 * @param ringtMap
 * @param lastRingStart
 *
 * @return The generated Laplacian matrix as an Eigen::SparseMatrix<double>.
 *
 * @throws None.
 */
Eigen::SparseMatrix<double> laplacianMatrix(vtkPolyData* mesh, vtkIdType ptId,
                                            const std::map<vtkIdType, long>& pointMap,
                                            const std::unordered_map<vtkIdType, long>& ringMap, long lastRingStart);

/**
 * Solve the laplace equations
 * @param mesh The pointer to the vtkPolyData object.
 * @param ptId The ID of the point.
 * @param ringCount The number of rings.
 *
 * @return weight computed by solving the system
 *
 */
std::function<double(vtkIdType)> solveLaplace(vtkPolyData* mesh, vtkIdType ptId, int ringCount);
