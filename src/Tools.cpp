#include "Tools.hpp"

#include <imgui.h>
#include <vtkActor.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkType.h>
#include <vtkUnsignedCharArray.h>

#include <format>
#include <iostream>

#include "harmonicFn.hpp"

Tools::Tools(vtkRenderer* renderer, MouseInteractorStylePP* picker, bool* picking)
    : m_renderer(renderer), m_picker(picker), m_picking(picking) {}

void Tools::showWindows() {
    if (m_showActorsWindow) actorListWindow();
    if (m_showFnWindow) functionsWindow();
}

void Tools::enableActorListWindow() { m_showActorsWindow = true; }
void Tools::enableFunctionWindow() { m_showFnWindow = true; }
void Tools::cleanup() {
    if (m_toRemove != nullptr) {
        m_renderer->RemoveActor(m_toRemove);
        m_toRemove = nullptr;
    }
}

void Tools::actorListWindow() {
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("vtk Actors", &m_showActorsWindow)) {
        {
            ImGui::BeginChild("Actor List", ImVec2(150, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
            auto actors = m_renderer->GetActors();
            int num_items = actors->GetNumberOfItems();

            for (int i = 0; i < num_items; ++i) {
                auto actor = dynamic_cast<vtkActor*>(actors->GetItemAsObject(i));
                const std::string label = std::format("{}: {}", i, actor->GetObjectName());
                if (ImGui::Selectable(label.c_str(), m_selectedActor == i)) {
                    m_selectedActor = i;
                }
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();

        {
            ImGui::BeginGroup();
            ImGui::BeginChild("item view",
                              ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));  // Leave room for 1 line below us
            auto actors = m_renderer->GetActors();
            if (actors->GetNumberOfItems() > 0) {
                auto actor = dynamic_cast<vtkActor*>(actors->GetItemAsObject(m_selectedActor));
                if (actor) {
                    ImGui::Text("Actor %d: %s", m_selectedActor, actor->GetObjectName().c_str());
                    ImGui::Separator();
                    ImGui::Text("Type: %s", actor->GetClassName());
                    bool edgeVisibility = actor->GetProperty()->GetEdgeVisibility();
                    if (ImGui::Checkbox("Edge Visibility", &edgeVisibility)) {
                        actor->GetProperty()->SetEdgeVisibility(edgeVisibility);
                    }
                    bool visible = actor->GetVisibility();
                    if (ImGui::Checkbox("Visible", &visible)) {
                        actor->SetVisibility(visible);
                    }
                    ImGui::Separator();
                    if (ImGui::Button("Remove")) {
                        m_toRemove = actor;
                    }
                }
            }
            ImGui::EndChild();
            ImGui::EndGroup();
        }
    }
}

void colorizeMesh(vtkPolyData* polyData, float* color) {
    if (polyData->GetNumberOfVerts() != polyData->GetNumberOfPoints()) {
        vtkNew<vtkCellArray> vertices;
        for (vtkIdType i = 0; i < polyData->GetNumberOfPoints(); ++i) {
            vertices->InsertNextCell(1);
            vertices->InsertCellPoint(i);
        }
        polyData->SetVerts(vertices);
    }
    vtkNew<vtkUnsignedCharArray> colors;
    colors->SetNumberOfComponents(3);
    colors->SetName("Colors");

    for (vtkIdType i = 0; i < polyData->GetNumberOfPoints(); ++i) {
        colors->InsertNextTuple3(color[0] * 255, color[1] * 255, color[2] * 255);
    }
    polyData->GetPointData()->SetScalars(colors);
}

void Tools::functionsWindow() {
    ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Harmonic functions visualization", &m_showFnWindow)) {
        if (!(*m_picking)) {
            if (ImGui::Button("Pick")) {
                m_picker->resetPickedState();
                *m_picking = true;
            }
        } else {
            ImGui::Text("Picking");
            ImGui::SameLine();
            if (ImGui::Button("Stop")) {
                *m_picking = false;
            }
        }
        if (m_picker->pickedSomething()) {
            *m_picking = false;
            auto actor = m_picker->getPickedActor();
            auto data = m_picker->getPickedData();
            auto pointId = m_picker->getPickedPointId();

            if (pointId && data) {
                double* point = (*data)->GetPoint(*pointId);
                ImGui::Text("Picked Point %lld : %lf %lf %lf", *pointId, point[0], point[1], point[2]);
            }
            if (actor) {
                ImGui::Text("Actor : %s", (*actor)->GetObjectName().c_str());
            }

            ImGui::Separator();
            ImGui::Text("Weight Function");
            std::array<const char*, 3> styles = {"Simple Harmonic", "Laplacian Diffusion", "solving Laplace Equations"};

            ImGui::Combo("Weighting Method", &m_weightingMethod, styles.begin(), styles.size());

            if (m_weightingMethod == 0 || m_weightingMethod == 2) {
                if (m_ringCount < 1) m_ringCount = 1;
                ImGui::InputInt("Ring Count", &m_ringCount);
            } else if (m_weightingMethod == 1) {
                if (m_ringCount < 0) m_ringCount = 0;
                ImGui::InputInt("Iterations", &m_ringCount);
                if (m_alpha < 0.0) m_alpha = 0.0;
                if (m_alpha >= 0.5) m_alpha = 0.49;
                ImGui::InputFloat("Alpha", &m_alpha);
            }

            ImGui::Separator();
            ImGui::Text("Color Transformation");

            ImGui::ColorEdit3("Default Color", m_colorNeutral);
            ImGui::ColorEdit3("Initial Color", m_colorStart);
            ImGui::ColorEdit3("End Color", m_colorEnd);

            if (actor && data && pointId) {
                auto polyData = *data;
                auto originActor = *actor;
                if (ImGui::Button("Apply")) {
                    colorizeMesh(polyData, m_colorNeutral);
                    std::function<double(vtkIdType)> harmonic = [](vtkIdType) -> double { return 0.0; };
                    if (m_weightingMethod == 0) {
                        harmonic = simpleHarmonic(polyData, *pointId, m_ringCount);
                    } else if (m_weightingMethod == 1) {
                        harmonic = laplacianDiffusion(polyData, *pointId, m_alpha, m_ringCount);
                    } else {
                        harmonic = solveLaplace(polyData, *pointId, m_ringCount);
                    }

                    double max = harmonic(*pointId);
                    for (vtkIdType ptid = 0; ptid < polyData->GetNumberOfPoints(); ++ptid) {
                        double weight = harmonic(ptid);
                        double normalized = weight / max;
                        if (weight > 0.0) {
                            double r = 255.0 * (normalized * m_colorStart[0] + (1.0 - normalized) * m_colorEnd[0]);
                            double g = 255.0 * (normalized * m_colorStart[1] + (1.0 - normalized) * m_colorEnd[1]);
                            double b = 255.0 * (normalized * m_colorStart[2] + (1.0 - normalized) * m_colorEnd[2]);
                            polyData->GetPointData()->GetScalars()->SetTuple3(ptid, r, g, b);
                        }
                    }
                }
            }
        }
    }
}
