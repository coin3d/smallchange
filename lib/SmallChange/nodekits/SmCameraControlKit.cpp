// FIXME: will split into several files soon, pederb 2003-09-29

/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2003 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SmallChange with software that can not be combined with the
 *  GNU GPL, and for taking advantage of the additional benefits of our
 *  support services, please contact Systems in Motion about acquiring
 *  a Coin Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org> for  more information.
 *
 *  Systems in Motion, Teknobyen, Abels Gate 5, 7030 Trondheim, NORWAY.
 *  <URL:http://www.sim.no>.
 *
\**************************************************************************/

/*!
  \class SmCameraControlKit SmCameraControlKit.h
  \brief The SmCameraControlKit class... 
  \ingroup nodekits

  FIXME: doc
*/

#include "SmCameraControlKit.h"
#include <Inventor/actions/SoHandleEventAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoGetMatrixAction.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoInfo.h>
#include <Inventor/sensors/SoOneShotSensor.h>
#include <Inventor/projectors/SbSphereSheetProjector.h>
#include <Inventor/SbViewVolume.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/events/SoMotion3Event.h>
#include <Inventor/events/SoKeyboardEvent.h>
#include <Inventor/events/SoButtonEvent.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/SbVec2s.h>
#include <Inventor/SbVec2f.h>
#include <Inventor/system/gl.h>
#include <float.h>

class SmEventHandler {
public:
  SmEventHandler(SmCameraControlKit * kit) : kit(kit), intcnt(0), vp(100,100) { }
  virtual ~SmEventHandler() { };
  
  virtual SbBool handleEvent(SoHandleEventAction * action) = 0;

  void setViewportRegion(const SbViewportRegion & vp) {
    this->vp = vp;
  }
  const SbViewportRegion & getViewportRegion(void) const {
    return this->vp;
  }
  float getGLAspectRatio(void) const {
    return this->vp.getViewportAspectRatio();
  }
  SbVec2s getGLSize(void) const {
    return this->vp.getViewportSizePixels();
  }
protected:
  SbBool isAnimationEnabled(void) const {
    return TRUE;
  }

  void interactiveCountInc(void) {
    this->intcnt++;
  }
  void interactiveCountDec(void) {
    this->intcnt--;
  }

  SmCameraControlKit * kit;
  void setViewing(const SbBool onoff) {
    this->kit->setViewing(onoff);
  }
  SbBool isViewing(void) const {
    return this->kit->isViewing();
  }
  SoCamera * getCamera(void) {
    return (SoCamera*) this->kit->getPart("camera", TRUE);
  }
private:
  int intcnt;
  SbViewportRegion vp;
};

static const int MOUSEPOSLOGSIZE = 16;

class SmExaminerEventHandler : public SmEventHandler {
public:
  SmExaminerEventHandler(SmCameraControlKit * kit);
  virtual ~SmExaminerEventHandler();

  virtual SbBool handleEvent(SoHandleEventAction * action);

  // copied from SoGuiExaminerViewerP.h.in
  void setMotion3OnCamera(SbBool enable);
  SbBool getMotion3OnCamera(void) const;

  float rotXWheelMotion(float value, float old);
  float rotYWheelMotion(float value, float old);

  void reorientCamera(const SbRotation & rotation);
  void spin(const SbVec2f & mousepos);
  void pan(const SbVec2f & mousepos, const SbVec2f & prevpos);
  void zoom(SoCamera * camera, const float diffvalue);
  void zoomByCursor(const SbVec2f & mousepos, const SbVec2f & prevpos);
  void pan(SoCamera * cam,
           float aspectratio, const SbPlane & panningplane,
           const SbVec2f & currpos, const SbVec2f & prevpos);


  SbVec2f lastmouseposition;
  SbPlane panningplane;

  SbBool spinanimatingallowed;
  SbVec2f lastspinposition;
  int spinsamplecounter;
  SbRotation spinincrement;
  SbSphereSheetProjector * spinprojector;

  SbRotation spinRotation;

  SbBool axiscrossEnabled;
  int axiscrossSize;


  struct { // tracking mouse movement in a log
    short size;
    short historysize;
    SbVec2s * position;
    SbTime * time;
  } log;

  // The Microsoft Visual C++ v6.0 compiler needs a name on this class
  // to be able to generate a constructor (which it wants to have for
  // running the the SbVec2s constructors). So don't try to be clever
  // and make it anonymous.
  struct Pointer {
    SbVec2s now, then;
  } pointer;

  SbBool button1down;
  SbBool button3down;
  SbBool ctrldown, shiftdown;

  void clearLog(void);
  void addToLog(const SbVec2s pos, const SbTime time);

  SbTime prevRedrawTime;

  SbBool motion3OnCamera;

  enum ViewerMode {
    IDLE,
    INTERACT,
    ZOOMING,
    PANNING,
    DRAGGING,
    SPINNING,
    SEEK_WAIT_MODE,
    SEEK_MODE
  } mode;

  ViewerMode currentmode;
  void setMode(const ViewerMode mode);

  void setCursorRepresentation(int mode);

private:
};


class SmCameraControlKitP {
public:
  SmCameraControlKitP(SmCameraControlKit * master) : master(master) { }
  
