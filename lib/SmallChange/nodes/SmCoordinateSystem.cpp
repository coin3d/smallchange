/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2004 by Systems in Motion.  All rights reserved.
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
  \class SmCoordinateSystem SmCoordinateSystem.h
  \brief The SmCoordinateSystem class is used to set the axis basis.

  FIXME: More doc.
*/

#include <SmallChange/nodes/SmCoordinateSystem.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/actions/SoGetMatrixAction.h>
#include <Inventor/nodes/SoTransformation.h>

#define PRIVATE(obj) (obj)->pimpl
#define PUBLIC(obj) (obj)->master

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

// Doc from superclass.
void
SmCoordinateSystem::doAction(SoAction * action)
{

  SbMatrix newmatrix;
  newmatrix.makeIdentity();
  
  newmatrix[0][0] = this->xAxis.getValue()[0];
  newmatrix[0][1] = this->xAxis.getValue()[1];
  newmatrix[0][2] = this->xAxis.getValue()[2];

  newmatrix[1][0] = this->yAxis.getValue()[0];
  newmatrix[1][1] = this->yAxis.getValue()[1];
  newmatrix[1][2] = this->yAxis.getValue()[2];

  newmatrix[2][0] = this->zAxis.getValue()[0];
  newmatrix[2][1] = this->zAxis.getValue()[1];
  newmatrix[2][2] = this->zAxis.getValue()[2];
  
  SoModelMatrixElement::mult(action->getState(), this, newmatrix);

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
  SmCoordinateSystem::doAction((SoAction *)action);
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

#undef PRIVATE
