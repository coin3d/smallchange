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
  \class PickCallback PickCallback.h Inventor/nodes/PickCallback.h
  \brief The PickCallback class is group node with callbacks when some child is picked.
  \ingroup nodes

  It provides an easy way of adding interactivity to your scene graph, since
  you can insert this group node in your inventor file, and just search for
  all PickCallback nodes when your program starts to add callbacks. You
  can also connect to the trigger field if you want some other node to
  trigger when something is picked.
*/

/*!
  \var SoSFBool PickCallback::pickable
  Can be set to FALSE to temporarily disable pick callbacks. Default value is TRUE.
*/

/*!
  \var SoSFTrigger PickCallback::trigger
  Will be triggered right after callbacks are called (if any).
*/

#include "PickCallback.h"
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/sensors/SoOneShotSensor.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/engines/SoTimeCounter.h>
#include <Inventor/misc/SoChildList.h>
#include <Inventor/events/SoEvent.h>
#include <Inventor/actions/SoHandleEventAction.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/SoPickedPoint.h>

// *************************************************************************

static int (*schemescriptcb)(const char *);
static void (*schemefilecb)(const char *);

SO_NODE_SOURCE(PickCallback);

/*!
  Constructor.
*/
PickCallback::PickCallback()
{
  SO_NODE_CONSTRUCTOR(PickCallback);
  
  SO_NODE_ADD_FIELD(pickable, (FALSE));
  SO_NODE_ADD_FIELD(onMousePress, (TRUE));
  SO_NODE_ADD_FIELD(onMouseRelease, (FALSE));
  SO_NODE_ADD_FIELD(onButton1, (TRUE));
  SO_NODE_ADD_FIELD(onButton2, (FALSE));
  SO_NODE_ADD_FIELD(onButton3, (FALSE));
  SO_NODE_ADD_FIELD(schemeFile, (""));
  SO_NODE_ADD_FIELD(schemeScript, (""));
  SO_NODE_ADD_FIELD(trigger, ());

  this->pickedpoint = NULL;
  this->buttonnum = 0;
  this->curraction = NULL;
}


/*!
  Destructor.
*/
PickCallback::~PickCallback()
{
}

/*!
  Required Coin method.
*/
void
PickCallback::initClass(void)
{
  SO_NODE_INIT_CLASS(PickCallback, SoGroup, "Group");
}

/*!
  Coin method. Checks to see if some child is picked, and triggers 
  callbacks when this happens.
*/
void
PickCallback::handleEvent(SoHandleEventAction * action)
{
  this->curraction = action;

  if (!this->pickable.getValue()) {
    inherited::handleEvent(action);
    return;
  }

  const SoEvent *event = action->getEvent();
  this->event = event;

  SbBool haltaction = FALSE;

  if (this->onMousePress.getValue() &&
      this->onButton1.getValue() &&
      SO_MOUSE_PRESS_EVENT(event, BUTTON1)) {
    this->buttonnum = 1;
    haltaction = this->testPick(action, TRUE);
  }  
  else if (this->onMouseRelease.getValue() &&
           this->onButton1.getValue() &&
           SO_MOUSE_RELEASE_EVENT(event, BUTTON1)) {
    this->buttonnum = 1;
    haltaction = this->testPick(action, FALSE);    
  }
  else if (this->onMousePress.getValue() &&
      this->onButton2.getValue() &&
      SO_MOUSE_PRESS_EVENT(event, BUTTON2)) {
    this->buttonnum = 2;
    haltaction = this->testPick(action, TRUE);
  }  
  else if (this->onMouseRelease.getValue() &&
           this->onButton2.getValue() &&
           SO_MOUSE_RELEASE_EVENT(event, BUTTON2)) {
    this->buttonnum = 2;
    haltaction = this->testPick(action, FALSE);    
  }

  if (haltaction) {
    action->isHandled();
  }
  else {
    inherited::handleEvent(action);
  }
}

const SoPickedPoint * 
PickCallback::getCurrentPickedPoint(void) const
{
  return this->pickedpoint;
}

const SoEvent * 
PickCallback::getCurrentEvent(void) const
{
  return this->event;
}


SbBool 
PickCallback::isButton1(void) const
{
  return this->buttonnum == 1;
}

SbBool 
PickCallback::isButton2(void) const
{
  return this->buttonnum == 2;
}

/*!
  Adds a callback that will be called when some node is picked.
  \sa removeCallback()
*/
void 
PickCallback::addCallback(void (*callback)(void *, SoPath *), void * userdata)
{
  this->cblist.addCallback((SoCallbackListCB*) callback, userdata);
}

/*!
  Removes a callback.
  \sa addCallback()
*/
void 
PickCallback::removeCallback(void (*callback)(void *, SoPath *), void * userdata)
{
  this->cblist.removeCallback((SoCallbackListCB*)callback, userdata);
}

SbBool 
PickCallback::testPick(SoHandleEventAction * action, const SbBool mousepress)
{
  const SoPickedPoint *pp = action->getPickedPoint();
  SbString schemeFile = this->schemeFile.getValue();
  SbString schemeScript = this->schemeScript.getValue();

  if (pp) {
    SoPath * path = pp->getPath();
    if (path->containsPath(action->getCurPath())) {
      this->pickedpoint = pp;
      this->mousepress = mousepress;
      this->cblist.invokeCallbacks(path);
      if (schemeFile.getLength() > 0 && schemefilecb) 
	schemefilecb(schemeFile.getString());
      if (schemeScript.getLength() > 0 && schemescriptcb)
        schemescriptcb(schemeScript.getString());
      this->trigger.setValue();
      this->pickedpoint = NULL;
      return TRUE;
    }
  }
  return FALSE;
}

SoHandleEventAction * 
PickCallback::getCurrentAction(void) const
{
  return this->curraction;
}

SbBool 
PickCallback::currentIsMouseDown(void) const
{
  return this->event && SO_MOUSE_PRESS_EVENT(this->event, ANY);
}

void 
PickCallback::setSchemeEvalFunctions(int (*scriptcb)(const char *),
                                     void (*filecb)(const char *))
{
  schemescriptcb = scriptcb;
  schemefilecb = filecb;
}
