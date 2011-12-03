#ifndef SMALLCHANGE_SMTOOLTIPKIT_H
#define SMALLCHANGE_SMTOOLTIPKIT_H

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
#include <Inventor/fields/SoSFTime.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoSFVec2s.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/SbVec2f.h>
#include <SmallChange/basic.h>

class SmTooltipKitP;
class SoSensor;
class SoPickedPoint;
class SbViewportRegion;

class SMALLCHANGE_DLL_API SmTooltipKit : public SoBaseKit {
  typedef SoBaseKit inherited;

  SO_KIT_HEADER(SmTooltipKit);

  SO_KIT_CATALOG_ENTRY_HEADER(topSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(resetTransform);
  SO_KIT_CATALOG_ENTRY_HEADER(position);
  SO_KIT_CATALOG_ENTRY_HEADER(depthBuffer);
  SO_KIT_CATALOG_ENTRY_HEADER(lightModel);
  SO_KIT_CATALOG_ENTRY_HEADER(camera);
  SO_KIT_CATALOG_ENTRY_HEADER(texture);
  SO_KIT_CATALOG_ENTRY_HEADER(shapeHints);
  SO_KIT_CATALOG_ENTRY_HEADER(pickStyle);
  SO_KIT_CATALOG_ENTRY_HEADER(materialBinding);
  SO_KIT_CATALOG_ENTRY_HEADER(backgroundColor);
  SO_KIT_CATALOG_ENTRY_HEADER(justification);
  SO_KIT_CATALOG_ENTRY_HEADER(backgroundShape);
  SO_KIT_CATALOG_ENTRY_HEADER(textColor);
  SO_KIT_CATALOG_ENTRY_HEADER(textShape);
  
public:
  SmTooltipKit(void);
  static void initClass(void);

  virtual void GLRender(SoGLRenderAction * action);
  virtual void handleEvent(SoHandleEventAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  virtual void search(SoSearchAction * action);
  virtual void callback(SoCallbackAction * action);
  virtual void getMatrix(SoGetMatrixAction * action);
  virtual void pick(SoPickAction * action);
  virtual void rayPick(SoRayPickAction * action);
  virtual void audioRender(SoAudioRenderAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);

protected:
  virtual ~SmTooltipKit();
  virtual SbBool affectsState(void) const;
  
public:
  
  void setPickedPoint(const SoPickedPoint * pp, const SbViewportRegion & vp);
  void setPickedPosition(const SbVec2f & pos, const SbViewportRegion & vp);
  void setViewportRegion(const SbViewportRegion & vp);

  SoSFBool autoTrigger;
  SoSFTime autoTriggerTime;
  SoSFBool isActive;
  SoMFString description;
  SoSFInt32 frameSize;
  SoSFVec2s offset;

private:

  void updateBackground(void);

  static void tooltip_changed_cb(void * closure, SoSensor * s);
  static void alarm_cb(void * closure, SoSensor * s);
  friend class SmTooltipKitP;
  SmTooltipKitP * pimpl;
};


#endif // SMALLCHANGE_SMTOOLTIPKIT_H
