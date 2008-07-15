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
  \class SmHelicopterEventHandler SmHelicopterEventHandler.h
  \brief The SmHelicopterEventHandler class... 
  \ingroup eventhandlers

  FIXME: doc
*/

#include "SmHelicopterEventHandler.h"
#include <Inventor/actions/SoHandleEventAction.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/events/SoLocation2Event.h>
#include <SmallChange/nodes/UTMCamera.h>
#include <Inventor/SbMatrix.h>
#include <Inventor/SbRotation.h>
#include <cmath>

SO_NODE_SOURCE(SmHelicopterEventHandler);

#define WAIT_FOR_BUTTONDOWN 0
#define LMB_DOWN 1
#define RMB_DOWN 2
#define MMB_DOWN 3
#define DIRECTION_NONE 0
#define DIRECTION_FWD 1
#define DIRECTION_BACK 2

SmHelicopterEventHandler::SmHelicopterEventHandler(void)
{
  SO_NODE_CONSTRUCTOR(SmHelicopterEventHandler);

  SO_NODE_ADD_FIELD(speed, (1.0f));
  SO_NODE_ADD_FIELD(resetRoll, (TRUE));
  
  this->state = WAIT_FOR_BUTTONDOWN;
  this->prevpos = SbVec2s(0,0);
  this->mousedownpos = SbVec2s(0,0);
  this->mousepos = SbVec2s(0,0);
  this->flydirection = DIRECTION_NONE;
  this->relspeedfly = 0.0f;
}


SmHelicopterEventHandler::~SmHelicopterEventHandler()
{
}

void
SmHelicopterEventHandler::initClass(void)
{
  static int initialized = 0;
  if (!initialized) {
    initialized = 1;
    SO_NODE_INIT_CLASS(SmHelicopterEventHandler, SmEventHandler, "SmEventHandler");
  }
}

SbBool 
SmHelicopterEventHandler::isAnimation(void)
{
  return this->state != WAIT_FOR_BUTTONDOWN;
}

void
SmHelicopterEventHandler::handleEvent(SoHandleEventAction * action)
{
  const SoEvent * event = action->getEvent();
  this->mousepos = event->getPosition();

  switch (this->state) {
  case WAIT_FOR_BUTTONDOWN:
    if (SO_MOUSE_PRESS_EVENT(event, BUTTON1)) {
      this->state = LMB_DOWN;
      this->enablePulse(TRUE);
      this->mousedownpos = event->getPosition();
    }
    else if (SO_MOUSE_PRESS_EVENT(event, BUTTON2)) {
      this->state = RMB_DOWN;
      this->enablePulse(TRUE);
      this->mousedownpos = event->getPosition();
    }
    else if (SO_MOUSE_PRESS_EVENT(event, BUTTON3)) {
      this->state = MMB_DOWN;
      this->enablePulse(TRUE);
      this->mousedownpos = event->getPosition();
    }
    break;
  case LMB_DOWN:
  case RMB_DOWN:
  case MMB_DOWN:
    if (SO_MOUSE_RELEASE_EVENT(event, ANY)) {
      this->state = WAIT_FOR_BUTTONDOWN;
      this->enablePulse(FALSE);
      this->flydirection = DIRECTION_NONE;
      this->touch(); // force a redraw in case someone is monitoring isAnimating
    }
    break;
  default:
    assert(0 && "unknown state");
    break;
  }
}

void
SmHelicopterEventHandler::pulse(void)
{
  SbVec2s winsize = this->getViewportRegion().getWindowSize();
  int dx, dy;

  dx = this->mousepos[0] - this->mousedownpos[0];
  dy = - (this->mousepos[1] - this->mousedownpos[1]);

  if (this->state == LMB_DOWN) {
    if (dy > 0 && this->flydirection == DIRECTION_BACK) {
      dy -= 20;
      if (dy < 0) dy = 0;
    }
    else if (dy < 0 && this->flydirection == DIRECTION_FWD) {
      dy += 20;
      if (dy > 0) dy = 0;
    }
    
    if (dx == 0 && dy == 0) return;
    this->relspeedfly = SbAbs(float(dy))/float(winsize[1]);
    this->moveCamera(SbVec3f(0.0f, 0.0f, (float)dy), TRUE);
    if (this->flydirection == DIRECTION_NONE && dy) {
      if (dy > 0) this->flydirection = DIRECTION_FWD;
      else this->flydirection = DIRECTION_BACK;
    }
    this->pitchCamera(((float)-dx/float(winsize[0])) * 0.1f);
    if (this->resetRoll.getValue()) this->kit->resetCameraRoll();
  }
  else if (this->state == RMB_DOWN) {
    if (dx == 0 && dy == 0) return;
    this->pitchCamera(((float)-dx / float(winsize[0])) * 0.1f);
    this->yawCamera(((float)-dy / float(winsize[1])) * 0.1f);
    if (this->resetRoll.getValue()) this->kit->resetCameraRoll();
  }
  else if (this->state == MMB_DOWN) {
    if (dx == 0 && dy == 0) return;
    float fx = float(dx) / float(winsize[0]);
    float fy = float(dy) / float(winsize[1]);
    this->relspeedfly = (float) sqrt(fx*fx+fy*fy);
    
    this->moveCamera(SbVec3f((float)dx,
                             (float)(-dy),
                             0.0f), TRUE);
  }
}


void
SmHelicopterEventHandler::moveCamera(const SbVec3f & vec, const SbBool dorotate)
{
  if (vec.sqrLength() == 0.0f) return;

  float dist = this->speed.getValue() * this->relspeedfly;
  if (dist == 0.0f) return;

  SoCamera * camera = (SoCamera*) this->getCamera();
  UTMCamera * utmcamera = (UTMCamera*) camera;

  SbMatrix camerarot;
  camerarot.setRotate(camera->orientation.getValue());

  SbVec3f dst = vec;
  if (dorotate) {
    camerarot.multDirMatrix(vec, dst);
  }
  
  if (camera->isOfType(UTMCamera::getClassTypeId())) {
    SbVec3d ddst(dst[0]*dist, dst[1]*dist, dst[2]*dist);
    SbVec3d pos = utmcamera->utmposition.getValue() + ddst;
    utmcamera->utmposition = pos;
  }
  else {
    // it's ok to do this even for UTMCamera. It just updates the
    // position relative to utmposition.
    camera->position = camera->position.getValue() + dst*dist;
  }
}
