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

/*!
  \class PickSwitch PickSwitch.h Inventor/nodes/PickSwitch.h
  \brief The PickSwitch class is a pick-activated switch node.

  \ingroup nodes

  This node can add simple interactivity to your scene. When the current
  child is picked, the whichChild field is changed to the active
  or inactive child. It can operate in two modes. If the toggle field
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
