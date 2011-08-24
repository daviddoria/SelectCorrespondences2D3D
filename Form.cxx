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
#include <QTextEdit>

// VTK
#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkFloatArray.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkInteractorStyleImage.h>
#include <vtkLookupTable.h>
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

void Form::on_actionHelp_activated()
{
  QTextEdit* help=new QTextEdit();
  
  help->setReadOnly(true);
  help->append("<h1>Image keypoints</h1>\
  Hold the right mouse button and drag to zoom in and out. <br/>\
  Hold the middle mouse button and drag to pan the image. <br/>\
  Click the left mouse button to select a keypoint.<br/> <p/>\
  <h1>Point cloud keypoints</h1>\
  Hold the left mouse button and drag to rotate the scene.<br/>\
  Hold the right mouse button and drag to zoom in and out. Hold the middle mouse button and drag to pan the scene. While holding control (CTRL), click the left mouse button to select a keypoint.<br/>\
  If you need to zoom in farther, hold shift while left clicking a point to change the camera's focal point to that point. You can reset the focal point by pressing 'r'.\
  <h1>Saving keypoints</h1>\
  The same number of keypoints must be selected in both the image and the point cloud before the points can be saved."
  );
  help->show();
}

void Form::on_actionQuit_activated()
{
  exit(0);
}

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
  this->toolBar_image->addAction(actionOpenImage);

  actionOpenPointCloud->setIcon(openIcon);
  this->toolBar_pointcloud->addAction(actionOpenPointCloud);

  // Save buttons
  QIcon saveIcon = QIcon::fromTheme("document-save");
  actionSaveImagePoints->setIcon(saveIcon);
  this->toolBar_image->addAction(actionSaveImagePoints);

  actionSavePointCloudPoints->setIcon(saveIcon);
  this->toolBar_pointcloud->addAction(actionSavePointCloudPoints);

  // Open points buttons
  actionLoad2DPoints->setIcon(openIcon);
  this->toolBar_image->addAction(actionLoad2DPoints);

  actionLoad3DPoints->setIcon(openIcon);
  this->toolBar_pointcloud->addAction(actionLoad3DPoints);

  this->pointSelectionStyle2D = NULL;
  this->pointSelectionStyle3D = NULL;
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
  this->ImageActor->InterpolateOff();
  
  // Add Actor to renderer
  this->LeftRenderer->AddActor(this->ImageActor);
  this->LeftRenderer->ResetCamera();

  vtkSmartPointer<vtkPointPicker> pointPicker = vtkSmartPointer<vtkPointPicker>::New();
  this->qvtkWidgetLeft->GetRenderWindow()->GetInteractor()->SetPicker(pointPicker);
  this->pointSelectionStyle2D = vtkSmartPointer<PointSelectionStyle2D>::New();
  this->pointSelectionStyle2D->SetCurrentRenderer(this->LeftRenderer);
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
  //std::cout << cameraPosition[0] << " " << cameraPosition[1] << " " << cameraPosition[2] << std::endl;
  //cameraPosition[0] *= -1;
  //cameraPosition[1] *= -1;
  cameraPosition[2] *= -1;
  this->LeftRenderer->GetActiveCamera()->SetPosition(cameraPosition);

  // Verify
  this->LeftRenderer->GetActiveCamera()->GetPosition(cameraPosition);
  //std::cout << cameraPosition[0] << " " << cameraPosition[1] << " " << cameraPosition[2] << std::endl;
  
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
  reader->GetOutput()->GetPointData()->SetActiveScalars("Intensity");

  float range[2];
  vtkFloatArray::SafeDownCast(reader->GetOutput()->GetPointData()->GetArray("Intensity"))->GetValueRange(range);
  
  vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
  //lookupTable->SetTableRange(0.0, 10.0);
  lookupTable->SetTableRange(range[0], range[1]);
  //lookupTable->SetHueRange(0, .5);
  //lookupTable->SetHueRange(.5, 1);
  lookupTable->SetHueRange(0, 1);
  
  this->PointCloudMapper->SetInputConnection(reader->GetOutputPort());
  this->PointCloudMapper->SetLookupTable(lookupTable);
  this->PointCloudActor->SetMapper(this->PointCloudMapper);
  this->PointCloudActor->GetProperty()->SetRepresentationToPoints();
  
  // Add Actor to renderer
  this->RightRenderer->AddActor(this->PointCloudActor);
  this->RightRenderer->ResetCamera();

  vtkSmartPointer<vtkPointPicker> pointPicker = vtkSmartPointer<vtkPointPicker>::New();
  
  pointPicker->PickFromListOn();
  pointPicker->AddPickList(this->PointCloudActor);
  this->qvtkWidgetRight->GetRenderWindow()->GetInteractor()->SetPicker(pointPicker);
  this->pointSelectionStyle3D = vtkSmartPointer<PointSelectionStyle3D>::New();
  this->pointSelectionStyle3D->SetCurrentRenderer(this->RightRenderer);
  this->pointSelectionStyle3D->Data = reader->GetOutput();
  this->qvtkWidgetRight->GetRenderWindow()->GetInteractor()->SetInteractorStyle(pointSelectionStyle3D);
  
  this->RightRenderer->ResetCamera();

  float averageSpacing = Helpers::ComputeAverageSpacing(reader->GetOutput()->GetPoints());
  this->pointSelectionStyle3D->SetMarkerRadius(averageSpacing);
}

