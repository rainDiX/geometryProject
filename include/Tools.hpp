#pragma once

#include <vtkNew.h>
#include <vtkRenderer.h>

#include "MouseInteractorStylePP.hpp"

class Tools {
   public:
    Tools() = delete;
    Tools(vtkRenderer* renderer, MouseInteractorStylePP* picker, bool* picking);
    Tools(Tools&) = delete;
    Tools(Tools&&) = delete;
    Tools& operator=(const Tools&) = delete;
    Tools& operator=(Tools&&) = delete;

    void showWindows();
    void actorListWindow();
    void functionsWindow();
    void enableActorListWindow();
    void enableFunctionWindow();
    void cleanup();

   private:
    int m_selectedActor = 0;
    bool m_showFnWindow = false;
    bool m_showActorsWindow = false;
    bool* m_picking;
    int m_weightingMethod = 0;
    float m_alpha = 1.0 / 4.0;
    int m_ringCount = 1;
    float m_colorStart[3] = {1.0, 0.0, 0.0};
    float m_colorEnd[3] = {0.0, 0.0, 1.0};
    float m_colorNeutral[3] = {1.0, 1.0, 1.0};
    vtkActor* m_toRemove = nullptr;
    vtkRenderer* m_renderer;
    MouseInteractorStylePP* m_picker;
};
