#ifndef SMALLCHANGE_SMPOPUPMENUKIT_H
#define SMALLCHANGE_SMPOPUPMENUKIT_H

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

#include <Inventor/nodekits/SoSubKit.h>
#include <Inventor/nodekits/SoBaseKit.h>
#include <Inventor/fields/SoMFNode.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFTime.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFVec2s.h>
#include <Inventor/fields/SoMFString.h>
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
  SO_KIT_CATALOG_ENTRY_HEADER(textColor);
  SO_KIT_CATALOG_ENTRY_HEADER(textPickStyle);
  SO_KIT_CATALOG_ENTRY_HEADER(textShape);
  SO_KIT_CATALOG_ENTRY_HEADER(activeMaterial);
  SO_KIT_CATALOG_ENTRY_HEADER(activeShape);
  SO_KIT_CATALOG_ENTRY_HEADER(borderShape);

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
  
  void setPickedPoint(const SoPickedPoint * pp, const SbViewportRegion & vp);
  void setNormalizedPosition(const SbVec2f & npt);

  void setViewportRegion(const SbViewportRegion & vp);

  SoMFString itemList;
  SoMFNode itemData;
  SoMFString itemSchemeScript;
  
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

