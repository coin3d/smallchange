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
  \class Rot2Heading Rot2Heading.h
  \brief Rot2Heading is a class for transforming a rotation into a vector.

  It is a useful engine, since it can, for instance, be used to
  create a headlight in a scene graph by connecting the camera
  orientation to the input, and a directional light direction to the
  output.
*/

/*!
  \var SoSFBool Rot2Heading::inverse
  Set to \e TRUE if the rotation should be inverted.
*/

/*!
  \var SoMFRotation Rot2Heading::rotation
  The rotation input.
*/

/*!  
  \var SoEngineOutput Rot2Heading::heading
  (SoSFVec3f) The rotated vector.
*/


#include "Rot2Heading.h"

SO_ENGINE_SOURCE(Rot2Heading);

// doc from parent
void
Rot2Heading::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_ENGINE_INIT_CLASS(Rot2Heading, SoEngine, "Engine");
  }
}

/*!
  Constructor.
*/
Rot2Heading::Rot2Heading(void)
{
  SO_ENGINE_CONSTRUCTOR(Rot2Heading);
  
  SO_ENGINE_ADD_INPUT(rotation, (SbRotation::identity()));
  SO_ENGINE_ADD_INPUT(inverse, (FALSE));

  SO_ENGINE_ADD_OUTPUT(heading, SoMFVec3f);  
}

/*!
  Destructor.
*/
Rot2Heading::~Rot2Heading()
{
}

// doc from parent
void
Rot2Heading::evaluate(void)
{
  int num = this->rotation.getNum();
  SO_ENGINE_OUTPUT(heading, SoMFVec3f, setNum(num));
  
  SbVec3f tmp;
  for (int i = 0; i < num; i++) {
    tmp.setValue(0.0f, 0.0f, -1.0f);
    if (this->inverse.getValue()) {
      this->rotation[i].inverse().multVec(tmp, tmp);
    }
    else {
      this->rotation[i].multVec(tmp, tmp);
    }
    SO_ENGINE_OUTPUT(heading, SoMFVec3f, set1Value(i, tmp));
  }
}

