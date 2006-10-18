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

// *************************************************************************

#include "SmCameraControlKit.h"

#include <float.h>

#include <Inventor/actions/SoHandleEventAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoGetMatrixAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoOrthographicCamera.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/SbRotation.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/nodes/SoInfo.h>
#include <Inventor/sensors/SoOneShotSensor.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/SbViewVolume.h>
#include <Inventor/elements/SoViewportRegionElement.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/sensors/SoTimerSensor.h>
#include <Inventor/events/SoKeyboardEvent.h>
#include <Inventor/SoPickedPoint.h>
#include <Inventor/system/gl.h>
#include <Inventor/errors/SoDebugError.h>
#include <Inventor/SbLinear.h>
#include <Inventor/C/tidbits.h>

#include <SmallChange/eventhandlers/SmExaminerEventHandler.h>
#include <SmallChange/nodes/UTMPosition.h>
#include <SmallChange/nodes/UTMCamera.h>
#include <SmallChange/nodes/SmHeadlight.h>

#include "cameracontrol.h"

// *************************************************************************

class SmCameraControlKitP {
public:
  SmCameraControlKitP(SmCameraControlKit * master) : master(master) { }
  
  SmCameraControlKit * master;
  SoOneShotSensor * autoclippingsensor;
  SoFieldSensor * eventhandlersensor;
  static void eventhandlersensor_cb(void * closure, SoSensor * sensor);
  static void autoclip_update(void * closure, SoSensor * sensor);

  static SbBool debug(void);
  int depth_bits;
  
  SoSearchAction * searchaction;
  SoGetMatrixAction * matrixaction;
  SoGetBoundingBoxAction * autoclipbboxaction;
};

SbBool
SmCameraControlKitP::debug(void)
{
  static const char * debug = coin_getenv("SMALLCHANGE_CAMERACONTROLKIT_DEBUG");
  return debug ? TRUE : FALSE;
}

#define PRIVATE(obj) (obj)->pimpl

// *************************************************************************

SO_KIT_SOURCE(SmCameraControlKit);

// *************************************************************************

