#pragma once

#include <nfd.h>
#include <vtkImGuiSDL2OpenGLRenderWindow.h>
#include <vtkImGuiSDL2RenderWindowInteractor.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkNew.h>
#include <vtkRenderer.h>

#include "MouseInteractorStylePP.hpp"

class Application {
   public:
    Application();
    ~Application();
    Application(Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&) = delete;

    void mainLoop();

   private:
    void mainWindow();
    bool m_running = false;
    bool m_showActorWindow = false;
    bool m_showFunctionsWindow = false;
    bool m_picking = false;
    vtkNew<vtkRenderer> m_renderer;
    vtkNew<vtkImGuiSDL2OpenGLRenderWindow> m_renWin;
    vtkNew<vtkImGuiSDL2RenderWindowInteractor> m_iren;
    vtkNew<vtkInteractorStyleSwitch> m_defaultStyle;
    vtkNew<MouseInteractorStylePP> m_pickingStyle;
    SDL_Window* m_window = nullptr;
    SDL_GLContext m_imguiContext = nullptr;
    nfdresult_t m_nfdInitialized;
};