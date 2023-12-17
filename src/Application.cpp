#include "Application.hpp"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <vtkCamera.h>
#include <vtkInteractorStyleSwitch.h>

#include <format>
#include <iostream>
#include <map>

#include "Tools.hpp"
#include "fileIO.hpp"

Application::Application() {
    renWin->AddRenderer(renderer);
    iren->SetRenderWindow(renWin);
    renderer->SetBackground(0.2, 0.2, 0.2);
    vtkNew<vtkInteractorStyleSwitch> style;
    style->SetCurrentStyleToTrackballCamera();
    style->SetDefaultRenderer(renderer);
    iren->SetInteractorStyle(style);
    renWin->SetSize(800, 800);
    iren->Initialize();

    // GUI Setup
    window = static_cast<SDL_Window *>(renWin->GetGenericWindowId());
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO &io = ImGui::GetIO();
    iren->SetImguiIO(&io);
    imgui_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, imgui_context);
    SDL_GL_SetSwapInterval(1);  // Enable vsync
    ImGui_ImplSDL2_InitForOpenGL(window, imgui_context);
    ImGui_ImplOpenGL3_Init();

    // file picker
    nfd_initialized = NFD_Init();
    if (nfd_initialized != NFD_OKAY) {
        std::cerr << "NFD failed to initialize, filepicker wont' work\n";
        std::cerr << std::format("Error: {}\n", NFD_GetError());
    }
}

Application::~Application() {
    if (nfd_initialized == NFD_OKAY) {
        NFD_Quit();
    }
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(imgui_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Application::mainLoop() {
    running = true;
    while (running) {
        renWin->Render();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
                running = false;
            renWin->PushContext();
            running = !iren->ProcessEvent(&event);
            renWin->PopContext();
        }

        SDL_GL_MakeCurrent(window, imgui_context);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        mainWindow();
        if (showActorWindow) actorListWindow(renderer, showActorWindow);
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }  // render loop
}

void Application::mainWindow() {
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Geometry Project", nullptr, ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */
                    auto path = pickModelFile();
                    if (path.has_value() && path->has_extension()) {
                        if (path->extension() == ".obj") {
                            auto actor = openObjectFile(*path);
                            renderer->AddActor(actor);
                        } else {
                            std::cout << "unknown file type" << std::endl;
                        }
                    } else {
                        std::cout << "unable to detect extension" << std::endl;
                    }
                }
                if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Do stuff */
                }
                if (ImGui::MenuItem("Quit", "Ctrl+Q")) {
                    running = false;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Tools")) {
                if (ImGui::MenuItem("Actors")) {
                    showActorWindow = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        if (ImGui::CollapsingHeader("View")) {
            double *color = renderer->GetBackground();
            float colorf[] = {float(color[0]), float(color[1]), float(color[2]), 1.0f};
            if (ImGui::ColorEdit3("Background", colorf)) {
                renderer->SetBackground(colorf[0], colorf[1], colorf[2]);
            }

            auto style = dynamic_cast<vtkInteractorStyleSwitch *>(iren->GetInteractorStyle());
            std::map<std::string, int> style_map = {{"vtkInteractorStyleTrackballCamera", 0},
                                                    {"vtkInteractorStyleJoystickCamera", 1},
                                                    {"vtkInteractorStyleMultiTouchCamera", 2}};
            const char *styles[] = {"Trackball", "Joystick", "Multitouch"};
            auto current_class = style_map[style->GetCurrentStyle()->GetClassName()];
            if (ImGui::Combo("Camera Style", &current_class, styles, IM_ARRAYSIZE(styles))) {
                switch (current_class) {
                    case 0:
                        style->SetCurrentStyleToTrackballCamera();
                        break;
                    case 1:
                        style->SetCurrentStyleToJoystickCamera();
                        break;
                    case 2:
                        style->SetCurrentStyleToMultiTouchCamera();
                        break;
                    default:
                        style->SetCurrentStyleToTrackballCamera();
                }
            }
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);
            ImGui::Text("Camera Parameters");
            auto camera = renderer->GetActiveCamera();
            float fov = float(camera->GetViewAngle());
            if (ImGui::SliderFloat("Field of view", &fov, 0.0f, 360.0f)) {
                camera->SetViewAngle(fov);
            }
            float clipNear = camera->GetClippingRange()[0];
            float clipFar = camera->GetClippingRange()[1];
            if (ImGui::SliderFloat("Clip Near", &clipNear, 0.0, 1.0, "%.5f")) {
                camera->SetClippingRange(clipNear, clipFar);
            }
            if (ImGui::SliderFloat("Clip Far", &clipNear, 1.0, 2000.0, "%.1f")) {
                camera->SetClippingRange(clipNear, clipFar);
            }
            if (ImGui::CollapsingHeader("Camera Position")) {
                double *position = camera->GetPosition();
                float positionf[3] = {static_cast<float>(position[0]), static_cast<float>(position[1]),
                                      static_cast<float>(position[2])};
                if (ImGui::InputFloat3("Position", positionf)) {
                    camera->SetPosition(positionf[0], positionf[1], positionf[2]);
                }
            }
        }

        auto size = ImGui::GetIO().DisplaySize;
        ImGui::Text("%.3f ms/frame (%.1f FPS) %.1f x %.1f", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate, size.x, size.y);
        ImGui::End();
    }
}