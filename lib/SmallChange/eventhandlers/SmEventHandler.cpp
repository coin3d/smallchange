/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
SmEventHandler::resetCameraFocalDistance(const SbViewportRegion & vpr)
{
  // do nothing by default
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
