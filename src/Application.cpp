#include "Application.hpp"

#include <SDL_mouse.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#include <vtkCamera.h>
#include <vtkPointPicker.h>

#include <format>
#include <iostream>
#include <map>
#include <memory>

#include "deformations.hpp"
#include "fileIO.hpp"

Application::Application() {
    m_renWin->AddRenderer(m_renderer);
    m_iren->SetRenderWindow(m_renWin);
    m_renderer->SetBackground(0.2, 0.2, 0.2);
    m_pickingStyle->SetDefaultRenderer(m_renderer);
    m_defaultStyle->SetCurrentStyleToTrackballCamera();
    m_defaultStyle->SetDefaultRenderer(m_renderer);
    m_iren->SetInteractorStyle(m_defaultStyle);
    vtkNew<vtkPointPicker> pointPicker;
    m_iren->SetPicker(pointPicker);
    m_renWin->SetSize(800, 800);
    m_iren->Initialize();

    // GUI Setup
    m_window = static_cast<SDL_Window *>(m_renWin->GetGenericWindowId());
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO &io = ImGui::GetIO();
    m_iren->SetImguiIO(&io);
    m_imguiContext = SDL_GL_CreateContext(m_window);
    SDL_GL_MakeCurrent(m_window, m_imguiContext);
    SDL_GL_SetSwapInterval(1);  // Enable vsync
    ImGui_ImplSDL2_InitForOpenGL(m_window, m_imguiContext);
    ImGui_ImplOpenGL3_Init();

    // file picker
    m_nfdInitialized = NFD_Init();
    if (m_nfdInitialized != NFD_OKAY) {
        std::cerr << "NFD failed to initialize, filepicker wont' work\n";
        std::cerr << std::format("Error: {}\n", NFD_GetError());
    }

    m_tools = std::make_unique<Tools>(m_renderer, m_pickingStyle, &m_picking);
}

Application::~Application() {
    if (m_nfdInitialized == NFD_OKAY) {
        NFD_Quit();
    }
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(m_imguiContext);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void Application::mainLoop() {
    m_running = true;
    int selected_actor = 0;
    bool pickingState = false;
    while (m_running) {
        m_renWin->Render();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) m_running = false;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(m_window))
                m_running = false;
            m_renWin->PushContext();
            m_running = !m_iren->ProcessEvent(&event);
            m_renWin->PopContext();
        }

        if (m_picking != pickingState) {
            if (m_picking) {
                m_iren->SetInteractorStyle(m_pickingStyle);
                SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR));
            } else {
                SDL_SetCursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW));
                // reset the state of the picker before switching
                m_pickingStyle->OnLeftButtonUp();
                m_iren->SetInteractorStyle(m_defaultStyle);
            }
            pickingState = m_picking;
        }

        SDL_GL_MakeCurrent(m_window, m_imguiContext);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        mainWindow();
        m_tools->showWindows();

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(m_window);

        m_tools->cleanup();
    }  // render loop
}

void Application::mainWindow() {
    {
        static float f = 0.0f;

        ImGui::Begin("Geometry Project", nullptr, ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */
                    auto path = pickModelFile();
                    if (path.has_value() && path->has_extension()) {
                        if (path->extension() == ".obj") {
                            openObjectFile(*path, m_renderer);
                        } else if (path->extension() == ".ply") {
                            openPLYFile(*path, m_renderer);
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
                    m_running = false;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Tools")) {
                if (ImGui::MenuItem("Actors")) {
                    m_tools->enableActorListWindow();
                }
                if (ImGui::MenuItem("Visualization")) {
                    m_tools->enableFunctionWindow();
                }
                if (ImGui::MenuItem("Deformations")) {
                    m_tools->enableDeformWindow();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        if (ImGui::CollapsingHeader("View", ImGuiTreeNodeFlags_DefaultOpen)) {
            double *color = m_renderer->GetBackground();
            float colorf[] = {float(color[0]), float(color[1]), float(color[2]), 1.0f};
            if (ImGui::ColorEdit3("Background", colorf)) {
                m_renderer->SetBackground(colorf[0], colorf[1], colorf[2]);
            }

            if (!m_picking) {
                auto style = dynamic_cast<vtkInteractorStyleSwitch *>(m_iren->GetInteractorStyle());
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
                ImGui::Text("Camera Parameters");
            }
            auto camera = m_renderer->GetActiveCamera();
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

        //spacer
        ImGui::Dummy(ImVec2(0, 20));
        auto size = ImGui::GetIO().DisplaySize;
        ImGui::Text("%.3f ms/frame (%.1f FPS) %.1f x %.1f", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate, size.x, size.y);
        ImGui::End();
    }
}