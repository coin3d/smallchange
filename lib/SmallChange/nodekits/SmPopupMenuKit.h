#ifndef SMALLCHANGE_SMPOPUPMENUKIT_H
#define SMALLCHANGE_SMPOPUPMENUKIT_H

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
#include <Inventor/fields/SoMFNode.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFTime.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFVec2s.h>
#include <Inventor/fields/SoSFString.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/fields/SoMFBool.h>
#include <SmallChange/basic.h>

class SmPopupMenuKitP;
class SoSensor;
class SoPickedPoint;
class SbViewportRegion;
class SbVec2f;

class SMALLCHANGE_DLL_API SmPopupMenuKit : public SoBaseKit {
  typedef SoBaseKit inherited;

  SO_KIT_HEADER(SmPopupMenuKit);

  SO_KIT_CATALOG_ENTRY_HEADER(topSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(resetTransform);
  SO_KIT_CATALOG_ENTRY_HEADER(transparencyType);
  SO_KIT_CATALOG_ENTRY_HEADER(position);
  SO_KIT_CATALOG_ENTRY_HEADER(depthBuffer);
  SO_KIT_CATALOG_ENTRY_HEADER(lightModel);
  SO_KIT_CATALOG_ENTRY_HEADER(camera);
  SO_KIT_CATALOG_ENTRY_HEADER(texture);
  SO_KIT_CATALOG_ENTRY_HEADER(shapeHints);
  SO_KIT_CATALOG_ENTRY_HEADER(pickStyle);
  SO_KIT_CATALOG_ENTRY_HEADER(materialBinding);
  SO_KIT_CATALOG_ENTRY_HEADER(backgroundMaterial);
  SO_KIT_CATALOG_ENTRY_HEADER(backgroundTexture);
  SO_KIT_CATALOG_ENTRY_HEADER(justification);
  SO_KIT_CATALOG_ENTRY_HEADER(backgroundShape);
  SO_KIT_CATALOG_ENTRY_HEADER(textSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(textFont);
  SO_KIT_CATALOG_ENTRY_HEADER(textColor);
  SO_KIT_CATALOG_ENTRY_HEADER(textPickStyle);
  SO_KIT_CATALOG_ENTRY_HEADER(itemSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(activeMaterial);
  SO_KIT_CATALOG_ENTRY_HEADER(activeShape);
  SO_KIT_CATALOG_ENTRY_HEADER(borderShape);
  SO_KIT_CATALOG_ENTRY_HEADER(titleSeparator);


public:
  SmPopupMenuKit(void);
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

  static void setSchemeEvalFunctions(int (*scriptcb)(const char *),
                                     void (*filecb)(const char *));
  
protected:
  virtual ~SmPopupMenuKit();
  virtual SbBool affectsState(void) const;
  
public:

  void setTransparencies(float active, float inactive);
  void setPickedPoint(const SoPickedPoint * pp, const SbViewportRegion & vp);
  void setNormalizedPosition(const SbVec2f & npt);

  void setViewportRegion(const SbViewportRegion & vp);

  SoMFString itemList;
  SoMFNode itemData;
  SoMFString itemSchemeScript;
  SoMFBool itemTagged;
  SoMFBool itemDisabled;
  SoSFString menuTitle;
  SoSFBool visible;
  SoSFBool isActive;

  // signal
  SoSFInt32 pickedItem;

  SoSFInt32 frameSize;
  SoSFVec2s offset;
  SoSFFloat spacing;
  SoSFBool closeParent;

  void setParent(SmPopupMenuKit * kit);
  void childFinished(SmPopupMenuKit * child);
  
private:
  void updateBackground(void);
  void updateActiveItem(void);
  void itemPicked(const int idx);

  static void items_changed_cb(void * closure, SoSensor * s);
  static void oneshot_cb(void * closure, SoSensor * s);
  static void trigger_cb(void * closure, SoSensor * s);
  static void activeitemchanged_cb(void * closure, SoSensor * s);
  static void opensub_cb(void * closure, SoSensor * s);
  static void isactive_cb(void * closure, SoSensor * s);
  friend class SmPopupMenuKitP;
  SmPopupMenuKitP * pimpl;
};


#endif // SMALLCHANGE_SMPOPUPMENUKIT_H
