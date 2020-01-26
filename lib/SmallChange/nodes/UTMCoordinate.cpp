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
  \class UTMCoordinate UTMCoordinate.h Inventor/nodes/UTMCoordinate.h
  \brief The UTMCoordinate class is a node for providing coordinates to shape nodes.

  \ingroup nodes

  When encountering a node of this type during traversal, the
  coordinates it contains will be put on the state stack for later use
  by shape nodes of types which needs coordinate sets (like SoFaceSet
  nodes or SoPointSet nodes). If your scene graph contains an
  UTMCamera node, the coordinates will be adjusted for this. This is
  just a convenience node which makes it possible to specify shapes
  directly in UTMCoordinates. It should only be used for quite simple
  and static geometry, as transformation nodes will have no effect on
  this node. To specify more complex geometry, you're better off
  using a UTMPosition node and specify the coordinates relative to
  the position using an SoCoordinate3 node.

  Note that an UTMCoordinate node will \e replace the coordinates
  already present in the state (if any).

  \sa UTMCamera, UTMPosition
*/

#include "UTMCoordinate.h"
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoGetPrimitiveCountAction.h>
#include <Inventor/actions/SoPickAction.h>
#include <Inventor/elements/SoGLCoordinateElement.h>
#include <Inventor/misc/SoState.h>
#include <SmallChange/elements/UTMElement.h>

/*!
  \var SoMFVec3f UTMCoordinate::point
  Coordinate set of 3D points. Currently only single precision, but will be
  double in the future.
*/


// *************************************************************************

SO_NODE_SOURCE(UTMCoordinate);

/*!
  Constructor.
*/
UTMCoordinate::UTMCoordinate(void)
{
  SO_NODE_CONSTRUCTOR(UTMCoordinate);

  SO_NODE_ADD_FIELD(point, (0.0f, 0.0f, 0.0f));
  this->dirty = TRUE;
}

/*!
  Destructor.
*/
UTMCoordinate::~UTMCoordinate()
{
}

/*!
  Required Coin method.
*/
void
UTMCoordinate::initClass(void)
{
  SO_NODE_INIT_CLASS(UTMCoordinate, SoNode, "Node");
}

/*!
  Coin method.
*/
void
UTMCoordinate::doAction(SoAction * action)
{
  this->updateCoords(action->getState());
  SoCoordinateElement::set3(action->getState(), this,
                            this->coords.getLength(), this->coords.getArrayPtr());
}


/*!
  Coin method.
*/
void
UTMCoordinate::GLRender(SoGLRenderAction * action)
{
  UTMCoordinate::doAction(action);
}


/*!
  Coin method.
*/
void
UTMCoordinate::callback(SoCallbackAction * action)
{
  UTMCoordinate::doAction(action);
}


/*!
  Coin method.
*/
void
UTMCoordinate::pick(SoPickAction * action)
{
  UTMCoordinate::doAction(action);
}


/*!
  Coin method.
*/
void
UTMCoordinate::getBoundingBox(SoGetBoundingBoxAction * action)
{
  UTMCoordinate::doAction(action);
}


/*!
  Coin method.
*/
void
UTMCoordinate::getPrimitiveCount(SoGetPrimitiveCountAction * action)
{
  UTMCoordinate::doAction(action);
}

/*!
  Overloaded to invalidate cache.
*/
void
UTMCoordinate::notify(SoNotList * nl)
{
  this->dirty = TRUE;
  inherited::notify(nl);
}

/*!
  Recalculates cache.
*/
void
UTMCoordinate::updateCoords(SoState * state)
{
  double pos[3];
  UTMElement::getReferencePosition(state, pos[0], pos[1], pos[2]);
  SbVec3f newtrans = UTMElement::getCurrentTranslation(state);
  if (this->dirty ||
      pos[0] != this->refpos[0] ||
      pos[1] != this->refpos[1] ||
      pos[2] != this->refpos[2] ||
      newtrans != this->trans) {
    int n = this->point.getNum();
    const SbVec3f * ptr = this->point.getValues(0);

    float posf[3];
    posf[0] = float(pos[0]) - newtrans[0];
    posf[1] = float(pos[1]) - newtrans[1];
    posf[2] = float(pos[2]) - newtrans[2];
    this->coords.truncate(0);

    for (int i = 0; i < n; i++) {
      this->coords.append(SbVec3f(ptr[i][0] - posf[0],
                                  ptr[i][1] - posf[1],
                                  ptr[i][2] - posf[2]));
    }
    this->dirty = FALSE;
    this->refpos[0] = pos[0];
    this->refpos[1] = pos[1];
    this->refpos[2] = pos[2];
    this->trans = newtrans;
  }
}
