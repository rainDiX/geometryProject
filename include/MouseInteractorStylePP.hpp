#pragma once

#include <vtkActor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPolyData.h>
#include <vtkType.h>
#include <optional>

class MouseInteractorStylePP : public vtkInteractorStyleTrackballCamera {
   public:
    static MouseInteractorStylePP* New();
    vtkTypeMacro(MouseInteractorStylePP, vtkInteractorStyleTrackballCamera);

    virtual void OnLeftButtonDown() override;

    std::optional<vtkActor*> getPickedActor() const;
    std::optional<vtkPolyData*> getPickedData() const;
    std::optional<vtkIdType> getPickedPointId() const;

    bool pickedSomething() const;

    void resetPickedState();

   private:
    bool m_pickedSomething = false;
    vtkIdType m_pickedPointId = -1;
    vtkActor* m_pickedActor = nullptr;
    vtkPolyData* m_pickedData = nullptr;
};
