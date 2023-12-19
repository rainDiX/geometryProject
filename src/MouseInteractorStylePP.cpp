#include "MouseInteractorStylePP.hpp"

#include <vtkPointPicker.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkType.h>

vtkStandardNewMacro(MouseInteractorStylePP);

void MouseInteractorStylePP::OnLeftButtonDown() {
    m_pickedPointId = -1;
    m_pickedActor = nullptr;
    m_pickedData = nullptr;
    this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0],
                                        this->Interactor->GetEventPosition()[1],
                                        0,  // always zero.
                                        this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
    double picked[3];
    this->Interactor->GetPicker()->GetPickPosition(picked);
    auto picker = reinterpret_cast<vtkPointPicker *>(this->Interactor->GetPicker());
    m_pickedPointId = picker->GetPointId();
    if (m_pickedPointId != -1) {
        m_pickedSomething = true;
        m_pickedActor = picker->GetActor();
        m_pickedData = static_cast<vtkPolyData *>(picker->GetDataSet());
    } else {
        m_pickedSomething = false;
    }
    // Forward events.
    vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}

std::optional<vtkActor *> MouseInteractorStylePP::getPickedActor() const {
    if (m_pickedActor != nullptr) return m_pickedActor;
    return {};
}
std::optional<vtkPolyData *> MouseInteractorStylePP::getPickedData() const {
    if (m_pickedData != nullptr) return m_pickedData;
    return {};
}

std::optional<vtkIdType> MouseInteractorStylePP::getPickedPointId() const {
    if (m_pickedPointId != -1) return m_pickedPointId;
    return {};
}

bool MouseInteractorStylePP::pickedSomething() const { return m_pickedSomething; }

void MouseInteractorStylePP::resetPickedState() { m_pickedSomething = false; }