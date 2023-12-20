#pragma once

#include <nfd.h>
#include <vtkImGuiSDL2OpenGLRenderWindow.h>
#include <vtkImGuiSDL2RenderWindowInteractor.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkNew.h>
#include <vtkRenderer.h>
#include <memory>

#include "MouseInteractorStylePP.hpp"
#include "Tools.hpp"

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
    bool m_picking = false;
    vtkNew<vtkRenderer> m_renderer;
    vtkNew<vtkImGuiSDL2OpenGLRenderWindow> m_renWin;
    vtkNew<vtkImGuiSDL2RenderWindowInteractor> m_iren;
    vtkNew<vtkInteractorStyleSwitch> m_defaultStyle;
    vtkNew<MouseInteractorStylePP> m_pickingStyle;
    std::unique_ptr<Tools> m_tools = nullptr;
    SDL_Window* m_window = nullptr;
    SDL_GLContext m_imguiContext = nullptr;
    nfdresult_t m_nfdInitialized;
};