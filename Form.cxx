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

#include "ui_Form.h"
#include "Form.h"

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkVector.h"

// Qt
#include <QFileDialog>
#include <QIcon>

// VTK
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleImage.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPointPicker.h>
#include <vtkProperty2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkXMLPolyDataReader.h>

// Custom
#include "Helpers.h"
#include "Types.h"


// Constructor
Form::Form()
{
  this->setupUi(this);

  this->LeftRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->RightRenderer = vtkSmartPointer<vtkRenderer>::New();

  this->qvtkWidgetLeft->GetRenderWindow()->AddRenderer(this->LeftRenderer);
  this->qvtkWidgetRight->GetRenderWindow()->AddRenderer(this->RightRenderer);

  // Setup image
  this->ImageActor = vtkSmartPointer<vtkImageActor>::New();
  this->ImageData = vtkSmartPointer<vtkImageData>::New();

  // Setup point cloud
  this->PointCloudActor = vtkSmartPointer<vtkActor>::New();
  this->PointCloudMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->PointCloud = vtkSmartPointer<vtkPolyData>::New();

  // Setup toolbar
  // Open file buttons
  QIcon openIcon = QIcon::fromTheme("document-open");
  actionOpenImage->setIcon(openIcon);
  this->toolBar->addAction(actionOpenImage);

  actionOpenPointCloud->setIcon(openIcon);
  this->toolBar->addAction(actionOpenPointCloud);

  // Save buttons
  QIcon saveIcon = QIcon::fromTheme("document-save");
  actionSaveImagePoints->setIcon(saveIcon);
  this->toolBar->addAction(actionSaveImagePoints);

  actionSavePointCloudPoints->setIcon(saveIcon);
  this->toolBar->addAction(actionSavePointCloudPoints);

  // Open points buttons
  actionLoad2DPoints->setIcon(openIcon);
  this->toolBar->addAction(actionLoad2DPoints);

  actionLoad3DPoints->setIcon(openIcon);
  this->toolBar->addAction(actionLoad3DPoints);
};


void Form::on_actionLoad2DPoints_activated()
{
   // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Text Files (*.txt)");

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  std::string line;
  std::ifstream fin(fileName.toStdString().c_str());

  if(fin == NULL)
  {
    std::cout << "Cannot open file." << std::endl;
  }

  pointSelectionStyle2D->RemoveAllPoints();
  
  while(getline(fin, line))
    {
    std::stringstream ss;
    ss << line;
    double p[3];
    ss >> p[0] >> p[1];
    p[2] = 0;
  
    pointSelectionStyle2D->AddNumber(p);
    }
}

void Form::on_actionLoad3DPoints_activated()
{
   // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Text Files (*.txt)");

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  std::string line;
  std::ifstream fin(fileName.toStdString().c_str());

  if(fin == NULL)
  {
    std::cout << "Cannot open file." << std::endl;
  }

  pointSelectionStyle3D->RemoveAllPoints();

  while(getline(fin, line))
    {
    std::stringstream ss;
    ss << line;
    double p[3];
    ss >> p[0] >> p[1] >> p[2];
    pointSelectionStyle3D->AddNumber(p);
    }
}

