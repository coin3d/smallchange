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
  \class SmExaminerEventHandler SmExaminerEventHandler.h
  \brief The SmExaminerEventHandler class... 
  \ingroup eventhandlers

  FIXME: doc
*/

#include "SmExaminerEventHandler.h"
#include <SmallChange/nodekits/SmCameraControlKit.h>
#include <SmallChange/nodes/UTMCamera.h>
#include <Inventor/actions/SoHandleEventAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/events/SoLocation2Event.h>
#include <Inventor/events/SoMotion3Event.h>
#include <Inventor/events/SoKeyboardEvent.h>
#include <Inventor/events/SoButtonEvent.h>
#include <Inventor/projectors/SbSphereSheetProjector.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/SoPickedPoint.h>
#include <float.h>
#include <assert.h>

SO_NODE_SOURCE(SmExaminerEventHandler);

#define MOUSEPOSLOGSIZE 16


static void add_camera_position(SoCamera * cam, const SbVec3f & val)
{
  if (cam->isOfType(UTMCamera::getClassTypeId())) {
    UTMCamera * utm = (UTMCamera*) cam;
    utm->utmposition = utm->utmposition.getValue() + SbVec3d(val[0], val[1], val[2]);
  }
  else {
    cam->position = cam->position.getValue() + val;
  }
}

SmExaminerEventHandler::SmExaminerEventHandler(void)
{
  SO_NODE_CONSTRUCTOR(SmExaminerEventHandler);

  SO_NODE_ADD_FIELD(enableSpin, (TRUE));
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
  this->button3enabled = TRUE;
}

SmExaminerEventHandler::~SmExaminerEventHandler()
{
  delete this->spinprojector;
  delete[] this->log.position;
  delete[] this->log.time;
}

void
SmExaminerEventHandler::initClass(void)
{
  static int initialized = 0;
  if (!initialized) {
    initialized = 1;
    SO_NODE_INIT_CLASS(SmExaminerEventHandler, SmEventHandler, "SmEventHandler");
  }
}

void 
SmExaminerEventHandler::enableButton3Movement(const SbBool onoff)
{
  this->button3enabled = onoff;
}

float 
SmExaminerEventHandler::clampZoom(const float val)
{
  return val;
}

SbBool 
SmExaminerEventHandler::isAnimating(void)
{
  return this->currentmode != IDLE;
}

void
SmExaminerEventHandler::preRender(SoGLRenderAction * action)
{
  SbTime now = SbTime::getTimeOfDay();
  double secs = now.getValue() - this->prevRedrawTime.getValue();
  this->prevRedrawTime = now;
  
  if (this->currentmode == SPINNING) {
    SbRotation deltaRotation = this->spinRotation;
    deltaRotation.scaleAngle(float(secs * 5.0));
    this->reorientCamera(deltaRotation);
  }
}

void
SmExaminerEventHandler::handleEvent(SoHandleEventAction * action)
{
  const SoEvent * ev = action->getEvent();

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
    case SoMouseButtonEvent::BUTTON4:
      if (press) this->zoom(this->getCamera(), 0.1f);
      break;
    case SoMouseButtonEvent::BUTTON5:
      if (press) this->zoom(this->getCamera(), -0.1f);
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
        add_camera_position(camera, dir);
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

  if (this->button3down && !this->button1down && !this->button3enabled) {
    combo &= !BUTTON3DOWN;
  }

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
        
        if (this->enableSpin.getValue() && (radians > 0.01f) && (deltatime < 0.300)) {
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
  
  add_camera_position(camera, (focalpoint-camera->focalDistance.getValue()*dir)-camera->position.getValue());
  
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
  add_camera_position(camera, (focalpoint-camera->focalDistance.getValue()*dir)-camera->position.getValue());

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
    this->kit->touch(); // // force a redraw in case someone is monitoring isAnimating
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
  if (this->currentmode == IDLE) this->touch();
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
  add_camera_position(cam, (focalpoint-cam->focalDistance.getValue()*direction)-
                      cam->position.getValue());
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
  add_camera_position(cam, -(current_planept-old_planept));
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
    const float newfocaldist = this->clampZoom(oldfocaldist * multiplicator);

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
      add_camera_position(cam, newpos-oldpos);
      cam->focalDistance = newfocaldist;
    }
  }
}

// Will set up a decent, best-guess focal distance for the camera,
// based e.g. on what is in the scene.
void
SmExaminerEventHandler::resetCameraFocalDistance(const SbViewportRegion & vpr)
{
  printf("SmExaminerEventHandler::resetCameraFocalDistance\n");
  SoNode * root = this->kit->getPart("scene", TRUE);
  assert(root);
  SoCamera * const camera = this->getCamera();

  UTMCamera * utmcamera = NULL;
  SbVec3f cameraposition;
  if (camera->isOfType(UTMCamera::getClassTypeId())) {
    utmcamera = (UTMCamera *)camera;
    cameraposition.setValue(utmcamera->utmposition.getValue());
  }
  else {
    cameraposition = camera->position.getValue();
  }
  
  // raypick-intersection attempt
  SoRayPickAction raypick(vpr);
  raypick.setPoint(vpr.getViewportSizePixels() / 2);
  raypick.apply(root);
  
  const SoPickedPoint * pp = raypick.getPickedPoint();
  if (pp) {
    SbVec3f hitpoint = pp->getPoint();
    if (utmcamera) { 
      SbVec3f tmp;
      tmp.setValue(utmcamera->utmposition.getValue());
      hitpoint += tmp; 
    }
    camera->focalDistance = (cameraposition - hitpoint).length();
    return;
  }
    
  // failed raypick-intersection attempt, try with bounding box
  SoGetBoundingBoxAction bba(vpr);
  bba.apply(root);
  const SbBox3f bbox = bba.getBoundingBox();
  if (bbox.hasVolume()) {
    SbSphere boundingsphere;
    boundingsphere.circumscribe(bbox);
    camera->focalDistance = boundingsphere.getRadius();
  }
}


// ************************************************************************

// Set cursor graphics according to mode.
void
SmExaminerEventHandler::setCursorRepresentation(int mode)
{
#if 0 // disabled
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

#undef MOUSEPOSLOGSIZE
