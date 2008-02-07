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

/*!
  \var SoSFVec3f::objectSpacePickedPoint
  Will be set before callbacks are called (if any)
*/

/*!
  \var SoSFVec3f::worldSpacePickedPoint
  Will be set before callbacks are called (if any)
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
#include <Inventor/sensors/SoOneShotSensor.h>
#include <Inventor/elements/SoViewportRegionElement.h>

// *************************************************************************





static int (*schemescriptcb)(const char *);
static void (*schemefilecb)(const char *);


class pc_sensordata {
public:
  PickCallback * nodeptr;
  SoPath * path;
  SbVec2s eventpos;
  SbViewportRegion viewport;
  SbBool mousepress;
  int buttonnum;
  SoPickedPoint * pickedpoint;

  static void sensorCB(void * userdata, SoSensor * sensor) {
    pc_sensordata * thisp = (pc_sensordata*) userdata;
    thisp->nodeptr->current = thisp;

//     fprintf(stderr,"callback: %d %d (%d %d)\n",
//             thisp->mousepress, thisp->buttonnum, thisp->eventpos[0], thisp->eventpos[1]);


    SbString schemeFile = thisp->nodeptr->schemeFile.getValue();
    SbString schemeScript = thisp->nodeptr->schemeScript.getValue();

    thisp->nodeptr->cblist.invokeCallbacks(thisp->path);
    if (schemeFile.getLength() > 0 && schemefilecb)
      schemefilecb(schemeFile.getString());
    if (schemeScript.getLength() > 0 && schemescriptcb)
      schemescriptcb(schemeScript.getString());
    thisp->nodeptr->trigger.setValue();

    delete thisp->pickedpoint;
    thisp->nodeptr->current = NULL;
    delete sensor;
    thisp->path->unref();
    delete thisp;
  }
};

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
  SO_NODE_ADD_FIELD(objectSpacePickedPoint, (0.0f, 0.0f, 0.0f));
  SO_NODE_ADD_FIELD(worldSpacePickedPoint, (0.0f, 0.0f, 0.0f));
  SO_NODE_ADD_FIELD(delayTrigger, (TRUE));

  this->pickedpoint = NULL;
  this->buttonnum = 0;
  this->current = NULL;
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
  if (!this->pickable.getValue()) {
    inherited::handleEvent(action);
    return;
  }

  const SoEvent *event = action->getEvent();

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
  if (this->current) {
    return this->current->pickedpoint;
  }
  return this->pickedpoint;
}

SbBool
PickCallback::isButton1(void) const
{
  if (this->current) {
    return this->current->buttonnum == 1;
  }
  return this->buttonnum == 1;
}

SbBool
PickCallback::isButton2(void) const
{
  if (this->current) {
    return this->current->buttonnum == 2;
  }
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
      this->objectSpacePickedPoint = pp->getObjectPoint();
      this->worldSpacePickedPoint = pp->getPoint();
      this->mousepress = mousepress;
      this->eventpos = action->getEvent()->getPosition();
      this->viewport = SoViewportRegionElement::get(action->getState());

      if (this->delayTrigger.getValue()) {

        pc_sensordata * data = new pc_sensordata;
        data->path = path->copy();
        data->path->ref();
        data->nodeptr = this;
        data->eventpos = this->eventpos;
        data->viewport = this->viewport;
        data->mousepress = this->mousepress;
        data->buttonnum = this->buttonnum;
        data->pickedpoint = new SoPickedPoint(*pp);
        SoOneShotSensor * sensor = new SoOneShotSensor(pc_sensordata::sensorCB, data);

//         fprintf(stderr,"schedule: %d %d (%d %d)\n",
//                 data->mousepress, data->buttonnum, data->eventpos[0], data->eventpos[1]);

        sensor->setPriority(mousepress ? 1 : 2);
        sensor->schedule();
        return TRUE;
      }
      else {
        this->pickedpoint = new SoPickedPoint(*pp);
        this->cblist.invokeCallbacks(path);
        if (schemeFile.getLength() > 0 && schemefilecb)
          schemefilecb(schemeFile.getString());
        if (schemeScript.getLength() > 0 && schemescriptcb)
          schemescriptcb(schemeScript.getString());
        this->trigger.setValue();
        delete this->pickedpoint;
        this->pickedpoint = NULL;
        return TRUE;
      }
    }
  }
  return FALSE;
}

SbBool
PickCallback::currentIsMouseDown(void) const
{
  if (this->current) {
    return this->current->mousepress;
  }
  return this->mousepress;
}

SbVec2s
PickCallback::getEventPosition(void) const
{
  if (this->current) {
    return this->current->eventpos;
  }
  return this->eventpos;
}

const SbViewportRegion &
PickCallback::getEventViewportRegion(void) const
{
  if (this->current) {
    return this->current->viewport;
  }
  return this->viewport;
}


void
PickCallback::setSchemeEvalFunctions(int (*scriptcb)(const char *),
                                     void (*filecb)(const char *))
{
  schemescriptcb = scriptcb;
  schemefilecb = filecb;
}