void Form::on_actionOpenImage_activated()
{
   // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Image Files (*.png *.mhd *.tif)");

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  typedef itk::ImageFileReader<FloatVectorImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName.toStdString());
  reader->Update();
  
  this->Image = reader->GetOutput();

  if(this->chkRGB->isChecked())
    {
    Helpers::ITKImagetoVTKRGBImage(this->Image, this->ImageData);
    }
  else
    {
    Helpers::ITKImagetoVTKMagnitudeImage(this->Image, this->ImageData);
    }
  
  this->ImageActor->SetInput(this->ImageData);

  // Add Actor to renderer
  this->LeftRenderer->AddActor(this->ImageActor);
  this->LeftRenderer->ResetCamera();

  vtkSmartPointer<vtkPointPicker> pointPicker = vtkSmartPointer<vtkPointPicker>::New();
  this->qvtkWidgetLeft->GetRenderWindow()->GetInteractor()->SetPicker(pointPicker);
  this->pointSelectionStyle2D = vtkSmartPointer<PointSelectionStyle2D>::New();
  this->qvtkWidgetLeft->GetRenderWindow()->GetInteractor()->SetInteractorStyle(pointSelectionStyle2D);

  this->LeftRenderer->ResetCamera();

  // Flip the image by changing the camera view up because of the conflicting conventions used by ITK and VTK
  //this->LeftRenderer = vtkSmartPointer<vtkRenderer>::New();
  //this->LeftRenderer->GradientBackgroundOn();
  //this->LeftRenderer->SetBackground(this->BackgroundColor);
  //this->LeftRenderer->SetBackground2(1,1,1);

  double cameraUp[3];
  cameraUp[0] = 0;
  cameraUp[1] = -1;
  cameraUp[2] = 0;
  this->LeftRenderer->GetActiveCamera()->SetViewUp(cameraUp);

  double cameraPosition[3];
  this->LeftRenderer->GetActiveCamera()->GetPosition(cameraPosition);
  std::cout << cameraPosition[0] << " " << cameraPosition[1] << " " << cameraPosition[2] << std::endl;
  //cameraPosition[0] *= -1;
  //cameraPosition[1] *= -1;
  cameraPosition[2] *= -1;
  this->LeftRenderer->GetActiveCamera()->SetPosition(cameraPosition);

  // Verify
  this->LeftRenderer->GetActiveCamera()->GetPosition(cameraPosition);
  std::cout << cameraPosition[0] << " " << cameraPosition[1] << " " << cameraPosition[2] << std::endl;
  
  this->qvtkWidgetLeft->GetRenderWindow()->Render();
}

void Form::on_actionOpenPointCloud_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Image Files (*.vtp)");

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
  reader->SetFileName(fileName.toStdString().c_str());
  reader->Update();
  
  this->PointCloudMapper->SetInputConnection(reader->GetOutputPort());
  this->PointCloudActor->SetMapper(this->PointCloudMapper);
  this->PointCloudActor->GetProperty()->SetRepresentationToPoints();
  
  // Add Actor to renderer
  this->RightRenderer->AddActor(this->PointCloudActor);
  this->RightRenderer->ResetCamera();

  vtkSmartPointer<vtkPointPicker> pointPicker = 
    vtkSmartPointer<vtkPointPicker>::New();
  this->qvtkWidgetRight->GetRenderWindow()->GetInteractor()->SetPicker(pointPicker);
  this->pointSelectionStyle3D = vtkSmartPointer<PointSelectionStyle3D>::New();
  this->qvtkWidgetRight->GetRenderWindow()->GetInteractor()->SetInteractorStyle(pointSelectionStyle3D);

  
  this->RightRenderer->ResetCamera();

}

void Form::on_actionSaveImagePoints_activated()
{
  if(this->pointSelectionStyle2D->Numbers.size() !=
     this->pointSelectionStyle3D->Numbers.size())
  {
    std::cerr << "The number of fixed seeds must match the number of moving seeds!" << std::endl;
    return;
  }
  
  QString fileName = QFileDialog::getSaveFileName(this, "Save File", ".", "Text Files (*.txt)");
  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  std::ofstream fout(fileName.toStdString().c_str());
 
  for(unsigned int i = 0; i < this->pointSelectionStyle2D->Numbers.size(); i++)
    {
    double p[3];
    this->pointSelectionStyle2D->Numbers[i]->GetPosition(p);
  
    fout << p[0] << " " << p[1] << std::endl;
 
    }
  fout.close();
}

void Form::on_actionSavePointCloudPoints_activated()
{
  if(this->pointSelectionStyle2D->Numbers.size() !=
     this->pointSelectionStyle3D->Numbers.size())
  {
    std::cerr << "The number of fixed seeds must match the number of moving seeds!" << std::endl;
    return;
  }
  
  QString fileName = QFileDialog::getSaveFileName(this, "Save File", ".", "Text Files (*.txt)");
  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }
    
  std::ofstream fout(fileName.toStdString().c_str());
 
  for(unsigned int i = 0; i < this->pointSelectionStyle3D->Numbers.size(); i++)
    {
    double p[3];
    this->pointSelectionStyle3D->Numbers[i]->GetPosition(p);
  
    fout << p[0] << " " << p[1] << " " << p[2] << std::endl;
 
    }
  fout.close();
}