void Form::on_actionSaveImagePoints_activated()
{
  if(!this->pointSelectionStyle2D || !this->pointSelectionStyle3D)
    {
    std::cerr << "You must have loaded and selected points from both the image and the corresponding point cloud!" << std::endl;
    return;
    }
  
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
 
  for(unsigned int i = 0; i < this->pointSelectionStyle2D->Coordinates.size(); i++)
    {
    //double p[3];
    //this->pointSelectionStyle2D->Numbers[i]->GetPosition(p);
    //fout << p[0] << " " << p[1] << std::endl;
    fout << this->pointSelectionStyle2D->Coordinates[i].x << " " << this->pointSelectionStyle2D->Coordinates[i].y << std::endl;
 
    }
  fout.close();
}

void Form::on_actionSavePointCloudPoints_activated()
{
  if(!this->pointSelectionStyle2D || !this->pointSelectionStyle3D)
    {
    std::cerr << "You must have loaded and selected points from both the image and the corresponding point cloud!" << std::endl;
    return;
    }
    
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
    /*
    double p[3];
    this->pointSelectionStyle3D->Numbers[i]->GetPosition(p);
  
    fout << p[0] << " " << p[1] << " " << p[2] << std::endl;
    */
    fout << this->pointSelectionStyle3D->Coordinates[i].x << " "
         << this->pointSelectionStyle3D->Coordinates[i].y << " "
         << this->pointSelectionStyle3D->Coordinates[i].z << std::endl;
    }
  fout.close();
}

void Form::on_btnDeleteLastImageKeypoint_clicked()
{
  this->LeftRenderer->RemoveViewProp( this->pointSelectionStyle2D->Numbers[this->pointSelectionStyle2D->Numbers.size() - 1]);
  this->LeftRenderer->RemoveViewProp( this->pointSelectionStyle2D->Points[this->pointSelectionStyle2D->Points.size() - 1]);
  this->pointSelectionStyle2D->Numbers.erase(this->pointSelectionStyle2D->Numbers.end()-1);
  this->pointSelectionStyle2D->Points.erase(this->pointSelectionStyle2D->Points.end()-1);
  this->pointSelectionStyle2D->Coordinates.erase(this->pointSelectionStyle2D->Coordinates.end()-1);
  this->qvtkWidgetLeft->GetRenderWindow()->Render();
}

void Form::on_btnDeleteAllImageKeypoints_clicked()
{
  this->pointSelectionStyle2D->RemoveAllPoints();
  this->qvtkWidgetLeft->GetRenderWindow()->Render();
}

void Form::on_btnDeleteLastPointcloudKeypoint_clicked()
{
  this->RightRenderer->RemoveViewProp( this->pointSelectionStyle3D->Numbers[this->pointSelectionStyle3D->Numbers.size() - 1]);
  this->RightRenderer->RemoveViewProp( this->pointSelectionStyle3D->Points[this->pointSelectionStyle3D->Points.size() - 1]);
  this->pointSelectionStyle3D->Numbers.erase(this->pointSelectionStyle3D->Numbers.end()-1);
  this->pointSelectionStyle3D->Points.erase(this->pointSelectionStyle3D->Points.end()-1);
  this->pointSelectionStyle3D->Coordinates.erase(this->pointSelectionStyle3D->Coordinates.end()-1);
  this->qvtkWidgetRight->GetRenderWindow()->Render();
}

void Form::on_btnDeleteAllPointcloudKeypoints_clicked()
{
  this->pointSelectionStyle3D->RemoveAllPoints();
  this->qvtkWidgetRight->GetRenderWindow()->Render();
}
