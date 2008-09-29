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
  \class SmSphereEventHandler SmSphereEventHandler.h
  \brief The SmSphereEventHandler class... 
  \ingroup eventhandlers

  FIXME: doc
*/

#include "SmSphereEventHandler.h"
#include <Inventor/actions/SoHandleEventAction.h>
#include <Inventor/events/SoMouseButtonEvent.h>

SO_NODE_SOURCE(SmSphereEventHandler);

SmSphereEventHandler::SmSphereEventHandler(void)
{
  SO_NODE_CONSTRUCTOR(SmSphereEventHandler); 

  SO_NODE_ADD_FIELD(minDistance, (1.0f));
  SO_NODE_ADD_FIELD(maxDistance, (10.0f));
  this->enableButton3Movement(FALSE);
}

SmSphereEventHandler::~SmSphereEventHandler()
{
}

void
SmSphereEventHandler::initClass(void)
{
  static int initialized = 0;
  if (!initialized) {
    initialized = 1;
    SO_NODE_INIT_CLASS(SmSphereEventHandler, SmExaminerEventHandler, "SmExaminerEventHandler");
  }
}

float 
SmSphereEventHandler::clampZoom(const float val)
{
  return SbClamp(val, this->minDistance.getValue(), this->maxDistance.getValue());
}

void
SmSphereEventHandler::handleEvent(SoHandleEventAction * action)
{
  inherited::handleEvent(action);
}
