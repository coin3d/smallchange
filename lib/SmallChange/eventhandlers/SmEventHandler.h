#ifndef SMALLCHANGE_SMEVENTHANDLER_H
#define SMALLCHANGE_SMEVENTHANDLER_H

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

#include <Inventor/SbViewportRegion.h>
#include <SmallChange/nodekits/SmCameraControlKit.h>
#include <SmallChange/basic.h>
#include <Inventor/SbVec2s.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoNode.h>

class SoHandleEventAction;
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
  virtual void resetCameraFocalDistance(const SbViewportRegion & vpr);
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
