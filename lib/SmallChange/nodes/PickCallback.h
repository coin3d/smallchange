#ifndef PICKCALLBACK_H
#define PICKCALLBACK_H

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

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFTrigger.h>
#include <Inventor/fields/SoSFString.h>
#include <Inventor/lists/SoCallbackList.h>

class SoPath;
class SoPickedPoint;
class SoEvent;

class PickCallback : public SoGroup {
  typedef SoGroup inherited;

  SO_NODE_HEADER(PickCallback);

public:
  static void initClass(void);
  PickCallback(void);
  
  SoSFBool pickable;
  SoSFTrigger trigger;
  SoSFBool onMousePress;
  SoSFBool onMouseRelease;
  SoSFBool onButton1;
  SoSFBool onButton2;
  SoSFBool onButton3;
  SoSFString schemeFile;
  SoSFString schemeScript;
  
  const SoPickedPoint * getCurrentPickedPoint(void) const;
  const SoEvent * getCurrentEvent(void) const;
  SoHandleEventAction * getCurrentAction(void) const;
  SbBool currentIsMouseDown(void) const;
  SbBool isButton1(void) const;
  SbBool isButton2(void) const;
  
  static void setSchemeEvalFunctions(int (*scriptcb)(const char *),
                                     void (*filecb)(const char *));
  
  void addCallback(void (*callback)(void *, SoPath *), void * userdata);
  void removeCallback(void (*callback)(void *, SoPath *), void * userdata);
  
protected:
  virtual ~PickCallback();
  virtual void handleEvent(SoHandleEventAction * action);

private:
  SbBool testPick(SoHandleEventAction * action, const SbBool mousepress);
  SoCallbackList cblist;
  const SoPickedPoint * pickedpoint;
  const SoEvent * event;
  SoHandleEventAction * curraction;
  SbBool mousepress;
  int buttonnum;
};

#endif // !PICKCALLBACK_H
