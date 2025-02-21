#include <string.h>
#include <QQuickWindow>
#include <QOpenGLFramebufferObjectFormat>
#include <QDebug>
#include <vtkElevationFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkTextProperty.h>
#include <vtkCamera.h>
#include <vtkColor.h>
#include <vtkStringArray.h>
#include <vtkVectorText.h>
#include <vtkAxes.h>
#include <vtkFollower.h>
#include "QVtkRenderer.h"
#include "QVtkItem.h"
#include "Utilities.h"

using namespace mb_system;

QVtkRenderer::QVtkRenderer() :
  displayProperties_(nullptr),
  item_(nullptr),
  initialized_(false),
  gridFilename_(nullptr),
  wheelEvent_(nullptr),
  mouseButtonEvent_(nullptr),
  mouseMoveEvent_(nullptr),
  prevZScale_(1.)
{
    worker_ = new LoadFileWorker(*this);

    // Handle things when worker thread finishes
    connect(worker_, &QThread::finished, this, &QVtkRenderer::handleFileLoaded,
            Qt::QueuedConnection);
  
}

QOpenGLFramebufferObject *QVtkRenderer::createFramebufferObject(const QSize &size) {

  // qDebug() << "QVtkRenderer::createFrameBufferObject";
  QOpenGLFramebufferObjectFormat format;
  format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

  // optionally enable multisampling by doing format.setSamples(4);
  return new QOpenGLFramebufferObject(size, format);
}


