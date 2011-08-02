/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "PointSelectionStyle.h"

#include <vtkAbstractPicker.h>
#include <vtkObjectFactory.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

vtkStandardNewMacro(PointSelectionStyle);
 
void PointSelectionStyle::OnLeftButtonDown() 
{
  std::cout << "Picking pixel: " << this->Interactor->GetEventPosition()[0] << " " << this->Interactor->GetEventPosition()[1] << std::endl;
  this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0], 
		      this->Interactor->GetEventPosition()[1], 
		      0,  // always zero.
		      this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
  double picked[3];
  this->Interactor->GetPicker()->GetPickPosition(picked);
  std::cout << "Picked value: " << picked[0] << " " << picked[1] << " " << picked[2] << std::endl;
  // forward events
  vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}
