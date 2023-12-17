#pragma once

#include <nfd.h>
#include <vtkImGuiSDL2OpenGLRenderWindow.h>
#include <vtkImGuiSDL2RenderWindowInteractor.h>
#include <vtkNew.h>
#include <vtkRenderer.h>

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
    bool running = false;
    bool showActorWindow = false;
    vtkNew<vtkRenderer> renderer;
    vtkNew<vtkImGuiSDL2OpenGLRenderWindow> renWin;
    vtkNew<vtkImGuiSDL2RenderWindowInteractor> iren;
    SDL_Window* window = nullptr;
    SDL_GLContext imgui_context = nullptr;
    nfdresult_t nfd_initialized;
};