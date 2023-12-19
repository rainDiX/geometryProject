#pragma once

#include <vtkRenderer.h>
#include <vtkNew.h>

#include "MouseInteractorStylePP.hpp"

void actorListWindow(vtkRenderer* renderer, bool& open, int& seleted);

void functionsWindow(vtkRenderer* renderer, MouseInteractorStylePP* picker, bool& open, bool& picking);
