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
  \class SmSwitchboard SmSwitchboard.h SmallChange/nodes/SmSwitchboard.h
  \brief The SmSwitchboard class is a group node that can toggle children
  on and off arbitrarily.

  FIXME: write doc

  \ingroup nodes
*/

// FIXME: implement proper searching / SearchAction handling  2002-02-07 larsa
// FIXME: implement proper writing / WriteAction handling  2002-02-07 larsa

#include <SmallChange/nodes/SmSwitchboard.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/misc/SoChildList.h>

#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoSearchAction.h>
#include <Inventor/actions/SoGetMatrixAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoPickAction.h>
#include <Inventor/actions/SoHandleEventAction.h>
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoGetPrimitiveCountAction.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/SoOutput.h>

#include <Inventor/errors/SoDebugError.h>

/*!
  \var SoMFBool SmSwitchboard::enable

  Selects which child to traverse during rendering (and some other)
  actions.

  When the length of this multifield is larger than the number of children
  this group has, the enable list is modulated over the children.  This lets
  you have full control over the number of times and order each child is
  traversed.

  Default enabled value is \c FALSE.
*/

// *************************************************************************

// doc in super
void
SmSwitchboard::initClass(void)
{
  SO_NODE_INIT_CLASS(SmSwitchboard, SoGroup, SoGroup);
}

SO_NODE_SOURCE(SmSwitchboard);

/*!
  Default constructor.
*/
SmSwitchboard::SmSwitchboard(void)
{
  SO_NODE_CONSTRUCTOR(SmSwitchboard);

  SO_NODE_ADD_FIELD(enable, (FALSE));
}

/*!
  Constructor.

  The argument should be the approximate number of children which is
  expected to be inserted below this node. The number need not be
  exact, as it is only used as a hint for better memory resource
  allocation.
*/
SmSwitchboard::SmSwitchboard(int numchildren)
  : inherited(numchildren)
{
  SO_NODE_CONSTRUCTOR(SmSwitchboard);

  SO_NODE_ADD_FIELD(enable, (FALSE));
}

/*!
  Destructor.
*/
SmSwitchboard::~SmSwitchboard(void) // virtual, protected
{
}

// Documented in superclass.
void
SmSwitchboard::doAction(SoAction * action)
{
  // FIXME: take PathCode and stuff into consideration...
  if (action->isOfType(SoGetBoundingBoxAction::getClassTypeId())) {
    // calculate center of bbox if bboxaction. This makes the
    // switchboard node behave exactly like a group node
    SoGetBoundingBoxAction * bbaction = (SoGetBoundingBoxAction*) action;
    // Initialize accumulation variables.
    SbVec3f acccenter(0.0f, 0.0f, 0.0f);
    int numcenters = 0;
    for (int idx = 0; idx < this->enable.getNum(); idx++) {
      const int numchildren = this->children->getLength();
      if ( numchildren > 0 )
        action->traverse((*this->children)[idx % numchildren]);
      // If center point is set, accumulate.
      if (bbaction->isCenterSet()) {
        acccenter += bbaction->getCenter();
        numcenters++;
        bbaction->resetCenter();
      }
    }
    if (numcenters != 0) {
      bbaction->setCenter(acccenter / float(numcenters), FALSE);
    }
  } else { // not a GetBoundingBoxAction
    for ( int idx = 0; idx < this->enable.getNum(); idx++ ) {
      if ( this->enable[idx] ) {
        const int numchildren = this->children->getLength();
        if ( numchildren > 0 )
          action->traverse((*this->children)[idx % numchildren]);
      }
    }
  }
}

void
SmSwitchboard::GLRender(SoGLRenderAction * action)
{
  SmSwitchboard::doAction((SoAction *) action);
}

void
SmSwitchboard::getBoundingBox(SoGetBoundingBoxAction * action)
{
  SmSwitchboard::doAction((SoAction *) action);
}

void
SmSwitchboard::getMatrix(SoGetMatrixAction * action)
{
  switch (action->getCurPathCode()) {
  case SoAction::OFF_PATH:
  case SoAction::IN_PATH:
    SmSwitchboard::doAction((SoAction *) action);
    break;
  default:
    break;
  }
}

void
SmSwitchboard::callback(SoCallbackAction *action)
{
  SmSwitchboard::doAction(action);
}

// Documented in superclass.
void
SmSwitchboard::pick(SoPickAction *action)
{
  SmSwitchboard::doAction((SoAction*)action);
}

// Documented in superclass.
void
SmSwitchboard::handleEvent(SoHandleEventAction *action)
{
  SmSwitchboard::doAction(action);
}

void
SmSwitchboard::search(SoSearchAction * action)
{
  SoNode::search(action);
  if (action->isFound()) return;
  SmSwitchboard::doAction(action);
}
