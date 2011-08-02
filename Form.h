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

#ifndef FORM_H
#define FORM_H

#include "ui_Form.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkSeedWidget.h>
#include <vtkPointHandleRepresentation2D.h>

// ITK
#include "itkImage.h"

// Qt
#include <QMainWindow>

// Custom
#include "Types.h"
#include "SeedCallback.h"
#include "PointSelectionStyle2D.h"
#include "PointSelectionStyle3D.h"

// Forward declarations
class vtkActor;
class vtkBorderWidget;
class vtkImageData;
class vtkImageActor;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkRenderer;

class Form : public QMainWindow, public Ui::Form
{
  Q_OBJECT
public:

  // Constructor/Destructor
  Form();
  ~Form() {};

public slots:
  void on_actionOpenImage_activated();
  void on_actionOpenPointCloud_activated();
  void on_actionSaveImagePoints_activated();
  void on_actionSavePointCloudPoints_activated();

protected:

  vtkSmartPointer<vtkRenderer> LeftRenderer;
  vtkSmartPointer<vtkRenderer> RightRenderer;
  
  // Image
  FloatVectorImageType::Pointer Image;
  vtkSmartPointer<vtkImageActor> ImageActor;
  vtkSmartPointer<vtkImageData> ImageData;
  
  // Point cloud
  vtkSmartPointer<vtkActor> PointCloudActor;
  vtkSmartPointer<vtkPolyDataMapper> PointCloudMapper;
  vtkSmartPointer<vtkPolyData> PointCloud;
  
  vtkSmartPointer<PointSelectionStyle2D> pointSelectionStyle2D;
  vtkSmartPointer<PointSelectionStyle3D> pointSelectionStyle3D;
};

#endif // Form_H
