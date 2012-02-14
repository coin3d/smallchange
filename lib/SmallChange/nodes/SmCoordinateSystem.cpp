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
  \class SmCoordinateSystem SmCoordinateSystem.h
  \brief The SmCoordinateSystem class is used to set the axis basis.

  FIXME: More doc.
*/

#include <SmallChange/nodes/SmCoordinateSystem.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/actions/SoGetMatrixAction.h>
#include <Inventor/nodes/SoTransformation.h>

SO_NODE_SOURCE(SmCoordinateSystem);

/*!
  Constructor.
*/
SmCoordinateSystem::SmCoordinateSystem()
{
  SO_NODE_CONSTRUCTOR(SmCoordinateSystem);

  SO_NODE_ADD_FIELD(xAxis, (SbVec3f(1.0f, 0, 0)));
  SO_NODE_ADD_FIELD(yAxis, (SbVec3f(0, 1.0f, 0)));
  SO_NODE_ADD_FIELD(zAxis, (SbVec3f(0, 0, 1.0f)));
}

/*!
  Destructor.
*/
SmCoordinateSystem::~SmCoordinateSystem()
{
}

// Documented in superclass
void
SmCoordinateSystem::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_NODE_INIT_CLASS(SmCoordinateSystem, SoTransformation, "Transformation");
  }
}

void
SmCoordinateSystem::calcMatrix(SbMatrix & m) const
{
  m.makeIdentity();

  m[0][0] = this->xAxis.getValue()[0];
  m[0][1] = this->xAxis.getValue()[1];
  m[0][2] = this->xAxis.getValue()[2];

  m[1][0] = this->yAxis.getValue()[0];
  m[1][1] = this->yAxis.getValue()[1];
  m[1][2] = this->yAxis.getValue()[2];

  m[2][0] = this->zAxis.getValue()[0];
  m[2][1] = this->zAxis.getValue()[1];
  m[2][2] = this->zAxis.getValue()[2];
}

// Doc from superclass.
void
SmCoordinateSystem::doAction(SoAction * action)
{
  SbMatrix m;
  this->calcMatrix(m);
  SoModelMatrixElement::mult(action->getState(), this, m);
}

// Doc from superclass.
void
SmCoordinateSystem::GLRender(SoGLRenderAction * action)
{
  SmCoordinateSystem::doAction((SoAction *)action);
}

// Doc from superclass.
void
SmCoordinateSystem::callback(SoCallbackAction * action)
{
  SmCoordinateSystem::doAction((SoAction *)action);
}

// Doc from superclass.
void
SmCoordinateSystem::getBoundingBox(SoGetBoundingBoxAction * action)
{
  SmCoordinateSystem::doAction((SoAction *)action);
}

// Doc from superclass.
void
SmCoordinateSystem::getMatrix(SoGetMatrixAction * action)
{
  SbMatrix m;
  this->calcMatrix(m);

  action->getMatrix().multLeft(m);
  action->getInverse().multRight(m.inverse());
}

// Doc from superclass.
void
SmCoordinateSystem::pick(SoPickAction * action)
{
  SmCoordinateSystem::doAction((SoAction *)action);
}

// Doc from superclass.
void
SmCoordinateSystem::getPrimitiveCount(SoGetPrimitiveCountAction * action)
{
  SmCoordinateSystem::doAction((SoAction *)action);
}