/*!
  Constructor. 
*/
SmCameraControlKit::SmCameraControlKit(void)
{
  PRIVATE(this) = new SmCameraControlKitP(this);
  PRIVATE(this)->depth_bits = -1; // < 0 means that depth bits is unknown

  PRIVATE(this)->autoclipbboxaction = new SoGetBoundingBoxAction(SbViewportRegion(100,100));
  PRIVATE(this)->searchaction = new SoSearchAction;
  PRIVATE(this)->matrixaction = new SoGetMatrixAction(SbViewportRegion(100,100));

  PRIVATE(this)->autoclippingsensor = 
    new SoOneShotSensor(SmCameraControlKitP::autoclip_update, PRIVATE(this));
  
  SO_KIT_CONSTRUCTOR(SmCameraControlKit);

  SO_KIT_ADD_FIELD(headlight, (TRUE));
  SO_KIT_ADD_FIELD(autoClipping, (TRUE));
  SO_KIT_ADD_FIELD(autoClippingStrategy,(VARIABLE_NEAR_PLANE));
  SO_KIT_ADD_FIELD(autoClippingValue, (0.6f));
  SO_KIT_ADD_FIELD(eventHandler, (NULL));
  SO_KIT_ADD_FIELD(viewUp, (0.0f, 1.0f, 0.0f));
  SO_KIT_ADD_FIELD(handleInheritedEventFirst, (TRUE));

  SO_KIT_DEFINE_ENUM_VALUE(AutoClippingStrategy, VARIABLE_NEAR_PLANE);
  SO_KIT_DEFINE_ENUM_VALUE(AutoClippingStrategy, CONSTANT_NEAR_PLANE);
  SO_KIT_SET_SF_ENUM_TYPE(autoClippingStrategy, AutoClippingStrategy);
  
  SO_KIT_ADD_CATALOG_ENTRY(topSeparator, SoSeparator, FALSE, this, "", FALSE);
  SO_KIT_ADD_CATALOG_ABSTRACT_ENTRY(camera, SoCamera, SoPerspectiveCamera, FALSE, topSeparator, headlightNode, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(headlightNode, SmHeadlight, FALSE, topSeparator, scene, TRUE);
  SO_KIT_ADD_CATALOG_ENTRY(scene, SoSeparator, TRUE, topSeparator, "", TRUE);

  SO_KIT_INIT_INSTANCE();

  this->eventHandler = new SmExaminerEventHandler;
  this->eventHandler.setDefault(TRUE);

  PRIVATE(this)->eventhandlersensor = 
    new SoFieldSensor(PRIVATE(this)->eventhandlersensor_cb, this);
  PRIVATE(this)->eventhandlersensor->attach(&this->eventHandler);

  SmHeadlight * hl = (SmHeadlight*) this->getAnyPart("headlightNode", TRUE);
  hl->on.connectFrom(&this->headlight);
}

/*!
  Destructor.
*/
SmCameraControlKit::~SmCameraControlKit(void)
{
  delete PRIVATE(this)->autoclipbboxaction;
  delete PRIVATE(this)->searchaction;
  delete PRIVATE(this)->matrixaction;
  delete PRIVATE(this)->autoclippingsensor;
  delete PRIVATE(this)->eventhandlersensor;
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

// *************************************************************************

void 
SmCameraControlKit::GLRender(SoGLRenderAction * action)
{
  if (PRIVATE(this)->depth_bits < 0) {
    GLint depthbits[1];
    glGetIntegerv(GL_DEPTH_BITS, depthbits);
    PRIVATE(this)->depth_bits = depthbits[0];
  }

  SoState * state = action->getState();
  SmEventHandler * eh = (SmEventHandler*) this->eventHandler.getValue();
  if (eh) {
    eh->setViewportRegion(SoViewportRegionElement::get(state));
    eh->preRender(action);
  }
  inherited::GLRender(action);
}

void 
SmCameraControlKit::handleEvent(SoHandleEventAction * action)
{
  if (this->handleInheritedEventFirst.getValue())
    inherited::handleEvent(action);
  if (action->isHandled()) { return; }

  const SbViewportRegion vpr = SoViewportRegionElement::get(action->getState());

  const SoEvent * ev = action->getEvent();
  if (SO_KEY_PRESS_EVENT(ev, V)) {
    this->viewAll(vpr);
    action->setHandled();
    return;
  }

  SmEventHandler * eh = (SmEventHandler*) this->eventHandler.getValue();
  if (eh) {
    eh->setViewportRegion(vpr);
    eh->handleEvent(action);
  }
  if (!this->handleInheritedEventFirst.getValue())
    inherited::handleEvent(action);
}

/*!  
  Convenience function for viewAll-functionality. Has special
  handling for UTMCamera and UTMPosition nodes.
*/
void 
SmCameraControlKit::viewAll(const SbViewportRegion & vp,
                            const float slack)
{
  SoCamera * cam = (SoCamera*) this->getPart("camera", TRUE);
  SoNode * root = this->getAnyPart("topSeparator", TRUE);

  SbBool oldsearch = SoBaseKit::isSearchingChildren();
  SoBaseKit::setSearchingChildren(TRUE);
  
  if (cam->isOfType(UTMCamera::getClassTypeId())) {
    SoSearchAction sa;
    sa.setSearchingAll(TRUE);
    sa.setInterest(SoSearchAction::FIRST);
    sa.setType(UTMPosition::getClassTypeId());
    
    sa.apply(root);
    if (sa.getPath()) {
      SoFullPath * p = (SoFullPath*) sa.getPath();
      UTMPosition * utm = (UTMPosition*) p->getTail();  
      ((UTMCamera*)cam)->utmposition = utm->utmposition;
    }
    else {
      SoDebugError::postWarning("SmCameraControlKit::viewAll",
                                "You're using a UTM Camera. "
                                "Please consider supplying at least one UTMPosition node "
                                "in your scene graph.");
    }
  }
  SoBaseKit::setSearchingChildren(oldsearch);
  cam->viewAll(root, vp, slack);
  if (cam->isOfType(UTMCamera::getClassTypeId())) {
    // move from position to utmposition and set position to (0,0,0)
    UTMCamera * utm = (UTMCamera*) cam;
    SbVec3d utmpos = utm->utmposition.getValue();
    SbVec3d tmp;
    tmp.setValue(utm->position.getValue());
    utmpos += tmp;
    utm->position = SbVec3f(0.0f, 0.0f, 0.0f);
    utm->utmposition = utmpos;
  }
}

void 
SmCameraControlKit::pointDir(const SbVec3f & dir, const SbBool resetroll)
{
  SoCamera * camera = (SoCamera*)
    this->getPart("camera", TRUE);

  camera->orientation = SbRotation(SbVec3f(0.0f, 0.0f, -1.0f), dir);
  if (resetroll) {
    this->resetCameraRoll();
  }
}

SbBool 
SmCameraControlKit::setAnyPart(const SbName & partname, SoNode * from,
                               SbBool anypart)
{
  SbBool ret = inherited::setAnyPart(partname, from, anypart);

  // FIXME: consider just using the camera that is in the scene
  // instead of moving it.  pederb, 2003-10-07
  if (ret && partname == "scene") {
    SoSearchAction sa;
    sa.setInterest(SoSearchAction::FIRST);
    sa.setType(SoCamera::getClassTypeId());
    sa.apply(from);
    if (sa.getPath()) {
      SoFullPath * p = (SoFullPath*) sa.getPath();
      SoCamera * cam = (SoCamera*) p->getTail();
      SoNode * parent = p->getNodeFromTail(1);
      if (parent->isOfType(SoGroup::getClassTypeId())) {
        cam->ref();
        SoGroup * g = (SoGroup*) parent;
        g->removeChild(cam);
        SoBaseKit::setAnyPart("camera", cam, TRUE);
        cam->unrefNoDelete();
      }
    }
  }
  return ret;
}

void 
SmCameraControlKit::notify(SoNotList * list)
{
  SoField * f = list->getLastField();

  if (f == &this->camera) {
    if (this->autoClipping.getValue()) PRIVATE(this)->autoclippingsensor->schedule();
  }
  else if (f == &this->scene) {
    if (this->autoClipping.getValue()) PRIVATE(this)->autoclippingsensor->schedule();
  }
  else if (f == &this->eventHandler) {
    // FIXME: ugly, get rid of this, pederb 2003-09-30
    SmEventHandler * eh = (SmEventHandler*) this->eventHandler.getValue();
    if (eh) eh->setCameraControlKit(this);
  }
  inherited::notify(list);
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

  // FIXME: ugly, pederb, 2003-09-30
  SbViewportRegion vp(100,100);
  SmEventHandler * eh = (SmEventHandler*) this->eventHandler.getValue();  
  if (eh) vp = eh->getViewportRegion();
  
  // important to apply on "topSeparator" (and not "scene"), since
  // SoGetBoundingBoxAction needs the UTMCamera to calculate the
  // bounding box correctly
  SoNode * sceneroot = this->getAnyPart("topSeparator", TRUE);
  
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
      // we can't call glGetIntegerv() here, since we might not have a
      // current context. The depth bits is found the first time
      // GLRender() is called.
      int depthbits = PRIVATE(this)->depth_bits;
      if (depthbits < 0) depthbits = 32; // just set to 32 if not found yet

      // From glFrustum() documentation: Depth-buffer precision is
      // affected by the values specified for znear and zfar. The
      // greater the ratio of zfar to znear is, the less effective the
      // depth buffer will be at distinguishing between surfaces that
      // are near each other. If r = far/near, roughly log (2) r bits
      // of depth buffer precision are lost. Because r approaches
      // infinity as znear approaches zero, you should never set znear
      // to zero.
      
      int use_bits = (int) (float(depthbits) * (1.0f-this->autoClippingValue.getValue())); 
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

  SbBool oldnear = camera->nearDistance.enableNotify(FALSE);
  SbBool oldfar = camera->farDistance.enableNotify(FALSE);
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

  if (oldnear) (void) camera->nearDistance.enableNotify(TRUE);
  if (oldfar) (void) camera->farDistance.enableNotify(TRUE);

#if 0 // debug
  SoDebugError::postInfo("SmCameraControlKit::setClippingPlanes",
                         "nearDistance==%f, farDistance==%f\n",
                         camera->nearDistance.getValue(),
                         camera->farDistance.getValue());
#endif // debug

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

SbBool 
SmCameraControlKit::isAnimating(void)
{
  if (::isSeeking()) return TRUE;
  
  SmEventHandler * eh = (SmEventHandler*) this->eventHandler.getValue();
  if (eh) {
    return eh->isAnimating();
  }
  return FALSE;
}

SbBool 
SmCameraControlKit::isBusy(void) const
{
  return ::isSeeking();
}

SbBool
SmCameraControlKit::seekToPoint(const SbVec3d & point,
                                const SbRotation & orientation,
                                const SbViewportRegion & vp)
{
  SoCamera * camera = (SoCamera*) this->getAnyPart("camera", TRUE);
  if (!camera) return FALSE;
  
  ::seekToPoint(camera, point, orientation);
  return TRUE;
}

SbBool 
SmCameraControlKit::seek(const SoEvent * event, const SbViewportRegion & vp)
{
#if 0 // # FIXME: disabled, does not work. fix if needed. (20061018 frodo)
  SoCamera * camera = (SoCamera*) this->getAnyPart("camera", TRUE);
  if (!camera) return FALSE;

  UTMCamera * utmcamera = camera->isOfType(UTMCamera::getClassTypeId()) ?
    (UTMCamera *)camera : NULL;

  SoRayPickAction rpaction(vp);
  rpaction.setPoint(event->getPosition());
  rpaction.setRadius(2);
  rpaction.apply(this->getAnyPart("topSeparator", TRUE));
                 
  SoPickedPoint * picked = rpaction.getPickedPoint();
  if (!picked) return FALSE;
  
  
  SbVec3d hitpoint;
  hitpoint.setValue(picked->getPoint());
  
  if (utmcamera) {
    hitpoint += utmcamera->utmposition.getValue();
  }
  
  if (utmcamera) {
    PRIVATE(this)->seek.camerastartposition = utmcamera->utmposition.getValue();
  }
  else {
    PRIVATE(this)->seek.camerastartposition.setValue(camera->position.getValue());
  }
  PRIVATE(this)->seek.camerastartorient = camera->orientation.getValue();
  
  float fd = PRIVATE(this)->seek.distance / 100.0f;
  fd *= (float) ((hitpoint - PRIVATE(this)->seek.camerastartposition).length());
  camera->focalDistance = fd;

  SbVec3f dir;
  dir.setValue(hitpoint - PRIVATE(this)->seek.camerastartposition);
  dir.normalize();
  
  // find a rotation that rotates current camera direction into new
  // camera direction.
  SbVec3f olddir;
  camera->orientation.getValue().multVec(SbVec3f(0, 0, -1), olddir);
  SbRotation diffrot(olddir, dir);
  SbVec3d tmp;
  tmp.setValue(fd * dir);

  SbVec3d cameraendposition = hitpoint - tmp;
  SbRotation cameraendorient = camera->orientation.getValue() * diffrot;

  this->seekToPoint(cameraendposition, cameraendorient, vp);
#endif

  return TRUE;
}

/*!
  Resets the roll. Useful if you want the "up" vector of the camera to
  be pointing as much up as possible.
*/
void
SmCameraControlKit::resetCameraRoll(void)
{
  SoCamera * camera = (SoCamera*) this->getPart("camera", TRUE);
  resetRoll(camera, this->viewUp.getValue());
}


// Will set up a decent, best-guess focal distance for the camera,
// based e.g. on what is in the scene.
void
SmCameraControlKit::resetCameraFocalDistance(const SbViewportRegion & vpr)
{
  SoNode * node = this->eventHandler.getValue();
  assert(node && node->isOfType(SmEventHandler::getClassTypeId()));
  
  SmEventHandler * eventhandler = (SmEventHandler *) node;
  eventhandler->resetCameraFocalDistance(vpr);
}

// *************************************************************************

#undef PRIVATE
#define PUBLIC(obj) (obj)->master

void 
SmCameraControlKitP::autoclip_update(void * closure, SoSensor * sensor)
{
  SmCameraControlKitP * thisp = (SmCameraControlKitP*) closure;
  thisp->master->setClippingPlanes();
}

void 
SmCameraControlKitP::eventhandlersensor_cb(void * closure, SoSensor * sensor)
{
  SmCameraControlKit * thisp = (SmCameraControlKit*) closure;
  SoNode * node = thisp->eventHandler.getValue();
  assert(node && node->isOfType(SmEventHandler::getClassTypeId()));
  SmEventHandler * eventhandler = (SmEventHandler *) node;
  eventhandler->setCameraControlKit(thisp);
}

#undef PUBLIC

