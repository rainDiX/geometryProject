#include "Tools.hpp"

#include <vtkActor.h>
#include <vtkProperty.h>

#include "imgui.h"

void actorListWindow(vtkRenderer* renderer, bool& open) {
    static int selected = 0;
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("vtk Actors", &open)) {
        {
            ImGui::BeginChild("Actor List", ImVec2(150, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
            auto actors = renderer->GetActors();
            int num_items = actors->GetNumberOfItems();

            for (int i = 0; i < num_items; ++i) {
                auto actor = dynamic_cast<vtkActor*>(actors->GetItemAsObject(i));
                if (ImGui::Selectable(actor->GetClassName(), selected == i)) {
                    selected = i;
                    std::cerr << i << '\n';
                }
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();

        {
            ImGui::BeginGroup();
            ImGui::BeginChild("item view",
                              ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));  // Leave room for 1 line below us
            auto actors = renderer->GetActors();
            auto actor = dynamic_cast<vtkActor*>(actors->GetItemAsObject(selected));
            if (actor) {
                ImGui::Text("Actor: %d", selected);
                ImGui::Separator();
                bool edgeVisibility = actor->GetProperty()->GetEdgeVisibility();
                if(ImGui::Checkbox("Edge Visibility", &edgeVisibility)) {
                    actor->GetProperty()->SetEdgeVisibility(edgeVisibility);
                }
            }
            ImGui::EndChild();
            ImGui::EndGroup();
        }
    }
}

void actorManagerWindow(vtkRenderer* renderer, vtkActor* actor, bool& open) {}
