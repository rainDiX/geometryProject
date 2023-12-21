#pragma once

#include <vtkPolyData.h>
#include <vtkType.h>

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
 * Builds a neighbor map for a given vtkPolyData mesh.
 *
 * @param mesh A pointer to the vtkPolyData mesh.
 *
 * @return An unordered map with vtkIdType keys and unordered sets of vtkIdType values,
 *         representing the neighbor map.
 *
 */
std::unordered_map<vtkIdType, std::unordered_set<vtkIdType>> buildNeighborMap(vtkPolyData* mesh);

/**
 * Generates an Harmonic function for the given parameters.
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
 * Generates an iterative Harmonic function for the given parameters.
 * At i = 0, f(ptId)=1.0, f(pt\ptId) == 0
 * f'(v) = alpha * (1/N) * \sum_j^N{f(v_j)} + (1 - alpha)*f(v_i) where N is the number of neighbors of v
 * @param mesh The pointer to the vtkPolyData object.
 * @param ptId The ID of the point.
 * @param alpha weight parameter (between 0 and 1/2)
 * @param iterations the number of iterations
 *
 * @return The harmonic weight function
 *
 */
std::function<double(vtkIdType)> iterativeHarmonic(vtkPolyData* mesh, vtkIdType ptId, double alpha, int iterations = 0);


// void laplacianMatrix();