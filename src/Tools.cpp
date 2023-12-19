#include "Tools.hpp"

#include <imgui.h>
#include <vtkActor.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>

#include <format>

void actorListWindow(vtkRenderer* renderer, bool& open, int& selected) {
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("vtk Actors", &open)) {
        {
            ImGui::BeginChild("Actor List", ImVec2(150, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
            auto actors = renderer->GetActors();
            int num_items = actors->GetNumberOfItems();

            for (int i = 0; i < num_items; ++i) {
                auto actor = dynamic_cast<vtkActor*>(actors->GetItemAsObject(i));
                const std::string label = std::format("{}: {}", i, actor->GetObjectName());
                if (ImGui::Selectable(label.c_str(), selected == i)) {
                    selected = i;
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
                ImGui::Text("Actor %d: %s", selected, actor->GetObjectName().c_str());
                ImGui::Separator();
                ImGui::Text("Type: %s", actor->GetClassName());
                bool edgeVisibility = actor->GetProperty()->GetEdgeVisibility();
                if (ImGui::Checkbox("Edge Visibility", &edgeVisibility)) {
                    actor->GetProperty()->SetEdgeVisibility(edgeVisibility);
                }
            }
            ImGui::EndChild();
            ImGui::EndGroup();
        }
    }
}

void functionsWindow(vtkRenderer* renderer, MouseInteractorStylePP* picker, bool& open, bool& picking) {
    ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Function XXX", &open)) {
        if (!picking) {
            if (ImGui::Button("Pick")) {
                picker->resetPickedState();
                picking = true;
            }
        } else {
            ImGui::Text("Picking");
            ImGui::SameLine();
            if (ImGui::Button("Stop")) {
                picking = false;
            }
        }
        if (picker->pickedSomething()) {
            picking = false;
            auto actor = picker->getPickedActor();
            auto data = picker->getPickedData();
            auto pointId = picker->getPickedPointId();

            if (pointId && data) {
                double* point = (*data)->GetPoint(*pointId);
                ImGui::Text("Picked Point : %lf %lf %lf", point[0], point[1], point[2]);
            }
            if (actor) {
                ImGui::Text("Actor : %s", (*actor)->GetObjectName().c_str());
            }
        }
    }
}