  SmCameraControlKit * master;
  SoOneShotSensor * cameraupdatesensor;
  SoOneShotSensor * autoclippingsensor;
  static void camera_update(void * closure, SoSensor * sensor);
  static void autoclip_update(void * closure, SoSensor * sensor);
  
  SmEventHandler * eventhandler;
  SbBool viewing;
  SoSearchAction * searchaction;
  SoGetMatrixAction * matrixaction;
  SoGetBoundingBoxAction * autoclipbboxaction;
};

#define PRIVATE(obj) (obj)->pimpl

SO_KIT_SOURCE(SmCameraControlKit);

/*!
  Constructor. 
*/
SmCameraControlKit::SmCameraControlKit(void)
{
  PRIVATE(this) = new SmCameraControlKitP(this);
  PRIVATE(this)->autoclipbboxaction = new SoGetBoundingBoxAction(SbViewportRegion(100,100));
  PRIVATE(this)->searchaction = new SoSearchAction;
  PRIVATE(this)->matrixaction = new SoGetMatrixAction(SbViewportRegion(100,100));

  PRIVATE(this)->viewing = TRUE;
  PRIVATE(this)->cameraupdatesensor = 
    new SoOneShotSensor(SmCameraControlKitP::camera_update, PRIVATE(this));
  PRIVATE(this)->autoclippingsensor = 
    new SoOneShotSensor(SmCameraControlKitP::autoclip_update, PRIVATE(this));
  
  SO_KIT_CONSTRUCTOR(SmCameraControlKit);

  SO_KIT_ADD_FIELD(headlight, (TRUE));
  SO_KIT_ADD_FIELD(type, (EXAMINER));
  SO_KIT_ADD_FIELD(autoClipping, (TRUE));
  SO_KIT_ADD_FIELD(autoClippingStrategy,(VARIABLE_NEAR_PLANE));
  SO_KIT_ADD_FIELD(autoClippingValue, (0.6f));

  SO_KIT_DEFINE_ENUM_VALUE(Type, EXAMINER);
  SO_KIT_DEFINE_ENUM_VALUE(Type, HELICOPTER);
  SO_KIT_DEFINE_ENUM_VALUE(Type, PLANE);
  SO_KIT_DEFINE_ENUM_VALUE(Type, SPHERE);
  SO_KIT_SET_SF_ENUM_TYPE(type, Type);

  SO_KIT_DEFINE_ENUM_VALUE(AutoClippingStrategy, VARIABLE_NEAR_PLANE);
  SO_KIT_DEFINE_ENUM_VALUE(AutoClippingStrategy, CONSTANT_NEAR_PLANE);
  SO_KIT_SET_SF_ENUM_TYPE(autoClippingStrategy, AutoClippingStrategy);
 
  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ABSTRACT_ENTRY(camera, SoCamera, SoPerspectiveCamera, FALSE, topSeparator, headlightSwitch, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(headlightSwitch, SoSwitch, FALSE, topSeparator, scene, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(headlightDummy, SoInfo, FALSE, headlightSwitch, headlightNode, FALSE);
  SO_KIT_ADD_CATALOG_ENTRY(headlightNode, SoDirectionalLight, FALSE, headlightSwitch, "", TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(scene, SoSeparator, TRUE, topSeparator, "", TRUE);

  SO_KIT_INIT_INSTANCE();

  PRIVATE(this)->eventhandler = new SmExaminerEventHandler(this);
  
  SoSwitch * sw = (SoSwitch*) this->getAnyPart("headlightSwitch", TRUE);
  sw->whichChild.connectFrom(&this->headlight);
  
  SmCameraControlKitP::camera_update(PRIVATE(this), NULL);
}

/*!
  Destructor.
*/
SmCameraControlKit::~SmCameraControlKit(void)
{
  delete PRIVATE(this)->autoclipbboxaction;
  delete PRIVATE(this)->searchaction;
  delete PRIVATE(this)->matrixaction;
  delete PRIVATE(this)->cameraupdatesensor;
  delete PRIVATE(this)->autoclippingsensor;
  delete PRIVATE(this)->eventhandler;
  delete PRIVATE(this);
}

// Documented in superclass
void
SmCameraControlKit::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_KIT_INIT_CLASS(SmCameraControlKit, SoBaseKit, "BaseKit");
  }
}

void 
SmCameraControlKit::GLRender(SoGLRenderAction * action)
{
  SoState * state = action->getState();
  if (PRIVATE(this)->eventhandler) {
    PRIVATE(this)->eventhandler->setViewportRegion(SoViewportRegionElement::get(state));
  }
  inherited::GLRender(action);
}

void 
SmCameraControlKit::handleEvent(SoHandleEventAction * action)
{
  SoState * state = action->getState();

  if (PRIVATE(this)->eventhandler && this->isViewing()) {
    if (!PRIVATE(this)->eventhandler->handleEvent(action)) {
      inherited::handleEvent(action);
    }
  }
  else {
    inherited::handleEvent(action);
  }
}

void 
SmCameraControlKit::notify(SoNotList * list)
{
  SoField * f = list->getLastField();
  if (f == &this->camera) {
    PRIVATE(this)->cameraupdatesensor->schedule();
    PRIVATE(this)->autoclippingsensor->schedule();
  }
  else if (f == &this->scene) {
    PRIVATE(this)->autoclippingsensor->schedule();
  }
  inherited::notify(list);
}

void 
SmCameraControlKit::setViewing(const SbBool onoff)
{
  PRIVATE(this)->viewing = onoff;
}

SbBool 
SmCameraControlKit::isViewing(void) const
{
  return PRIVATE(this)->viewing;
}

// Position the near and far clipping planes just in front of and
// behind the scene's bounding box. This will give us the optimal
// utilization of the z buffer resolution by shrinking it to its
// minimum depth.
//
// Near and far clipping planes are specified in the camera fields
// nearDistance and farDistance.
void
SmCameraControlKit::setClippingPlanes(void)
{
  SoCamera * camera = (SoCamera*) this->getAnyPart("camera", TRUE);
  // This is necessary to avoid a crash in case there is no scene
  // graph specified by the user.
  if (camera == NULL) return;

  const SbViewportRegion & vp = PRIVATE(this)->eventhandler->getViewportRegion();
  
  SoNode * sceneroot = this->getPart("scene", TRUE);
  
  PRIVATE(this)->autoclipbboxaction->setViewportRegion(vp);
  PRIVATE(this)->autoclipbboxaction->apply(sceneroot);

  SbXfBox3f xbox = PRIVATE(this)->autoclipbboxaction->getXfBoundingBox();

  SbMatrix cammat;
  SbMatrix inverse;
  this->getCameraCoordinateSystem(camera, sceneroot, cammat, inverse);
  xbox.transform(inverse);

  SbMatrix mat;
  mat.setTranslate(- camera->position.getValue());
  xbox.transform(mat);
  mat = camera->orientation.getValue().inverse();
  xbox.transform(mat);
  SbBox3f box = xbox.project();

  // Bounding box was calculated in camera space, so we need to "flip"
  // the box (because camera is pointing in the (0,0,-1) direction
  // from origo.
  float nearval = -box.getMax()[2];
  float farval = -box.getMin()[2];

  // FIXME: what if we have a weird scale transform in the scenegraph?
  // Could we end up with nearval > farval then? Investigate, then
  // either use an assert() (if it can't happen) or an So@Gui@Swap()
  // (to handle it). 20020116 mortene.

  // Check if scene is completely behind us.
  if (farval <= 0.0f) { return; }
  
  if (camera->isOfType(SoPerspectiveCamera::getClassTypeId())) {
    // Disallow negative and small near clipping plane distance.
    
    float nearlimit; // the smallest value allowed for nearval
    if (this->autoClippingStrategy.getValue() == CONSTANT_NEAR_PLANE) {
      nearlimit = this->autoClippingValue.getValue();
    }
    else {
      // From glFrustum() documentation: Depth-buffer precision is
      // affected by the values specified for znear and zfar. The
      // greater the ratio of zfar to znear is, the less effective the
      // depth buffer will be at distinguishing between surfaces that
      // are near each other. If r = far/near, roughly log (2) r bits
      // of depth buffer precision are lost. Because r approaches
      // infinity as znear approaches zero, you should never set znear
      // to zero.

      GLint depthbits[1];
      glGetIntegerv(GL_DEPTH_BITS, depthbits);
      
      int use_bits = (int) (float(depthbits[0]) * (1.0f-this->autoClippingValue.getValue())); 
      float r = (float) pow(2.0, (double) use_bits);
      nearlimit = farval / r;
    }
    
    if (nearlimit >= farval) {
      // (The "5000" magic constant was found by fiddling around a bit
      // on an OpenGL implementation with a 16-bit depth-buffer
      // resolution, adjusting to find something that would work well
      // with both a very "stretched" / deep scene and a more compact
      // single-model one.)
      nearlimit = farval / 5000.0f;
    }
    
    // adjust the near plane if the the value is too small.
    if (nearval < nearlimit) {
      nearval = nearlimit;
    }    
  }
  // Some slack around the bounding box, in case the scene fits
  // exactly inside it. This is done to minimize the chance of
  // artifacts caused by the limitation of the z-buffer
  // resolution. One common artifact if this is not done is that the
  // near clipping plane cuts into the corners of the model as it's
  // rotated.
  const float SLACK = 0.001f;

  // FrustumCamera can be found in the SmallChange CVS module. We
  // should not change the nearDistance for this camera, as this will
  // modify the frustum.
  if (camera->getTypeId().getName() == "FrustumCamera") {
    nearval = camera->nearDistance.getValue();
    farval = farval * (1.0f + SLACK); 
    if (farval <= nearval) {
      // nothing is visible, so just set farval to som value > nearval.
      farval = nearval + 10.0f;
    }
    camera->farDistance = farval;
  }
  else {
    camera->nearDistance = nearval * (1.0f - SLACK);
    camera->farDistance = farval * (1.0f + SLACK);
  }

  // FIXME: there's a possible optimization to take advantage of here,
  // since we are able to sometimes know for sure that all geometry is
  // completely inside the view volume. I quote from the "OpenGL FAQ
  // and Troubleshooting Guide":
  //
  //  "10.050 I know my geometry is inside the view volume. How can I
  //  turn off OpenGL's view-volume clipping to maximize performance?
  //
  //   Standard OpenGL doesn't provide a mechanism to disable the
  //   view-volume clipping test; thus, it will occur for every
  //   primitive you send.
  //
  //   Some implementations of OpenGL support the
  //   GL_EXT_clip_volume_hint extension. If the extension is
  //   available, a call to
  //   glHint(GL_CLIP_VOLUME_CLIPPING_HINT_EXT,GL_FASTEST) will inform
  //   OpenGL that the geometry is entirely within the view volume and
  //   that view-volume clipping is unnecessary. Normal clipping can
  //   be resumed by setting this hint to GL_DONT_CARE. When clipping
  //   is disabled with this hint, results are undefined if geometry
  //   actually falls outside the view volume."
  //
  // 20020117 mortene.
}

// Returns the coordinate system the current camera is located in. If
// there are transformations before the camera in the scene graph,
// this must be considered before doing certain operations. \a matrix
// and \a inverse will not contain the transformations caused by the
// camera fields, only the transformations traversed before the camera
// in the scene graph.
void
SmCameraControlKit::getCameraCoordinateSystem(SoCamera * camera,
                                              SoNode * root,
                                              SbMatrix & matrix,
                                              SbMatrix & inverse)
{
  PRIVATE(this)->searchaction->reset();
  PRIVATE(this)->searchaction->setSearchingAll(TRUE);
  PRIVATE(this)->searchaction->setInterest(SoSearchAction::FIRST);
  PRIVATE(this)->searchaction->setNode(camera);
  PRIVATE(this)->searchaction->apply(root);

  matrix = inverse = SbMatrix::identity();
  if (PRIVATE(this)->searchaction->getPath()) {
    PRIVATE(this)->matrixaction->apply(PRIVATE(this)->searchaction->getPath());
    matrix = PRIVATE(this)->matrixaction->getMatrix();
    inverse = PRIVATE(this)->matrixaction->getInverse();
  }
  PRIVATE(this)->searchaction->reset();
}

#undef PRIVATE
#define PUBLIC(obj) (obj)->master

void 
SmCameraControlKitP::camera_update(void * closure, SoSensor * sensor)
{
  SmCameraControlKitP * thisp = (SmCameraControlKitP*) closure;
  
  SoCamera * cam = (SoCamera*) PUBLIC(thisp)->getAnyPart("camera", TRUE);
  SoDirectionalLight * l = (SoDirectionalLight*) PUBLIC(thisp)->getAnyPart("headlightNode", TRUE);
  SbVec3f dir(0.0f, 0.0f, -1.0f);
  cam->orientation.getValue().multVec(dir, dir);
  l->direction = dir;
}

void 
SmCameraControlKitP::autoclip_update(void * closure, SoSensor * sensor)
{
  SmCameraControlKitP * thisp = (SmCameraControlKitP*) closure;
  
  thisp->master->setClippingPlanes();
}

#undef PUBLIC

SmExaminerEventHandler::SmExaminerEventHandler(SmCameraControlKit * kit)
  : SmEventHandler(kit)
{
  this->currentmode = IDLE;

  this->prevRedrawTime = SbTime::getTimeOfDay();
  this->spinanimatingallowed = TRUE;
  this->spinsamplecounter = 0;
  this->spinincrement = SbRotation::identity();

  // FIXME: use a smaller sphere than the default one to have a larger
  // area close to the borders that gives us "z-axis rotation"?
  // 19990425 mortene.
  this->spinprojector = new SbSphereSheetProjector(SbSphere(SbVec3f(0, 0, 0), 0.8f));
  SbViewVolume volume;
  volume.ortho(-1, 1, -1, 1, -1, 1);
  this->spinprojector->setViewVolume(volume);

  this->axiscrossEnabled = FALSE;
  this->axiscrossSize = 25;

  this->spinRotation.setValue(SbVec3f(0, 0, -1), 0);

  this->log.size = MOUSEPOSLOGSIZE;
  this->log.position = new SbVec2s [ MOUSEPOSLOGSIZE ];
  this->log.time = new SbTime [ MOUSEPOSLOGSIZE ];
  this->log.historysize = 0;
  this->button1down = FALSE;
  this->button3down = FALSE;
  this->ctrldown = FALSE;
  this->shiftdown = FALSE;
  this->pointer.now = SbVec2s(0, 0);
  this->pointer.then = SbVec2s(0, 0);
  this->motion3OnCamera = TRUE;
}

SmExaminerEventHandler::~SmExaminerEventHandler()
{
  delete this->spinprojector;
  delete[] this->log.position;
  delete[] this->log.time;
}

SbBool 
SmExaminerEventHandler::handleEvent(SoHandleEventAction * action)
{
  const SoEvent * ev = action->getEvent();

  // Let the end-user toggle between camera-interaction mode
  // ("viewing") and scenegraph-interaction mode with ALT key(s).
  if (ev->getTypeId().isDerivedFrom(SoKeyboardEvent::getClassTypeId())) {
    SoKeyboardEvent * ke = (SoKeyboardEvent *)ev;
    switch (ke->getKey()) {
    case SoKeyboardEvent::LEFT_ALT:
    case SoKeyboardEvent::RIGHT_ALT:
      if (this->isViewing() && (ke->getState() == SoButtonEvent::DOWN)) {
        this->setViewing(FALSE);
        return TRUE;
      }
      else if (!this->isViewing() && (ke->getState() == SoButtonEvent::UP)) {
        this->setViewing(TRUE);
        return TRUE;
      }
    default:
      break;
    }
  }

  // We're in "interact" mode (ie *not* the camera modification mode),
  // so don't handle the event here. It should either be forwarded to
  // the scenegraph, or caught by So@Gui@Viewer::processSoEvent() if
  // it's an ESC press (to switch modes).
  if (!this->isViewing()) { return FALSE; }
    
  // Events when in "ready-to-seek" mode are ignored, except those
  // which influence the seek mode itself -- these are handled further
  // up the inheritance hierarchy.
  //  if (this->isSeekMode()) { return FALSE; }

  const SoType type(ev->getTypeId());

  const SbVec2s size(this->getGLSize());
  const SbVec2f prevnormalized = this->lastmouseposition;
  const SbVec2s pos(ev->getPosition());
  const SbVec2f posn((float) pos[0] / (float) SbMax((int)(size[0] - 1), 1),
                     (float) pos[1] / (float) SbMax((int)(size[1] - 1), 1));

  this->lastmouseposition = posn;

  // Set to TRUE if any event processing happened. Note that it is not
  // necessary to restrict ourselves to only do one "action" for an
  // event, we only need this flag to see if any processing happened
  // at all.
  SbBool processed = FALSE;

  const ViewerMode currentmode = this->currentmode;
  ViewerMode newmode = currentmode;

  // Mismatches in state of the modifier keys happens if the user
  // presses or releases them outside the viewer window.
  if (this->ctrldown != ev->wasCtrlDown()) {
    this->ctrldown = ev->wasCtrlDown();
  }
  if (this->shiftdown != ev->wasShiftDown()) {
    this->shiftdown = ev->wasShiftDown();
  }

  // Mouse Button / Spaceball Button handling

  if (type.isDerivedFrom(SoMouseButtonEvent::getClassTypeId())) {
    processed = TRUE;

    const SoMouseButtonEvent * const event = (const SoMouseButtonEvent *) ev;
    const int button = event->getButton();
    const SbBool press = event->getState() == SoButtonEvent::DOWN ? TRUE : FALSE;

    // SoDebugError::postInfo("processSoEvent", "button = %d", button);
    switch (button) {
    case SoMouseButtonEvent::BUTTON1:
      this->button1down = press;
      if (press && (currentmode == SEEK_WAIT_MODE)) {
        newmode = SEEK_MODE;
        assert(0 && "not implemented");
        //        this->seekToPoint(pos); // implicitly calls interactiveCountInc()
      }
      break;
    case SoMouseButtonEvent::BUTTON2:
      processed = FALSE; // pass on to superclass, so popup menu is shown
      break;
    case SoMouseButtonEvent::BUTTON3:
      this->button3down = press;
      break;
    default:
      break;
    }
  }

  // Keyboard handling
  if (type.isDerivedFrom(SoKeyboardEvent::getClassTypeId())) {
    const SoKeyboardEvent * const event = (const SoKeyboardEvent *) ev;
    const SbBool press = event->getState() == SoButtonEvent::DOWN ? TRUE : FALSE;
    switch (event->getKey()) {
    case SoKeyboardEvent::LEFT_CONTROL:
    case SoKeyboardEvent::RIGHT_CONTROL:
      processed = TRUE;
      this->ctrldown = press;
      break;
    case SoKeyboardEvent::LEFT_SHIFT:
    case SoKeyboardEvent::RIGHT_SHIFT:
      processed = TRUE;
      this->shiftdown = press;
      break;
    default:
      break;
    }
  }

  // Mouse Movement handling
  if (type.isDerivedFrom(SoLocation2Event::getClassTypeId())) {
    const SoLocation2Event * const event = (const SoLocation2Event *) ev;

    processed = TRUE;

    if (this->currentmode == ZOOMING) {
      this->zoomByCursor(posn, prevnormalized);
    }
    else if (this->currentmode == PANNING) {
      this->pan(this->getCamera(), this->getGLAspectRatio(),
                this->panningplane, posn, prevnormalized);
    }
    else if (this->currentmode == DRAGGING) {
      this->addToLog(event->getPosition(), event->getTime());
      this->spin(posn);
    }
    else {
      processed = FALSE;
    }
  }

  // Spaceball & Joystick handling
  if (type.isDerivedFrom(SoMotion3Event::getClassTypeId())) {
    SoMotion3Event * const event = (SoMotion3Event *) ev;
    SoCamera * const camera = this->getCamera();
    if (camera) {
      if (this->motion3OnCamera) {
        SbVec3f dir = event->getTranslation();
        camera->orientation.getValue().multVec(dir,dir);
        camera->position = camera->position.getValue() + dir;
        camera->orientation = 
          event->getRotation() * camera->orientation.getValue();
        processed = TRUE;
      }
      else {
        // FIXME: move/rotate model
        SoDebugError::postInfo("So@Gui@ExaminerViewer::processSoEvent",
                               "SoMotion3Event for model movement is not implemented yet");
        processed = TRUE;
      }
    }
  }

  enum {
    BUTTON1DOWN = 1 << 0,
    BUTTON3DOWN = 1 << 1,
    CTRLDOWN =    1 << 2,
    SHIFTDOWN =   1 << 3
  };
  unsigned int combo =
    (this->button1down ? BUTTON1DOWN : 0) |
    (this->button3down ? BUTTON3DOWN : 0) |
    (this->ctrldown ? CTRLDOWN : 0) |
    (this->shiftdown ? SHIFTDOWN : 0);

  switch (combo) {
  case 0:
    if (currentmode == SPINNING) { break; }
    newmode = IDLE;
    if ((currentmode == DRAGGING) &&
        this->isAnimationEnabled() && (this->log.historysize >= 3)) {
      SbTime stoptime = (ev->getTime() - this->log.time[0]);
      if (stoptime.getValue() < 0.100) {
        const SbVec2s glsize(this->getGLSize());
        SbVec3f from = this->spinprojector->project(SbVec2f(float(this->log.position[2][0]) / 
                                                                     float(SbMax(glsize[0]-1, 1)),
                                                                     float(this->log.position[2][1]) / 
                                                                     float(SbMax(glsize[1]-1, 1))));
        SbVec3f to = this->spinprojector->project(posn);
        SbRotation rot = this->spinprojector->getRotation(from, to);

        SbTime delta = (this->log.time[0] - this->log.time[2]);
        double deltatime = delta.getValue();
        rot.invert();
        rot.scaleAngle(float(0.200 / deltatime));

        SbVec3f axis;
        float radians;
        rot.getValue(axis, radians);
        if ((radians > 0.01f) && (deltatime < 0.300)) {
          newmode = SPINNING;
          this->spinRotation = rot;
        }
      }
    }
    break;
  case BUTTON1DOWN:
    newmode = DRAGGING;
    break;
  case BUTTON3DOWN:
  case CTRLDOWN|BUTTON1DOWN:
  case SHIFTDOWN|BUTTON1DOWN:
    newmode = PANNING;
    break;
  case BUTTON1DOWN|BUTTON3DOWN:
  case CTRLDOWN|BUTTON3DOWN:
  case CTRLDOWN|SHIFTDOWN|BUTTON1DOWN:
    newmode = ZOOMING;
    break;

    // There are many cases we don't handle that just falls through to
    // the default case, like SHIFTDOWN, CTRLDOWN, CTRLDOWN|SHIFTDOWN,
    // SHIFTDOWN|BUTTON3DOWN, SHIFTDOWN|CTRLDOWN|BUTTON3DOWN, etc.
    // This is a feature, not a bug. :-)
    //
    // mortene.

  default:
    // The default will make a spin stop and otherwise not do
    // anything.
    if ((currentmode != SEEK_WAIT_MODE) &&
        (currentmode != SEEK_MODE)) {
      newmode = IDLE;
    }
    break;
  }

  if (newmode != currentmode) {
    this->setMode(newmode);
  }

  // If not handled in this class, pass on upwards in the inheritance
  // hierarchy.
  return processed;
}

// The "rotX" wheel is the wheel on the left decoration on the
// examiner viewer.  This function translates interaction with the
// "rotX" wheel into camera movement.
float
SmExaminerEventHandler::rotXWheelMotion(float value, float oldvalue)
{
  SoCamera * camera = this->getCamera();
  if (camera == NULL) return 0.0f; // can happen for empty scenegraph

  SbVec3f dir;
  camera->orientation.getValue().multVec(SbVec3f(0, 0, -1), dir);

  SbVec3f focalpoint = camera->position.getValue() +
    camera->focalDistance.getValue() * dir;

  camera->orientation = SbRotation(SbVec3f(-1, 0, 0), value - oldvalue) *
    camera->orientation.getValue();

  camera->orientation.getValue().multVec(SbVec3f(0, 0, -1), dir);
  camera->position = focalpoint - camera->focalDistance.getValue() * dir;

  return value;
}

// The "rotY" wheel is the wheel on the bottom decoration on the
// examiner viewer.  This function translates interaction with the
// "rotX" wheel into camera movement.
float
SmExaminerEventHandler::rotYWheelMotion(float value, float oldvalue)
{
  SoCamera * camera = this->getCamera();
  if (camera == NULL) return 0.0f; // can happen for empty scenegraph

  SbVec3f dir;
  camera->orientation.getValue().multVec(SbVec3f(0, 0, -1), dir);

  SbVec3f focalpoint = camera->position.getValue() +
    camera->focalDistance.getValue() * dir;

  camera->orientation = SbRotation(SbVec3f(0, 1, 0), oldvalue - value) *
    camera->orientation.getValue();

  camera->orientation.getValue().multVec(SbVec3f(0, 0, -1), dir);
  camera->position = focalpoint - camera->focalDistance.getValue() * dir;

  return value;
}

// ************************************************************************

// The viewer is a state machine, and all changes to the current state
// are made through this call.
void
SmExaminerEventHandler::setMode(const ViewerMode newmode)
{
  const ViewerMode oldmode = this->currentmode;
  if (newmode == oldmode) { return; }

  switch (newmode) {
  case DRAGGING:
    // Set up initial projection point for the projector object when
    // first starting a drag operation.
    this->spinprojector->project(this->lastmouseposition);
    this->interactiveCountInc();
    this->clearLog();
    break;

  case SPINNING:
    this->interactiveCountInc();
    this->kit->touch(); // force redraw
    break;

  case PANNING:
    {
      // The plane we're projecting the mouse coordinates to get 3D
      // coordinates should stay the same during the whole pan
      // operation, so we should calculate this value here.
      SoCamera * cam = this->getCamera();
      if (cam == NULL) { // can happen for empty scenegraph
        this->panningplane = SbPlane(SbVec3f(0, 0, 1), 0);
      }
      else {
        SbViewVolume vv = cam->getViewVolume(this->getGLAspectRatio());
        this->panningplane = vv.getPlane(cam->focalDistance.getValue());
      }
    }
    this->interactiveCountInc();
    break;

  case ZOOMING:
    this->interactiveCountInc();
    break;

  default: // include default to avoid compiler warnings.
    break;
  }

  switch (oldmode) {
  case SPINNING:
  case DRAGGING:
  case PANNING:
  case ZOOMING:
    this->interactiveCountDec();
    break;

  default:
    break;
  }

#if 0
  if (oldmode == ZOOMING) {
    SbVec3f v = this->getCamera()->position.getValue();
    SoDebugError::postInfo("So@Gui@ExaminerViewerP::setMode",
                           "new camera position after zoom: <%e, %e, %e>",
                           v[0], v[1], v[2]);
  }
#endif

  this->setCursorRepresentation(newmode);
  this->currentmode = newmode;
}

void
SmExaminerEventHandler::clearLog(void)
{
  this->log.historysize = 0;
}

// This method adds another point to the mouse location log, used for spin
// animation calculations.
void
SmExaminerEventHandler::addToLog(const SbVec2s pos, const SbTime time)
{
  // In case someone changes the const size setting at the top of this
  // file too small.
  assert (this->log.size > 2 && "mouse log too small!");

  if (this->log.historysize > 0 && pos == this->log.position[0]) {
    // This can at least happen under SoQt.
    SoDebugError::postInfo("SmExaminerEventHandler::addToLog", "got position already!");
    return;
  }

  for (int i = this->log.size - 1; i > 0; i--) {
    this->log.position[i] = this->log.position[i-1];
    this->log.time[i] = this->log.time[i-1];
  }
  this->log.position[0] = pos;
  this->log.time[0] = time;
  if (this->log.historysize < this->log.size)
    this->log.historysize += 1;
}

// Rotate the camera by the given amount, then reposition it so we're
// still pointing at the same focal point.
void
SmExaminerEventHandler::reorientCamera(const SbRotation & rot)
{
  SoCamera * cam = this->getCamera();
  if (cam == NULL) return;

  // Find global coordinates of focal point.
  SbVec3f direction;
  cam->orientation.getValue().multVec(SbVec3f(0, 0, -1), direction);
  SbVec3f focalpoint = cam->position.getValue() +
    cam->focalDistance.getValue() * direction;

  // Set new orientation value by accumulating the new rotation.
  cam->orientation = rot * cam->orientation.getValue();

  // Reposition camera so we are still pointing at the same old focal point.
  cam->orientation.getValue().multVec(SbVec3f(0, 0, -1), direction);
  cam->position = focalpoint - cam->focalDistance.getValue() * direction;
}

// Uses the sphere sheet projector to map the mouseposition unto
// a 3D point and find a rotation from this and the last calculated point.
void
SmExaminerEventHandler::spin(const SbVec2f & pointerpos)
{
  if (this->log.historysize < 2) return;
  assert(this->spinprojector != NULL);
  
  SbVec2s glsize(this->getGLSize());
  SbVec2f lastpos;
  lastpos[0] = float(this->log.position[1][0]) / float(SbMax((int)(glsize[0]-1), 1));
  lastpos[1] = float(this->log.position[1][1]) / float(SbMax((int)(glsize[1]-1), 1));

  this->spinprojector->project(lastpos);
  SbRotation r;
  this->spinprojector->projectAndGetRotation(pointerpos, r);
  r.invert();
  this->reorientCamera(r);

  // Calculate an average angle magnitude value to make the transition
  // to a possible spin animation mode appear smooth.

  SbVec3f dummy_axis, newaxis;
  float acc_angle, newangle;
  this->spinincrement.getValue(dummy_axis, acc_angle);
  acc_angle *= this->spinsamplecounter; // weight
  r.getValue(newaxis, newangle);
  acc_angle += newangle;

  this->spinsamplecounter++;
  acc_angle /= this->spinsamplecounter;
  // FIXME: accumulate and average axis vectors aswell? 19990501 mortene.
  this->spinincrement.setValue(newaxis, acc_angle);

  // Don't carry too much baggage, as that'll give unwanted results
  // when the user quickly trigger (as in "click-drag-release") a spin
  // animation.
  if (this->spinsamplecounter > 3) this->spinsamplecounter = 3;
}

// ************************************************************************

// Calculate a zoom/dolly factor from the difference of the current
// cursor position and the last.
void
SmExaminerEventHandler::zoomByCursor(const SbVec2f & thispos,
                                     const SbVec2f & prevpos)
{
  // There is no "geometrically correct" value, 20 just seems to give
  // about the right "feel".
  this->zoom(this->getCamera(),
             (thispos[1] - prevpos[1]) * 20.0f);
}

// This method sets whether Motion3 events should affect the camera or
// the model.
void
SmExaminerEventHandler::setMotion3OnCamera(SbBool enable)
{
  this->motion3OnCamera = enable;
}

// This method returns whether Motion3 events affects the camera or
// the model.
SbBool
SmExaminerEventHandler::getMotion3OnCamera(void) const
{
  return this->motion3OnCamera;
}

void
SmExaminerEventHandler::pan(SoCamera * cam,
                            float aspectratio, const SbPlane & panningplane,
                            const SbVec2f & currpos, const SbVec2f & prevpos)
{
  if (cam == NULL) return; // can happen for empty scenegraph
  if (currpos == prevpos) return; // useless invocation

  // Find projection points for the last and current mouse coordinates.
  SbViewVolume vv = cam->getViewVolume(aspectratio);
  SbLine line;
  vv.projectPointToLine(currpos, line);
  SbVec3f current_planept;
  panningplane.intersect(line, current_planept);
  vv.projectPointToLine(prevpos, line);
  SbVec3f old_planept;
  panningplane.intersect(line, old_planept);

  // Reposition camera according to the vector difference between the
  // projected points.
  cam->position = cam->position.getValue() - (current_planept - old_planept);
}

void
SmExaminerEventHandler::zoom(SoCamera * cam, const float diffvalue)
{
  if (cam == NULL) return; // can happen for empty scenegraph
  SoType t = cam->getTypeId();
  SbName tname = t.getName();

  // This will be in the range of <0, ->>.
  float multiplicator = float(exp(diffvalue));

  if (t.isDerivedFrom(SoOrthographicCamera::getClassTypeId())) {

    // Since there's no perspective, "zooming" in the original sense
    // of the word won't have any visible effect. So we just increase
    // or decrease the field-of-view values of the camera instead, to
    // "shrink" the projection size of the model / scene.
    SoOrthographicCamera * oc = (SoOrthographicCamera *)cam;
    oc->height = oc->height.getValue() * multiplicator;
    
  }
  else {
    // FrustumCamera can be found in the SmallChange CVS module (it's
    // a camera that lets you specify (for instance) an off-center
    // frustum (similar to glFrustum())
    if (!t.isDerivedFrom(SoPerspectiveCamera::getClassTypeId()) &&
        tname != "FrustumCamera") {
      static SbBool first = TRUE;
      if (first) {
        SoDebugError::postWarning("SmCameraControlKit::zoom",
                                  "Unknown camera type, "
                                  "will zoom by moving position, but this might not be correct.");
        first = FALSE;
      }
    }
    
    const float oldfocaldist = cam->focalDistance.getValue();
    const float newfocaldist = oldfocaldist * multiplicator;

    SbVec3f direction;
    cam->orientation.getValue().multVec(SbVec3f(0, 0, -1), direction);

    const SbVec3f oldpos = cam->position.getValue();
    const SbVec3f newpos = oldpos + (newfocaldist - oldfocaldist) * -direction;

    // This catches a rather common user interface "buglet": if the
    // user zooms the camera out to a distance from origo larger than
    // what we still can safely do floating point calculations on
    // (i.e. without getting NaN or Inf values), the faulty floating
    // point values will propagate until we start to get debug error
    // messages and eventually an assert failure from core Coin code.
    //
    // With the below bounds check, this problem is avoided.
    //
    // (But note that we depend on the input argument ''diffvalue'' to
    // be small enough that zooming happens gradually. Ideally, we
    // should also check distorigo with isinf() and isnan() (or
    // inversely; isinfite()), but those only became standardized with
    // C99.)
    const float distorigo = newpos.length();
    // sqrt(FLT_MAX) == ~ 1e+19, which should be both safe for further
    // calculations and ok for the end-user and app-programmer.
    if (distorigo > float(sqrt(FLT_MAX))) {
      SoDebugError::postWarning("SmCameraControlKit::zoom",
                                "zoomed too far (distance to origo==%f (%e))",
                                distorigo, distorigo);
    }
    else {
      cam->position = newpos;
      cam->focalDistance = newfocaldist;
    }
  }
}

// ************************************************************************

// Set cursor graphics according to mode.
void
SmExaminerEventHandler::setCursorRepresentation(int mode)
{
#if 0
  if (!PUBLIC(this)->isCursorEnabled()) {
    PUBLIC(this)->setComponentCursor(So@Gui@Cursor::getBlankCursor());
    return;
  }

  switch (mode) {
  case SmExaminerEventHandler::INTERACT:
    PUBLIC(this)->setComponentCursor(So@Gui@Cursor(So@Gui@Cursor::DEFAULT));
    break;

  case SmExaminerEventHandler::IDLE:
  case SmExaminerEventHandler::DRAGGING:
  case SmExaminerEventHandler::SPINNING:
    PUBLIC(this)->setComponentCursor(So@Gui@Cursor::getRotateCursor());
    break;

  case SmExaminerEventHandler::ZOOMING:
    PUBLIC(this)->setComponentCursor(So@Gui@Cursor::getZoomCursor());
    break;

  case SmExaminerEventHandler::SEEK_MODE:
  case SmExaminerEventHandler::SEEK_WAIT_MODE:
    PUBLIC(this)->setComponentCursor(So@Gui@Cursor(So@Gui@Cursor::CROSSHAIR));
    break;

  case SmExaminerEventHandler::PANNING:
    PUBLIC(this)->setComponentCursor(So@Gui@Cursor::getPanCursor());
    break;

  default: assert(0); break;
  }
#endif // disabled
}
