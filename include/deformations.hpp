#pragma once

#include <vtkPolyData.h>
#include <vtkType.h>

#include <functional>

/**
 * Performs Laplacian smoothing on a vtkPolyData mesh.
 * xi = (1/N) * \sum_{j \in Neighbors_i} xj
 *
 * @param mesh The vtkPolyData mesh to be smoothed.
 * @param numIterations The number of smoothing iterations to be performed.
 *
 */
void laplacianSmoothing(vtkPolyData* mesh, int numIterations);

/**
 * Translates a point in the mesh by dis in the normal direction and weighted by the weight function
 *
 * @param mesh a pointer to the vtkPolyData object representing the mesh
 * @param ptId the ID of the point in the mesh
 * @param dist the distance to translate the point
 * @param weightFn
 *
 */
void weightedTranslate(vtkPolyData* mesh, vtkIdType ptId, double dist, std::function<double(vtkIdType)> weightFn);