void QVtkRenderer::render() {

  if (!worker_->okToRender()) {
    qDebug() << "QVtkRenderer::render() do not render yet";
    return;
  }

  if (!renderWindow_ || !windowInteractor_) {
    qDebug() << "renderWindow not yet defined";
    return;
  }

  qDebug() << "QVtkRenderer::render()";
  
  renderWindow_->PushState();
  
  initializeOpenGLState();
  renderWindow_->Start();


  if (!initialized_) {
    initialize();
    initialized_ = true;
  }

  axesActor_->SetVisibility(displayProperties_->showAxes);
  
  // User changed vertical scale - rebuild pipeline
  if (displayProperties_->verticalExagg != prevZScale_) {
    assemblePipeline();
    prevZScale_ = displayProperties_->verticalExagg;
  }
  
  if (wheelEvent_ && !wheelEvent_->isAccepted()) {
    // qDebug() << "render(): handle wheelEvent";
    if (wheelEvent_->delta() > 0) {
      windowInteractor_->InvokeEvent(vtkCommand::MouseWheelForwardEvent);
    }
    else {
      windowInteractor_->InvokeEvent(vtkCommand::MouseWheelBackwardEvent);
    }
    wheelEvent_->accept();
  }
  
  if (mouseButtonEvent_ && !mouseButtonEvent_->isAccepted()) {
    // qDebug() << "render(): handle mouseButtonEvent";



    if (mouseButtonEvent_->type() == QEvent::MouseButtonPress) {

      bool cntrlKey =
        (mouseButtonEvent_->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0;
      
      bool shiftKey =
        (mouseButtonEvent_->modifiers() & Qt::ShiftModifier) > 0 ? 1 : 0;
      
      bool dblClick =
        mouseButtonEvent_->type() == QEvent::MouseButtonDblClick ? 1 : 0;
        
      if (mouseButtonEvent_->buttons() & Qt::LeftButton) {
        // qDebug() << "QVtkRenderer() - got left button";
        qDebug() << "x: " << mouseButtonEvent_->x() <<
          " y: " << mouseButtonEvent_->y();
        
        windowInteractor_->SetEventInformation(mouseButtonEvent_->x(),
                                               mouseButtonEvent_->y(),
                                               cntrlKey,
                                               shiftKey,
                                               dblClick);

        windowInteractor_->InvokeEvent(vtkCommand::LeftButtonPressEvent);
      }
      else if (mouseButtonEvent_->buttons() & Qt::RightButton) {
        // qDebug() << "QVtkRenderer() - got right button";
        windowInteractor_->SetEventInformation(mouseButtonEvent_->x(),
                                               mouseButtonEvent_->y(),
                                               cntrlKey,
                                               shiftKey,
                                               dblClick);
        
        windowInteractor_->InvokeEvent(vtkCommand::RightButtonPressEvent);
      }
      else if (mouseButtonEvent_->buttons() & Qt::MiddleButton) {
        // qDebug() << "QVtkRenderer() - got middle button";
        windowInteractor_->InvokeEvent(vtkCommand::MiddleButtonPressEvent);        
      }      
    }
    else if (mouseButtonEvent_->type() == QEvent::MouseButtonRelease) {
      // qDebug() << "QVtkRenderer: mouse button release";
      if (mouseButtonEvent_->button() == Qt::LeftButton) {
        // qDebug() << "QVtkRenderer: left mouse button release";
        windowInteractor_->InvokeEvent(vtkCommand::LeftButtonReleaseEvent);
      }
      else if (mouseButtonEvent_->button() == Qt::RightButton) {
        // qDebug() << "QVtkRenderer: right mouse button release";
        windowInteractor_->InvokeEvent(vtkCommand::RightButtonReleaseEvent);
      }
      else if (mouseButtonEvent_->button() == Qt::MiddleButton) {
        // qDebug() << "QVtkRenderer: right mouse button release";
        windowInteractor_->InvokeEvent(vtkCommand::MiddleButtonReleaseEvent);
      }      
    }
    mouseButtonEvent_->accept();
  }

  if (mouseMoveEvent_ && !mouseMoveEvent_->isAccepted()) {
    // qDebug() << "QVtkRenderer::render() - mouse move event";
    
    bool cntrlKey =
      (mouseButtonEvent_->modifiers() & Qt::ControlModifier) > 0 ? 1 : 0;
      
    bool shiftKey =
      (mouseButtonEvent_->modifiers() & Qt::ShiftModifier) > 0 ? 1 : 0;
      
    bool dblClick =
      mouseButtonEvent_->type() == QEvent::MouseButtonDblClick ? 1 : 0;
    
    if (mouseMoveEvent_->type() == QEvent::MouseMove) {
      // Got left-button mouse-drag
      // qDebug() << "QVtkRenderer::render(): command mouse move; x=" <<
      // mouseMoveEvent_->x() << ", y=" <<
      // mouseMoveEvent_->y();

      windowInteractor_->SetEventInformation(mouseMoveEvent_->x(),
                                             mouseMoveEvent_->y(),
                                             cntrlKey,
                                             shiftKey,
                                             dblClick);

      windowInteractor_->InvokeEvent(vtkCommand::MouseMoveEvent);
      mouseMoveEvent_->accept();
    }
  }

  int *rendererSize = renderWindow_->GetSize();

  if (item_->width() != rendererSize[0] || item_->height() != rendererSize[1]) {
    // Resize render window to fit within its QVtkItem 
    renderWindow_->SetSize(item_->width(), item_->height());
  }

  renderWindow_->Render();
  
  // Done with render. Reset OpenGL state
  renderWindow_->PopState();
  item_->window()->resetOpenGLState();;

}


// Copy data from item to this renderer
void QVtkRenderer::synchronize(QQuickFramebufferObject *item) {
  
  if (!item_) {
    // The item argument is the QVtkItem associated with this renderer;
    // keep a copy as item_ member
    item_ = static_cast<QVtkItem *>(item);
  }

  // Copy pointer to display properties
  displayProperties_ = item_->displayProperties();
  
  if (gridFilenameChanged(item_->getGridFilename())) {

    gridReader_ = vtkSmartPointer<GmtGridReader>::New();
    
    qDebug() << "synchronize(): change busy state to true";
    item_->setAppBusy(true);

    qDebug() << "synchronize(): start worker thread";
    worker_->start();
    qDebug() << "synchronize(): worker started!";
  }
  
  // Mouse wheel moved
  if (item_->latestWheelEvent() &&
      !item_->latestWheelEvent()->isAccepted()) {
    // Copy and accept latest wheel event
    // qDebug() << "synchronize() - copy wheelEvent";
    // Get latest wheel event generated by the QVtkItem
    wheelEvent_ = std::make_shared<QWheelEvent>(*item_->latestWheelEvent());
    item_->latestWheelEvent()->accept();
  }

  // Mouse button pressed/released
  if (item_->latestMouseButtonEvent() &&
      !item_->latestMouseButtonEvent()->isAccepted()) {
    // qDebug() << "synchronize() - copy mouseButtonEvent";
    // Get latest mouse button event generated by the QVtkItem
    mouseButtonEvent_ =
      std::make_shared<QMouseEvent>(*item_->latestMouseButtonEvent());
    item_->latestMouseButtonEvent()->accept();
  }

  // Mouse moved
  if (item_->latestMouseMoveEvent() &&
      !item_->latestMouseMoveEvent()->isAccepted()) {
    // qDebug() << "synchronize() - copy mouseMoveEvent";
    // Get latest mouse move event generated by the QVtkItem    
    mouseMoveEvent_ =
      std::make_shared<QMouseEvent>(*item_->latestMouseMoveEvent());
    item_->latestMouseMoveEvent()->accept();
  }
}


void QVtkRenderer::initialize() {
  qDebug() << "QVtkRenderer::initialize()";
  initialized_ = true;
}



bool QVtkRenderer::initializePipeline(const char *gridFilename) {
  qDebug() << "QVtkRenderer::initializePipeline() " << gridFilename;

  // Colors for axes
  namedColors_ = vtkSmartPointer<vtkNamedColors>::New();


  // Color data points based on z-value
  elevColorizer_ =
    vtkSmartPointer<vtkElevationFilter>::New();

  
  elevLookupTable_ =
    vtkSmartPointer<vtkLookupTable>::New();
  
  
  // Create VTK renderer (not the same as QT renderer)
  qDebug() << "create vtk renderer";
  renderer_ =
    vtkSmartPointer<vtkRenderer>::New();

  // Create rotation transform and filter
  transform_ = vtkSmartPointer<vtkTransform>::New();
  transformFilter_ = vtkSmartPointer<vtkTransformFilter>::New();
  
  // Create mapper
  qDebug() << "create vtk mapper";
  surfaceMapper_ = vtkSmartPointer<vtkPolyDataMapper>::New();

  // Create actor for grid surface
  qDebug() << "create vtk actor";
  surfaceActor_ = vtkSmartPointer<vtkActor>::New();

  // Create vtk renderWindow
  qDebug() << "create renderWindow";
  renderWindow_ =
    vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();

  // Create vtk windowInteractor
  qDebug() << "create windowInteractor";
  windowInteractor_ =
    vtkSmartPointer<vtkGenericRenderWindowInteractor>::New();

  // Create interactor style
  interactorStyle_ =
    vtkSmartPointer<PickerInteractorStyle>::New();  

  interactorStyle_->initialize(this, windowInteractor_);

  // Axes actor
  axesActor_ = vtkSmartPointer<vtkCubeAxesActor>::New();

  bool status = assemblePipeline();

  return status;
}


void QVtkRenderer::initializeOpenGLState()
{
  renderWindow_->OpenGLInitState();
  renderWindow_->MakeCurrent();
  QOpenGLFunctions::initializeOpenGLFunctions();
  QOpenGLFunctions::glUseProgram(0);
}



void QVtkRenderer::setupAxes(vtkCubeAxesActor *axesActor,
                             vtkColor3d &axisColor,
                             double *surfaceBounds,
                             double *gridBounds,
                             const char *xUnits, const char *yUnits,
                             const char *zUnits) {

  qDebug() << "setupAxes(): " <<
    " xMin: " << surfaceBounds[0] << ", xMax: " << surfaceBounds[1] <<
    ", yMin: " << surfaceBounds[2] << ", yMax: " << surfaceBounds[3] <<
    ", zMin: " << surfaceBounds[4] << ", zMax: " << surfaceBounds[5];
  
  axesActor->SetUseTextActor3D(0);


  axesActor->SetBounds(surfaceBounds);
  
  axesActor->SetXAxisRange(gridBounds[0], gridBounds[1]);
  axesActor->SetYAxisRange(gridBounds[2], gridBounds[3]);  
  axesActor->SetZAxisRange(gridBounds[4], gridBounds[5]);
  
  axesActor->GetTitleTextProperty(0)->SetColor(axisColor.GetData());
  axesActor->GetTitleTextProperty(0)->SetFontSize(48);
  axesActor->GetLabelTextProperty(0)->SetColor(axisColor.GetData());

  axesActor->GetTitleTextProperty(1)->SetColor(axisColor.GetData());
  axesActor->GetLabelTextProperty(1)->SetColor(axisColor.GetData());

  axesActor->GetTitleTextProperty(2)->SetColor(axisColor.GetData());
  axesActor->GetLabelTextProperty(2)->SetColor(axisColor.GetData());
  
  axesActor->GetXAxesLinesProperty()->SetColor(axisColor.GetData());
  axesActor->GetYAxesLinesProperty()->SetColor(axisColor.GetData());
  axesActor->GetZAxesLinesProperty()->SetColor(axisColor.GetData());    
  
  axesActor->DrawXGridlinesOn();
  axesActor->DrawYGridlinesOn();
  axesActor->DrawZGridlinesOn();
  
  axesActor->SetXTitle(xUnits);
  axesActor->SetYTitle(yUnits);
  axesActor->SetZTitle(zUnits);

#if VTK_MAJOR_VERSION == 6
  axesActor->SetGridLineLocation(VTK_GRID_LINES_FURTHEST);
#endif
#if VTK_MAJOR_VERSION > 6
  axesActor->SetGridLineLocation(
				  axesActor->VTK_GRID_LINES_FURTHEST);
#endif
  
  axesActor->XAxisMinorTickVisibilityOff();
  axesActor->YAxisMinorTickVisibilityOff();
  axesActor->ZAxisMinorTickVisibilityOff();

  // axesActor->SetFlyModeToStaticEdges();
  
}


bool QVtkRenderer::assemblePipeline() {

  qDebug() << "QVtkRenderer::assemblePipeline() for " << gridFilename_;

  double gridBounds[6];
  gridReader_->gridBounds(&gridBounds[0], &gridBounds[1],
                          &gridBounds[2], &gridBounds[3],
                          &gridBounds[4], &gridBounds[5]);
  
  qDebug() << "xMin: " << gridBounds[0] << ", xMax: " << gridBounds[1] <<
    "yMin: " << gridBounds[2] << ", yMax: " << gridBounds[3] <<
    "zMin: " << gridBounds[4] << ", zMax: " << gridBounds[5];


  double *dBounds = gridReader_->GetOutput()->GetBounds();
  
  qDebug() << "GetBounds() - xMin: " << dBounds[0] << ", xMax: " <<
    dBounds[1] <<
    "yMin: " << dBounds[2] << ", yMax: " << dBounds[3] <<
    "zMin: " << dBounds[4] << ", zMax: " << dBounds[5];


  elevColorizer_->SetInputConnection(gridReader_->GetOutputPort());  
  elevColorizer_->SetLowPoint(0, 0, gridBounds[4]);
  elevColorizer_->SetHighPoint(0, 0, gridBounds[5]);
  elevColorizer_->SetScalarRange(gridBounds[4], gridBounds[5]);
  
  bool doTransform = true;
  if (doTransform) {
    float zScale = displayProperties_->verticalExagg;
    qDebug() << "zScale: " << zScale;
    /// Do we have to recreate transform and transform filter each time?
    transform_ = vtkSmartPointer<vtkTransform>::New();
    transformFilter_ = vtkSmartPointer<vtkTransformFilter>::New();  
    transform_->Scale(1., 1., zScale);
    transformFilter_->SetTransform(transform_);

    // transformFilter_->SetInputConnection(elevColorizer_->GetOutputPort());
    transformFilter_->SetInputConnection(elevColorizer_->GetOutputPort());
    surfaceMapper_->SetInputConnection(transformFilter_->GetOutputPort());
  }
  else {
    surfaceMapper_->SetInputConnection(elevColorizer_->GetOutputPort());    
  }

  qDebug() << "create LUT";
  makeLookupTable(Haxby, elevLookupTable_);

  double *mapperBounds = surfaceMapper_->GetBounds();
  qDebug() << "mapper z bounds: " << mapperBounds[4] <<
    " " << mapperBounds[5];
    
  qDebug() << "SetScalarRange " << dBounds[4] << "  " << dBounds[5];
  surfaceMapper_->SetScalarRange(dBounds[4], dBounds[5]);
  surfaceMapper_->ScalarVisibilityOn();
  surfaceMapper_->SetLookupTable(elevLookupTable_);

  mapperBounds = surfaceMapper_->GetBounds();
  qDebug() << "NOW - mapper z bounds: " << mapperBounds[4] <<
    " " << mapperBounds[5];
    
  // Assign surfaceMapper to actor
  qDebug() << "assign surfaceMapper to actor";
  surfaceActor_->SetMapper(surfaceMapper_);

  // Add actor to renderer
  renderer_->AddActor(surfaceActor_);

  // Add renderer to the renderWindow
  qDebug() << "add renderer to renderWindow";
  renderWindow_->AddRenderer(renderer_);

  interactorStyle_->SetDefaultRenderer(renderer_);
  interactorStyle_->polyData_ = gridReader_->GetOutput();

  windowInteractor_->SetInteractorStyle(interactorStyle_);
  windowInteractor_->SetRenderWindow(renderWindow_);
  
  // Per QtVTK example
  windowInteractor_->EnableRenderOff();

  vtkColor3d axisColor = namedColors_->GetColor3d("black");

  // Set up axes
  setupAxes(axesActor_, axisColor,
            surfaceMapper_->GetBounds(),
            gridBounds,
            gridReader_->xUnits(), gridReader_->yUnits(),
            gridReader_->zUnits());

  axesActor_->SetCamera(renderer_->GetActiveCamera());

  renderer_->AddActor(axesActor_);    

  renderer_->ResetCamera();

  qDebug() << "pipeline assembled";  
  return true;

}



bool QVtkRenderer::assemblePipelineTest(mb_system::GmtGridReader *gridReader,
                                        const char *gridFilename,
                                        vtkElevationFilter *elevColorizer,
                                        vtkRenderer *renderer,
                                        vtkPolyDataMapper *surfaceMapper,
                                        vtkGenericOpenGLRenderWindow *renderWindow,
                                        vtkGenericRenderWindowInteractor *windowInteractor,
                                        PickerInteractorStyle *interactorStyle,
                                        vtkActor *surfaceActor,
                                        vtkCubeAxesActor *axesActor,
                                        const DisplayProperties *displayProperties) {


  qDebug() << "QVtkRenderer::assemblePipeline() " << gridFilename;

  double gridBounds[6];
  gridReader->gridBounds(&gridBounds[0], &gridBounds[1],
                         &gridBounds[2], &gridBounds[3],
                         &gridBounds[4], &gridBounds[5]);
  
  qDebug() << "xMin: " << gridBounds[0] << ", xMax: " << gridBounds[1] <<
    "yMin: " << gridBounds[2] << ", yMax: " << gridBounds[3] <<
    "zMin: " << gridBounds[4] << ", zMax: " << gridBounds[5];


  double *dBounds = gridReader->GetOutput()->GetBounds();
  
  qDebug() << "GetBounds() - xMin: " << dBounds[0] << ", xMax: " <<
    dBounds[1] <<
    "yMin: " << dBounds[2] << ", yMax: " << dBounds[3] <<
    "zMin: " << dBounds[4] << ", zMax: " << dBounds[5];

  elevColorizer->SetInputConnection(gridReader->GetOutputPort());  
  elevColorizer->SetLowPoint(0, 0, gridBounds[4]);
  elevColorizer->SetHighPoint(0, 0, gridBounds[5]);

  float zScale = 1.;
  
  bool scaleZ = true;
  if (scaleZ) {
    /* **
    // Compute z-scale factor
    zScale = GmtGridReader::zScaleLatLon(gridBounds[3] - gridBounds[2],
                                         gridBounds[1] - gridBounds[0],
                                         gridBounds[5] - gridBounds[4]);
                                         *** */
    zScale = 1.;
    zScale = displayProperties->verticalExagg;
    
    std::cout << "zScale " << zScale << std::endl;

    vtkNew<vtkTransform> transform;
    qDebug() << "zScale: " << zScale;
    transform->Scale(1., 1., zScale);
    vtkNew<vtkTransformFilter> zScaleFilter;
    zScaleFilter->SetTransform(transform);
    zScaleFilter->SetInputConnection(elevColorizer->GetOutputPort());
  
    surfaceMapper->SetInputConnection(zScaleFilter->GetOutputPort());
  }
  else {
    surfaceMapper->SetInputConnection(elevColorizer->GetOutputPort());
  }
  
  // Assign surfaceMapper to actor
  qDebug() << "assign surfaceMapper to actor";
  surfaceActor->SetMapper(surfaceMapper);

  // Add actor to renderer
  renderer->AddActor(surfaceActor);

  // Add renderer to the renderWindow
  qDebug() << "add renderer to renderWindow";
  renderWindow->AddRenderer(renderer);

  interactorStyle->SetDefaultRenderer(renderer);
  interactorStyle->polyData_ = gridReader->GetOutput();

  windowInteractor->SetInteractorStyle(interactorStyle);
  windowInteractor->SetRenderWindow(renderWindow);
  
  // Per QtVTK example
  windowInteractor->EnableRenderOff();

  // Colors for axes
  vtkSmartPointer<vtkNamedColors> colors = 
    vtkSmartPointer<vtkNamedColors>::New();

  vtkColor3d axisColor = colors->GetColor3d("black");

  // Set up axes
  setupAxes(axesActor, axisColor,
            surfaceMapper->GetBounds(),
            gridBounds,
            gridReader->xUnits(), gridReader->yUnits(), gridReader->zUnits());

  axesActor->SetCamera(renderer->GetActiveCamera());

  renderer->AddActor(axesActor);    

  renderer->ResetCamera();

  // Initialize the OpenGL context for the renderer
  ///  renderWindow->OpenGLInitContext();

  qDebug() << "pipeline assembled";  
  return true;
}


bool QVtkRenderer::gridFilenameChanged(char *filename) {
  bool changed = false;

  if (!filename && gridFilename_) {
    // Item now has no gridFilename, free renderer's member and set to null
    free((void *)gridFilename_);
    gridFilename_ = nullptr;
    changed = true;
  }
  else if (filename && !gridFilename_) {
    // Item now has gridFilename, renderer doesn't - copy it
    gridFilename_ = strdup(filename);
    changed = true;
  }
  else if (filename && gridFilename_) {
    if (strcmp(filename, gridFilename_)) {
      // Item filename differs from renderer's - copy it
      changed = true;
      free((void *)gridFilename_);
      gridFilename_ = strdup(filename);
    }
  }
  
  return changed;
}



void QVtkRenderer::handleFileLoaded() {
  // Called when worker thread is finished

  
  qDebug() << "handleFileLoaded() current thread: ";
  qDebug() << QThread::currentThread();

  // Render the FBO again
  update();

  // Initialize the OpenGL context for the renderer
  qDebug() << "initialize OpenGL context for renderer";
  renderWindow_->OpenGLInitContext();
  
  qDebug() << "handleFileLoaded(): change busy state to false";
  item_->setAppBusy(false);

  
}



QVtkRenderer::LoadFileWorker::LoadFileWorker(QVtkRenderer &parent) :
  parent_(parent) {}
  

void QVtkRenderer::LoadFileWorker::run() {
  okToRender_ = true;
  qDebug() << "QVtkRenderer::LoadFileLoader::run()";

  parent_.gridReader_->SetFileName(parent_.gridFilename_);
  parent_.gridReader_->Update();

  if (parent_.gridReader_->GetErrorCode()) {
    std::cerr << "Error during gridReader Update(): " <<
      parent_.gridReader_->GetErrorCode() << std::endl;
    
    return;
  }

  // Critical region - don't render during this phase
  okToRender_ = false;
  qDebug() << "**** handleFileLoaded() - initialize pipeline";
  // Grid file is loaded - initialize pipeline
  parent_.initializePipeline(parent_.gridFilename_);
  qDebug() << "**** handleFileLoaded() - pipeline ready";    

  // All done - ok to render
  okToRender_ = true;  
  qDebug() << "QVtkRenderer::LoadFileLoader::run() finished";
}

