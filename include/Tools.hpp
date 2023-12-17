#pragma once

#include <vtkRenderer.h>
#include <vtkNew.h>
#include <vtkImGuiSDL2RenderWindowInteractor.h>


void actorListWindow(vtkRenderer* renderer, bool& open);

void actorManagerWindow(vtkRenderer* renderer, vtkActor* actor, bool& open);
