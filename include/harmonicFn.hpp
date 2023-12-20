#pragma once

#include <vtkPolyData.h>
#include <vtkType.h>

#include <functional>
#include <unordered_map>

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
 * Generates an Harmonic function for the given parameters.
 *
 * @param mesh The pointer to the vtkPolyData object.
 * @param pointId The ID of the point.
 * @param ringCount The number of rings.
 *
 * @return The harmonic function that calculates the weight of the point, h(p) = (ringCount - ring(p)) / ringCount.
 *
 */
std::function<double(vtkIdType)> simpleHarmonic(vtkPolyData* mesh, vtkIdType pointId, long ringCount);

// void laplacianMatrix();