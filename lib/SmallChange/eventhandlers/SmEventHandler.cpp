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
  \class SmEventHandler SmEventHandler.h
  \brief The SmEventHandler class... 
  \ingroup eventhandlers

  FIXME: doc
*/

#include "SmEventHandler.h"
#include <SmallChange/nodekits/SmCameraControlKit.h>
#include <Inventor/sensors/SoTimerSensor.h>
#include <Inventor/SbMatrix.h>
#include <Inventor/SbRotation.h>
#include <Inventor/nodes/SoCamera.h>

SO_NODE_ABSTRACT_SOURCE(SmEventHandler);

SmEventHandler::SmEventHandler(void) 
  : kit(NULL), intcnt(0), vp(100,100) 
{
  SO_NODE_CONSTRUCTOR(SmEventHandler);

  this->pulser = new SoTimerSensor(pulse_cb, this);
  this->pulseenabled = FALSE;
}

SmEventHandler::~SmEventHandler() 
{
  delete this->pulser;
}

void
SmEventHandler::initClass(void)
{
  static int initialized = 0;
  if (!initialized) {
    initialized = 1;
    SO_NODE_INIT_ABSTRACT_CLASS(SmEventHandler, SoNode, "Node");
  }
}

void 
SmEventHandler::setCameraControlKit(SmCameraControlKit * kit)
{
  this->kit = kit;
}

void 
SmEventHandler::setViewportRegion(const SbViewportRegion & vp) 
{
  this->vp = vp;
}

const SbViewportRegion & 
SmEventHandler::getViewportRegion(void) const 
{
  return this->vp;
}

float 
SmEventHandler::getGLAspectRatio(void) const 
{
  return this->vp.getViewportAspectRatio();
}

SbVec2s 
SmEventHandler::getGLSize(void) const 
{
  return this->vp.getViewportSizePixels();
}

SbBool 
SmEventHandler::isAnimationEnabled(void) const 
{
  return TRUE;
}

void 
SmEventHandler::interactiveCountInc(void) 
{
  this->intcnt++;
}

void 
SmEventHandler::interactiveCountDec(void) 
{
  this->intcnt--;
}

SoCamera * 
SmEventHandler::getCamera(void) 
{
  return (SoCamera*) this->kit->getPart("camera", TRUE);
}

void 
SmEventHandler::preRender(SoGLRenderAction * action)
{
  // do nothing here
}

void 
SmEventHandler::pulse(void)
{
  // do nothing here
}

SbBool 
SmEventHandler::isAnimating(void)
{
  return FALSE;
}

void 
SmEventHandler::enablePulse(const SbBool onoff)
{
  if (onoff && !this->pulseenabled) {
    this->pulseenabled = TRUE;
    this->pulser->schedule();
  }
  else if (!onoff && this->pulseenabled) {
    this->pulseenabled = FALSE;
    this->pulser->unschedule();
  }
}

void 
SmEventHandler::pulse_cb(void * closure, SoSensor * s)
{
  SmEventHandler * thisp = (SmEventHandler*) closure;
  if (thisp->pulseenabled) thisp->pulse();
}

/*!
  Yaw camera with \a rad radians.
*/
void
SmEventHandler::yawCamera(const float rad)
{
  SoCamera * camera = this->getCamera();
  SbMatrix camerarot;
  camerarot.setRotate(camera->orientation.getValue());

  SbMatrix yawmat;
  yawmat.setRotate(SbRotation(SbVec3f(1.0f, 0.0f, 0.0f), rad));
  camerarot.multLeft(yawmat);
  camera->orientation = SbRotation(camerarot);
}

/*!
  Roll camera with \a rad radians.
*/

void
SmEventHandler::rollCamera(const float rad)
{
  SoCamera * camera = this->getCamera();
  SbMatrix camerarot;
  camerarot.setRotate(camera->orientation.getValue());

  SbMatrix rollmat;
  rollmat.setRotate(SbRotation(SbVec3f(0.0f, 0.0f, 1.0f), rad));
  camerarot.multLeft(rollmat);
  camera->orientation = SbRotation(camerarot);
}

/*!
  Pitch camera with \a rad radians.
*/
void
SmEventHandler::pitchCamera(const float rad)
{
  SoCamera * camera = this->getCamera();
  SbMatrix camerarot;
  camerarot.setRotate(camera->orientation.getValue());

  SbMatrix pitchmat;
  pitchmat.setRotate(SbRotation(SbVec3f(0.0f, 1.0f, 0.0f), rad));
  camerarot.multLeft(pitchmat);
  camera->orientation = SbRotation(camerarot);
}

SbVec3f 
SmEventHandler::getViewUp(void) const
{
  return this->kit->viewUp.getValue();
}
