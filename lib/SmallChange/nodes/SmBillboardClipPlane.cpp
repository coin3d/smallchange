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
