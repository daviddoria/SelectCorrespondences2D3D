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
#include "itkDeformationFieldSource.h"
#include "itkDeformationFieldTransform.h"

// Qt
#include <QFileDialog>
#include <QIcon>

// VTK
#include <vtkActor.h>
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
#include <vtkSeedRepresentation.h>
#include <vtkSeedWidget.h>
#include <vtkSmartPointer.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkXMLPolyDataReader.h>

// Custom
#include "Helpers.h"
#include "Types.h"
#include "PointSelectionStyle.h"

// Constructor
Form::Form()
{
  this->setupUi(this);

  this->LeftRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->RightRenderer = vtkSmartPointer<vtkRenderer>::New();

  std::cout << "Left renderer: " << this->LeftRenderer << std::endl;
  std::cout << "Right renderer: " << this->RightRenderer << std::endl;
  
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
  QIcon openIcon = QIcon::fromTheme("document-open");
  actionOpenImage->setIcon(openIcon);
  this->toolBar->addAction(actionOpenImage);
  
  actionOpenPointCloud->setIcon(openIcon);
  this->toolBar->addAction(actionOpenPointCloud);
  
  QIcon saveIcon = QIcon::fromTheme("document-save");
  actionSaveImagePoints->setIcon(saveIcon);
  this->toolBar->addAction(actionSaveImagePoints);
    
  actionSavePointCloudPoints->setIcon(saveIcon);
  this->toolBar->addAction(actionSavePointCloudPoints);
  
  this->HandleRepresentation2D = vtkSmartPointer<vtkPointHandleRepresentation2D>::New();
  this->HandleRepresentation2D->GetProperty()->SetColor(1,0,0);
  this->SeedRepresentation2D = vtkSmartPointer<vtkSeedRepresentation>::New();
  this->SeedRepresentation2D->SetHandleRepresentation(this->HandleRepresentation2D);
  
  this->HandleRepresentation3D = vtkSmartPointer<vtkPointHandleRepresentation2D>::New();
  this->HandleRepresentation3D->GetProperty()->SetColor(1,0,0);
  this->SeedRepresentation3D = vtkSmartPointer<vtkSeedRepresentation>::New();
  this->SeedRepresentation3D->SetHandleRepresentation(this->HandleRepresentation3D);
};

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

  vtkSmartPointer<vtkInteractorStyleImage> imageInteractorStyle =
      vtkSmartPointer<vtkInteractorStyleImage>::New();
  this->qvtkWidgetLeft->GetRenderWindow()->GetInteractor()->SetInteractorStyle(imageInteractorStyle);

  this->LeftRenderer->ResetCamera();

  
  // Seed widget
  this->SeedWidget2D = vtkSmartPointer<vtkSeedWidget>::New();
  this->SeedWidget2D->SetInteractor(this->qvtkWidgetRight->GetRenderWindow()->GetInteractor());
  this->SeedWidget2D->SetRepresentation(this->SeedRepresentation2D);
  
  this->SeedCallback2D = vtkSmartPointer<vtkSeedCallback>::New();
  this->SeedCallback2D->SetWidget(this->SeedWidget2D);
  
  this->SeedWidget2D->AddObserver(vtkCommand::PlacePointEvent,this->SeedCallback2D);
  this->SeedWidget2D->AddObserver(vtkCommand::InteractionEvent,this->SeedCallback2D);
  this->SeedWidget2D->On();
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
  
  // Add Actor to renderer
  this->RightRenderer->AddActor(this->PointCloudActor);
  this->RightRenderer->ResetCamera();

  vtkSmartPointer<vtkPointPicker> pointPicker = 
    vtkSmartPointer<vtkPointPicker>::New();
  this->qvtkWidgetRight->GetRenderWindow()->GetInteractor()->SetPicker(pointPicker);
  vtkSmartPointer<PointSelectionStyle> pointSelectionStyle =
      vtkSmartPointer<PointSelectionStyle>::New();
  this->qvtkWidgetRight->GetRenderWindow()->GetInteractor()->SetInteractorStyle(pointSelectionStyle);

  
  this->RightRenderer->ResetCamera();

  //std::cout << "Fixed interactor: " << this->qvtkWidgetLeft->GetRenderWindow()->GetInteractor() << std::endl;
  // Seed widget
  this->SeedWidget3D = vtkSmartPointer<vtkSeedWidget>::New();
  this->SeedWidget3D->SetInteractor(this->qvtkWidgetLeft->GetRenderWindow()->GetInteractor());
  this->SeedWidget3D->SetRepresentation(this->SeedRepresentation3D);
  
  this->SeedCallback3D = vtkSmartPointer<vtkSeedCallback>::New();
  this->SeedCallback3D->SetWidget(this->SeedWidget3D);
  
  this->SeedWidget3D->AddObserver(vtkCommand::PlacePointEvent,this->SeedCallback3D);
  this->SeedWidget3D->AddObserver(vtkCommand::InteractionEvent,this->SeedCallback3D);
  this->SeedWidget3D->On();
}

void Form::on_actionSaveImagePoints_activated()
{
  if(this->SeedRepresentation2D->GetNumberOfSeeds() !=
     this->SeedRepresentation3D->GetNumberOfSeeds())
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
 
  for(vtkIdType i = 0; i < this->SeedRepresentation2D->GetNumberOfSeeds(); i++)
    {
    double p[3];
    this->SeedRepresentation2D->GetSeedDisplayPosition(i, p);
  
    fout << p[0] << " " << p[1] << std::endl;
 
    }
  fout.close();
}

void Form::on_actionSavePointCloudPoints_activated()
{
  if(this->SeedRepresentation2D->GetNumberOfSeeds() !=
     this->SeedRepresentation3D->GetNumberOfSeeds())
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
 
  for(vtkIdType i = 0; i < this->SeedRepresentation3D->GetNumberOfSeeds(); i++)
    {
    double p[3];
    this->SeedRepresentation3D->GetSeedDisplayPosition(i, p);
  
    fout << p[0] << " " << p[1] << " " << p[2] << std::endl;
 
    }
  fout.close();
}
