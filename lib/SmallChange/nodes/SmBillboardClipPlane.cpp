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

#include "SmBillboardClipPlane.h"
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoPickAction.h>
#include <Inventor/elements/SoGLClipPlaneElement.h>
#include <Inventor/elements/SoCullElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoViewVolumeElement.h>

SO_NODE_SOURCE(SmBillboardClipPlane);

/*!
  Constructor.
*/
SmBillboardClipPlane::SmBillboardClipPlane(void)
{
  SO_NODE_CONSTRUCTOR(SmBillboardClipPlane);
  SO_NODE_ADD_FIELD(on, (TRUE));
}

/*!
  Destructor.
*/
SmBillboardClipPlane::~SmBillboardClipPlane()
{
}

// Doc from superclass.
void
SmBillboardClipPlane::initClass(void)
{
  static int initialized = 0;
  if (!initialized) {
    initialized = 1;
    SO_NODE_INIT_CLASS(SmBillboardClipPlane, SoNode, "Node");
  }
}

// Doc from superclass.
void
SmBillboardClipPlane::doAction(SoAction * action)
{
  if (this->on.isIgnored() || this->on.getValue()) {
    SbPlane plane = SmBillboardClipPlane::calcPlane(action->getState());
    SoClipPlaneElement::add(action->getState(), this, plane);
  }
}

// Doc from superclass.
void
SmBillboardClipPlane::GLRender(SoGLRenderAction * action)
{
  if (this->on.isIgnored() || this->on.getValue()) {
    SbPlane p = SmBillboardClipPlane::calcPlane(action->getState());
    SoClipPlaneElement::add(action->getState(), this, p);
    p.transform(SoModelMatrixElement::get(action->getState()));
    // update cull element as well (so Coin can cull away stuff)
    SoCullElement::addPlane(action->getState(), p);
  }
}

// Doc from superclass.
void
SmBillboardClipPlane::callback(SoCallbackAction * action)
{
  SmBillboardClipPlane::doAction(action);
}

// Doc from superclass.
void
SmBillboardClipPlane::pick(SoPickAction * action)
{
  SmBillboardClipPlane::doAction(action);
}

SbPlane
SmBillboardClipPlane::calcPlane(SoState * state)
{
  const SbViewVolume & vv = SoViewVolumeElement::get(state);
  SbVec3f dir = - vv.getProjectionDirection();
  return SbPlane(dir, 0.0f);
}
