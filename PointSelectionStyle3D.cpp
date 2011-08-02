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

#include "PointSelectionStyle3D.h"

#include <vtkAbstractPicker.h>
#include <vtkFollower.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkVectorText.h>

#include <sstream>

vtkStandardNewMacro(PointSelectionStyle3D);
 
void PointSelectionStyle3D::OnLeftButtonDown() 
{
  //std::cout << "Picking pixel: " << this->Interactor->GetEventPosition()[0] << " " << this->Interactor->GetEventPosition()[1] << std::endl;
  this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0], 
		      this->Interactor->GetEventPosition()[1], 
		      0,  // always zero.
		      this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
  double picked[3];
  this->Interactor->GetPicker()->GetPickPosition(picked);
  //std::cout << "Picked point with coordinate: " << picked[0] << " " << picked[1] << " " << picked[2] << std::endl;
  
  std::stringstream ss;
  ss << Numbers.size();
  vtkSmartPointer<vtkVectorText> textSource = 
    vtkSmartPointer<vtkVectorText>::New();
  textSource->SetText( ss.str().c_str() );
   
  // Create a mapper
  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection( textSource->GetOutputPort() );
 
  // Create a subclass of vtkActor: a vtkFollower that remains facing the camera
  vtkSmartPointer<vtkFollower> follower = vtkSmartPointer<vtkFollower>::New();
  follower->SetMapper( mapper );
  follower->SetPosition(picked);
  follower->GetProperty()->SetColor( 1, 0, 0 ); // red
  follower->SetScale( .1, .1, .1 );
 
  this->Numbers.push_back(follower);
  this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor( follower );
  
  // Forward events
  vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}
