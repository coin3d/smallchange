#ifndef SMALLCHANGE_SMEVENTHANDLER_H
#define SMALLCHANGE_SMEVENTHANDLER_H

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

#include <Inventor/SbViewportRegion.h>
#include <SmallChange/nodekits/SmCameraControlKit.h>
#include <SmallChange/basic.h>
#include <Inventor/SbVec2s.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoNode.h>

class SoHandleEventAction;
class SmCameraControlKit;
class SoCamera;
class SoSensor;
class SoTimerSensor;
class SbVec3f;
class SoGLRenderAction;

class SMALLCHANGE_DLL_API SmEventHandler : public SoNode {
  typedef SoNode inherited;

  SO_NODE_ABSTRACT_HEADER(SmEventHandler);  
public:
  static void initClass(void);
  
  // FIXME: get rid of this method, pederb 2003-09-30
  void setCameraControlKit(SmCameraControlKit * kit);
  void setViewportRegion(const SbViewportRegion & vp);
  const SbViewportRegion & getViewportRegion(void) const;
  float getGLAspectRatio(void) const;
  SbVec2s getGLSize(void) const;

  virtual void preRender(SoGLRenderAction * action);
  virtual void pulse(void);
  virtual SbBool isAnimating(void);
  void enablePulse(const SbBool onoff);

protected:
  SmEventHandler(void);
  virtual ~SmEventHandler();

  SbVec3f getViewUp(void) const;

  SbBool isAnimationEnabled(void) const;
  void interactiveCountInc(void);
  void interactiveCountDec(void);

  SoCamera * getCamera(void);

  void yawCamera(const float rad);
  void rollCamera(const float rad);
  void pitchCamera(const float rad);

  SmCameraControlKit * kit;
  SoTimerSensor * pulser;
private:
  static void pulse_cb(void * closure, SoSensor * s);
  
  SbBool pulseenabled;
  int intcnt;
  SbViewportRegion vp;
};

#endif // SMALLCHANGE_SMEVENTHANDLER_H
