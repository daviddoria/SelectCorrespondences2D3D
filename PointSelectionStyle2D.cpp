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

#include "PointSelectionStyle2D.h"

#include <vtkAbstractPicker.h>
#include <vtkActor2D.h>
#include <vtkCoordinate.h>
#include <vtkFollower.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkVectorText.h>

#include <sstream>

vtkStandardNewMacro(PointSelectionStyle2D);
 
void PointSelectionStyle2D::OnLeftButtonDown() 
{
  //std::cout << "Picking pixel: " << this->Interactor->GetEventPosition()[0] << " " << this->Interactor->GetEventPosition()[1] << std::endl;
  this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0], 
		      this->Interactor->GetEventPosition()[1], 
		      0,  // always zero.
		      this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
  double picked[3];
  this->Interactor->GetPicker()->GetPickPosition(picked);
  //std::cout << "Picked point with coordinate: " << picked[0] << " " << picked[1] << " " << picked[2] << std::endl;

  AddNumber(picked);
 
  // Forward events
  vtkInteractorStyleImage::OnLeftButtonDown();
}

void PointSelectionStyle2D::RefreshNumbers()
{
  // Determine a reasonable scale
  vtkSmartPointer<vtkCoordinate> coordinate1 = vtkSmartPointer<vtkCoordinate>::New();
  coordinate1->SetCoordinateSystemToNormalizedDisplay();
  // We want the number to take up 10% of the window
  coordinate1->SetValue(.1,.1,.1);
  double* c1 = coordinate1->GetComputedWorldValue(this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

  vtkSmartPointer<vtkCoordinate> coordinate0 = vtkSmartPointer<vtkCoordinate>::New();
  coordinate0->SetCoordinateSystemToNormalizedDisplay();
  // We want the number to take up 10% of the window
  coordinate0->SetValue(0,0,0);
  double* c0 = coordinate0->GetComputedWorldValue(this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

  double scale = fabs(c1[0]-c0[0]);
  std::cout << "Scale: " << scale << std::endl;
  // The numbers get smaller as we zoom in, because a smaller piece of the world takes up 10% of the screen
  //scale = 1./scale;

  for(unsigned int i = 0; i < Numbers.size(); ++i)
    {
    //Numbers[i]->SetScale(scale, scale, 1.);
    }

  this->Interactor->GetRenderWindow()->Render();
}

void PointSelectionStyle2D::OnRightButtonUp()
{
  RefreshNumbers();
  
  // Forward events
  vtkInteractorStyleImage::OnRightButtonUp();
}

void PointSelectionStyle2D::RemoveAllPoints()
{
  for(unsigned int i = 0; i < Coordinates.size(); ++i)
    {
    this->CurrentRenderer->RemoveViewProp( Numbers[i]);
    this->CurrentRenderer->RemoveViewProp( Points[i]);
    }
  Numbers.clear();
  Points.clear();
  Coordinates.clear();
}


void PointSelectionStyle2D::AddNumber(double p[3])
{
  Coord2D coord;
  coord.x = p[0];
  coord.y = p[1];
  Coordinates.push_back(coord);
  
  p[0] = static_cast<int>( p[0] + 0.5 );
  p[1] = static_cast<int>( p[1] + 0.5 );
  p[2] = 0;
  std::cout << "Adding marker at " << p[0] << " " << p[1] << " " << p[2] << std::endl;
  // Convert the current number to a string
  std::stringstream ss;
  ss << Coordinates.size();

  // Create the number
  // Create the text
  vtkSmartPointer<vtkVectorText> textSource = vtkSmartPointer<vtkVectorText>::New();
  textSource->SetText( ss.str().c_str() );
/*
  // Create a mapper
  vtkSmartPointer<vtkPolyDataMapper> textMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  textMapper->SetInputConnection( textSource->GetOutputPort() );

  // Create a subclass of vtkActor: a vtkFollower that remains facing the camera
  vtkSmartPointer<vtkFollower> textFollower = vtkSmartPointer<vtkFollower>::New();
  textFollower->SetMapper( textMapper );
  textFollower->SetCamera(this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->GetActiveCamera());

  p[2] = -10; // so it is a little bit in front of the image (the sign should be set based on the position of the camera)
  textFollower->SetPosition(p);
  textFollower->GetProperty()->SetColor( 1, 0, 0 ); // red

  this->Numbers.push_back(textFollower);
  this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor( textFollower );
  */


  //get the bounds of the text
  textSource->Update();
  double* bounds = textSource->GetOutput()->GetBounds();
  //transform the polydata to be centered over the pick position
  double center[3] = {0.5*(bounds[1]+bounds[0]), 0.5*(bounds[3]+bounds[2]), 0.0 };

  vtkSmartPointer<vtkTransform> trans = vtkSmartPointer<vtkTransform>::New();
  trans->Translate( -center[0], -center[1], 0 );
  trans->Translate( p[0], p[1], 0 );

  vtkSmartPointer<vtkTransformPolyDataFilter> tpd = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  tpd->SetTransform( trans );
  tpd->SetInputConnection(  textSource->GetOutputPort() );

  // Create a mapper
  vtkSmartPointer<vtkPolyDataMapper2D> mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  vtkSmartPointer<vtkCoordinate> coordinate = vtkSmartPointer<vtkCoordinate>::New();
  coordinate->SetCoordinateSystemToWorld();
  mapper->SetTransformCoordinate( coordinate );
  mapper->SetInputConnection( tpd->GetOutputPort() );

  vtkSmartPointer<vtkActor2D> actor = vtkSmartPointer<vtkActor2D>::New();
  actor->SetMapper( mapper );
  actor->GetProperty()->SetColor( 1, 1, 0 ); // yellow
  this->CurrentRenderer->AddViewProp( actor );

  // Create the dot
  // Create a sphere
  vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
  sphereSource->SetRadius(.5);
  sphereSource->SetCenter(p);
  sphereSource->Update();

  // Create a mapper
  vtkSmartPointer<vtkPolyDataMapper> sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  sphereMapper->SetInputConnection( sphereSource->GetOutputPort() );

  // Create a subclass of vtkActor: a vtkFollower that remains facing the camera
  vtkSmartPointer<vtkActor> sphereActor = vtkSmartPointer<vtkActor>::New();
  sphereActor->SetMapper( sphereMapper );
  sphereActor->GetProperty()->SetColor( 1, 0, 0 ); // red

  this->Points.push_back(sphereActor);
  this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor( sphereActor );

  RefreshNumbers();
}
