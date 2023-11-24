/*=========================================================================

  Program:   Visualization Toolkit
  Module:    Mace.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImGuiAdapter/vtkImGuiSDL2OpenGLRenderWindow.h"
#include "vtkImGuiAdapter/vtkImGuiSDL2RenderWindowInteractor.h"

#include <vtkActor.h>
#include <vtkConeSource.h>
#include <vtkGlyph3D.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkInteractorStyleSwitch.h>
#include <vtkSphereSource.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#if (defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV))
#include <OpenGLES/ES3/gl.h>
#else
#include <GLES3/gl3.h>
#endif

int main(int, char**)
{
  // VTK side, draw
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkImGuiSDL2OpenGLRenderWindow> renWin;
  renWin->AddRenderer(renderer);
  vtkNew<vtkImGuiSDL2RenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin);

  vtkNew<vtkInteractorStyleSwitch> style;
  style->SetCurrentStyleToTrackballCamera();
  style->SetDefaultRenderer(renderer);
  iren->SetInteractorStyle(style);

  vtkNew<vtkSphereSource> sphere;
  sphere->SetThetaResolution(8);
  sphere->SetPhiResolution(8);
  vtkNew<vtkPolyDataMapper> sphereMapper;
  sphereMapper->SetInputConnection(sphere->GetOutputPort());
  vtkNew<vtkActor> sphereActor;
  sphereActor->SetMapper(sphereMapper);

  vtkNew<vtkConeSource> cone;
  cone->SetResolution(6);

  vtkNew<vtkGlyph3D> glyph;
  glyph->SetInputConnection(sphere->GetOutputPort());
  glyph->SetSourceConnection(cone->GetOutputPort());
  glyph->SetVectorModeToUseNormal();
  glyph->SetScaleModeToScaleByVector();
  glyph->SetScaleFactor(0.25);

  vtkNew<vtkPolyDataMapper> spikeMapper;
  spikeMapper->SetInputConnection(glyph->GetOutputPort());

  vtkNew<vtkActor> spikeActor;
  spikeActor->SetMapper(spikeMapper);

  renderer->AddActor(sphereActor);
  renderer->AddActor(spikeActor);
  renderer->SetBackground(0.2, 0.3, 0.4);
  renWin->SetSize(800, 800);

  iren->Initialize();

  // ImGui side. The window is created and handled by VTK, we pass it to ImGui.
  SDL_Window * window = static_cast<SDL_Window*>(renWin->GetGenericWindowId());
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  iren->SetImguiIO(&io);
  SDL_GLContext imgui_context = SDL_GL_CreateContext(window);
  SDL_GL_MakeCurrent(window, imgui_context);
  SDL_GL_SetSwapInterval(1); // Enable vsync

  ImGui_ImplSDL2_InitForOpenGL(window, imgui_context);
  ImGui_ImplOpenGL3_Init();

  // Imgui state
  bool show_demo_window = false;
  // Main loop
  bool done =  false;
  while (!done)
  {
    renWin->Render();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
        done = true;
      if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
          event.window.windowID == SDL_GetWindowID(window))
        done = true;
      renWin->PushContext();
      done = iren->ProcessEvent(&event);
      renWin->PopContext();
    }
    SDL_GL_MakeCurrent(window, imgui_context);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
      ImGui::ShowDemoWindow(&show_demo_window);
    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello imgui-vtk!"); // Create a window called and append into it.

      if (ImGui::Button("Button"))  // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
      ImGui::SameLine();
      ImGui::Text("counter = %d", counter);

      bool spike_actor_visible = static_cast<bool>(spikeActor->GetVisibility());
      ImGui::Checkbox("Show spikes", &spike_actor_visible);
      spikeActor->SetVisibility(static_cast<vtkTypeBool>(spike_actor_visible));

      ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
      ImGui::End();
    }


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
  } // render loop

  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
  SDL_GL_DeleteContext(imgui_context);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
