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
  \class PickSwitch PickSwitch.h Inventor/nodes/PickSwitch.h
  \brief The PickSwitch class is a pick-activated switch node.
  \ingroup nodes

  This node can add simple interactivity to your scene. When the current
  child is picked, the whichChild field is changed to the active
  ot inactive child. It can operate in two modes. If the toggle field
  is set to TRUE, the active/inactive state is changed each time
  the child is picked. If toggle is FALSE, the active child is
  the current child only when the some child is selected and the mouse
  button is down. As soon as the mouse button is released, the switch
  node becomes inactive again.
*/

/*!
  \var SoSFBool PickSwitch::isActive
  This field is set to TRUE when the switch node is active (selected).
  When this happens depends on the toggle field.
*/

/*!
  \var SoSFInt32 PickSwitch::activeChild
  Specifies which child should be traversed when node is active.
*/

/*!
  \var SoSFInt32 PickSwitch::inactiveChild
  Specifies which child should be traversed when node is inactive.
*/

/*!
  \var SoSFBool PickSwitch::toggle
  Set to TRUE to toggle active/inactive for each pick. When this field is
  FALSE, the node is active only while some child is selected and the mouse
  button is down.
*/

#include "PickSwitch.h"
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

SO_NODE_SOURCE(PickSwitch);

/*!
  Constructor.
*/
PickSwitch::PickSwitch()
{
  SO_NODE_CONSTRUCTOR(PickSwitch);

  SO_NODE_ADD_FIELD(isActive, (FALSE));
  SO_NODE_ADD_FIELD(activeChild, (-1));
  SO_NODE_ADD_FIELD(inactiveChild, (-1));
  SO_NODE_ADD_FIELD(toggle, (FALSE));
}

/*!
  Destructor.
*/
PickSwitch::~PickSwitch()
{
}

/*!
  Required Coin method.
*/
void
PickSwitch::initClass(void)
{
  SO_NODE_INIT_CLASS(PickSwitch, SoSwitch, "Switch");
}

/*!
  Coin method. Sets node active/inactive based on user interaction.
*/
void
PickSwitch::handleEvent(SoHandleEventAction *action)
{
  const SoEvent *event = action->getEvent();

  SbBool haltaction = FALSE;
  if (SO_MOUSE_PRESS_EVENT(event, BUTTON1)) {
    const SoPickedPoint *pp = action->getPickedPoint();
    if (pp) {
      SoPath *path = pp->getPath();
      if (path->containsPath(action->getCurPath())) {
        this->isActive = this->toggle.getValue() ? !this->isActive.getValue() : TRUE;
        this->whichChild = this->isActive.getValue() ?
          this->activeChild.getValue() :
          this->inactiveChild.getValue();
        haltaction = TRUE;
      }
    }
  }
  else if (SO_MOUSE_RELEASE_EVENT(event, BUTTON1) && this->isActive.getValue()) {
    if (!this->toggle.getValue()) {
      this->whichChild = this->inactiveChild.getValue();
      this->isActive = FALSE;
      haltaction = TRUE;
    }
  }

  if (haltaction) {
    action->isHandled();
  }
  else {
    inherited::handleEvent(action);
  }
}

