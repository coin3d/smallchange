/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2002 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.  See the
 *  file LICENSE.GPL at the root directory of this source distribution
 *  for more details.
 *
 *  If you desire to use SmallChange with software that is incompatible
 *  licensewise with the GPL, and / or you would like to take
 *  advantage of the additional benefits with regard to our support
 *  services, please contact Systems in Motion about acquiring a Coin
 *  Professional Edition License.  See <URL:http://www.coin3d.org> for
 *  more information.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  <URL:http://www.sim.no>, <mailto:support@sim.no>
 *
\**************************************************************************/

/*!
  \class UTMCamera UTMCamera.h
  \brief The UTMCamera class defines a camera node with perspective rendering and an UTM position.
  \ingroup nodes

  This node adds three fields to the perspective camera node: easting,
  northing and elevation. These fields are defined as strings, to make
  it possible to specify positions with maximum resolution, not
  limited by the floating point precision. The position vector moves
  the camera relative to the UTM position. Use this camera instead of
  a normal PerspectiveCamera if you plan to operate with large
  floating point positions that might destroy the floating point
  precision. This is typically useful if you want to place object
  using, for instance, UTM coordinates.

  \sa UTMPosition, UTMCoordinate 
*/

#include "UTMCamera.h"
#include <Inventor/actions/SoCallbackAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoAudioRenderAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/actions/SoGetPrimitiveCountAction.h>
#include <Inventor/actions/SoHandleEventAction.h>
#include <Inventor/actions/SoRayPickAction.h>
#include <Inventor/actions/SoGetMatrixAction.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/elements/SoCacheElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/misc/SoState.h>
#include <SmallChange/elements/UTMElement.h>

/*!
  \var SoSFString UTMCamera::easting
  
  The easting (+X) position of the camera.
*/

/*!
  \var SoSFString UTMCamera::northing
  
  The northing (+Y) position of the camera.
*/

/*!
  \var SoSFString UTMCamera::elevation
  
  The elevation (+X) position of the camera.
*/

// *************************************************************************

SO_NODE_SOURCE(UTMCamera);

/*!
  Constructor.
*/
UTMCamera::UTMCamera()
{
  SO_NODE_CONSTRUCTOR(UTMCamera);

  SO_NODE_ADD_FIELD(utmposition, (0.0, 0.0, 0.0)); 
  SO_NODE_ADD_FIELD(easting, ("0.0"));
  SO_NODE_ADD_FIELD(northing, ("0.0"));
  SO_NODE_ADD_FIELD(elevation, ("0.0"));

  // hackish field to move transform in front of the camera to after
  // an UTMPosition node.
  SO_NODE_ADD_FIELD(moveTransform, (FALSE)); 
}

/*!
  Destructor.
*/
UTMCamera::~UTMCamera()
{
}

/*!
  Required Coin method.
*/
void
UTMCamera::initClass(void)
{
  SO_NODE_INIT_CLASS(UTMCamera, SoPerspectiveCamera, "PerspectiveCamera");
}

/*!
  Coin method.
*/
void 
UTMCamera::callback(SoCallbackAction * action)
{
  this->setReferencePosition(action->getState());
  SoCamera::callback(action);
}

/*!
  Coin method.
*/
void 
UTMCamera::GLRender(SoGLRenderAction * action)
{
  SoCacheElement::invalidate(action->getState());
  this->setReferencePosition(action->getState());
  SoCamera::GLRender(action);
}

/*!
  Coin method.
*/
void 
UTMCamera::audioRender(SoAudioRenderAction * action)
{
  this->setReferencePosition(action->getState());
  SoCamera::audioRender(action);
}

/*!
  Coin method.
*/
void 
UTMCamera::getBoundingBox(SoGetBoundingBoxAction * action)
{
  this->setReferencePosition(action->getState());
  SoCamera::getBoundingBox(action);
}

/*!
  Coin method.
*/
void 
UTMCamera::handleEvent(SoHandleEventAction * action)
{
  SoCamera::handleEvent(action);
}

/*!
  Coin method.
*/
void 
UTMCamera::rayPick(SoRayPickAction * action)
{
  this->setReferencePosition(action->getState());
  SoCamera::rayPick(action);
}

/*!
  Coin method.
*/
void 
UTMCamera::getPrimitiveCount(SoGetPrimitiveCountAction * action)
{
  this->setReferencePosition(action->getState());
  SoCamera::getPrimitiveCount(action);
}

/*!
  Returns the current position of the camera. The value of the position
  field will be added to each respective value before returning. 
*/
void 
UTMCamera::getPosition(double & easting, double & northing, double & elevation)
{
  SbVec3d utm = this->utmposition.getValue();
  easting = utm[0];
  northing = utm[1];
  elevation = utm[2];
}

/*!
  Internal method that will update UTMElement to set the
  reference position.
*/
void 
UTMCamera::setReferencePosition(SoState * state)
{
  SbVec3d utm = this->utmposition.getValue();
  UTMElement::setReferencePosition(state,
                                   utm[0], utm[1], utm[2]);
  
  if (this->moveTransform.getValue() && state->isElementEnabled(SoModelMatrixElement::getClassStackIndex())) {
    SbMatrix m = SoModelMatrixElement::get(state);
    SoModelMatrixElement::makeIdentity(state, this);
    UTMElement::setGlobalTransform(state, m);
  }
}

void 
UTMCamera::getMatrix(SoGetMatrixAction * action)
{
  this->setReferencePosition(action->getState());
  if (this->moveTransform.getValue()) {
    SbMatrix m = action->getMatrix();
    action->getMatrix() = SbMatrix::identity();
    action->getInverse() = SbMatrix::identity();
    UTMElement::setGlobalTransform(action->getState(), m);
  }
}

/*!
  Overloaded to recalculate cached values when something change.
*/
void 
UTMCamera::notify(SoNotList * nl)
{
  SoField * f = nl->getLastField();
  SbVec3d utm = this->utmposition.getValue();
  SbBool update = FALSE;
  if (f == &this->easting) {
    utm[0] = atof(this->easting.getValue().getString());
    update = TRUE;
  }
  else if (f == &this->northing) {
    utm[1] = atof(this->northing.getValue().getString());
    update = TRUE;
  }
  else if (f == &this->elevation) {
    utm[2] = atof(this->elevation.getValue().getString());
    update = TRUE;
  }
  else if (f == &this->position) {
    SbVec3f v = this->position.getValue();
    if (v != SbVec3f(0.0f, 0.0f, 0.0f)) {
//       myfprintf(stderr,"Warning: don't use camera position, but utmposition (%g %g %g)\n",
//                 v[0], v[1], v[2]);
    }
  }
  if (update) {
    SbBool old = this->enableNotify(FALSE);
    this->utmposition = utm;
    (void) this->enableNotify(old);
  }
  inherited::notify(nl);
}
