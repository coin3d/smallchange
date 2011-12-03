#ifndef SMALLCHANGE_SMCAMERACONTROLKIT_H
#define SMALLCHANGE_SMCAMERACONTROLKIT_H

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

#include <Inventor/nodekits/SoSubKit.h>
#include <Inventor/nodekits/SoBaseKit.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <SmallChange/basic.h>

class SmCameraControlKitP;
class SoCamera;
class SbMatrix;
class SbViewportRegion;
class SbRotation;
class SoEvent;

class SMALLCHANGE_DLL_API SmCameraControlKit : public SoBaseKit {
  typedef SoBaseKit inherited;

  SO_KIT_HEADER(SmCameraControlKit);

  SO_KIT_CATALOG_ENTRY_HEADER(topSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(camera);
  SO_KIT_CATALOG_ENTRY_HEADER(headlightNode);
  SO_KIT_CATALOG_ENTRY_HEADER(scene);
  
public:
  SmCameraControlKit(void);
  static void initClass(void);
  
  virtual void GLRender(SoGLRenderAction * action);
  virtual void handleEvent(SoHandleEventAction * action);

  enum AutoClippingStrategy {
    VARIABLE_NEAR_PLANE,
    CONSTANT_NEAR_PLANE
  };

  SbBool isAnimating(void);
  SbBool isBusy(void) const;
  SbBool seek(const SoEvent * event, const SbViewportRegion & vp);
  SbBool seekToPoint(const SbVec3d & point,
                     const SbRotation & orientation,
                     const SbViewportRegion & vp);
  void viewAll(const SbViewportRegion & vp, const float slack = 1.0f);
  void pointDir(const SbVec3f & dir, const SbBool resetroll = TRUE);
  void resetCameraRoll(void);

  void resetCameraFocalDistance(const SbViewportRegion & vpr);

protected:

  virtual SbBool setAnyPart(const SbName & partname, SoNode * from,
                            SbBool anypart = TRUE);
  
  virtual void notify(SoNotList * list);
  virtual ~SmCameraControlKit();
  
public:
  SoSFBool headlight;
  SoSFBool autoClipping;
  SoSFEnum autoClippingStrategy;
  SoSFFloat autoClippingValue;
  SoSFNode eventHandler;
  SoSFVec3f viewUp;
  SoSFBool handleInheritedEventFirst;

private:
  void setClippingPlanes(void);
  void getCameraCoordinateSystem(SoCamera * camera,
                                 SoNode * root,
                                 SbMatrix & matrix,
                                 SbMatrix & inverse);

  friend class SmCameraControlKitP;
  SmCameraControlKitP * pimpl;
};

#endif // SMALLCHANGE_SMCAMERACONTROLKIT_H
