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
  \class SmInverseRotation SmInverseRotation.h
  \brief SmInverseRotation is a class for calculating the inverse rotation.

*/

/*!
  \var SoSFBool SmInverseRotation::inverse
  The inverse rotation.
*/

/*!
  \var SoMFRotation SmInverseRotation::rotation
  The rotation input.
*/


#include "SmInverseRotation.h"

SO_ENGINE_SOURCE(SmInverseRotation);

// doc from parent
void
SmInverseRotation::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_ENGINE_INIT_CLASS(SmInverseRotation, SoEngine, "Engine");
  }
}

/*!
  Constructor.
*/
SmInverseRotation::SmInverseRotation(void)
{
  SO_ENGINE_CONSTRUCTOR(SmInverseRotation);
  
  SO_ENGINE_ADD_INPUT(rotation, (SbRotation::identity()));

  SO_ENGINE_ADD_OUTPUT(inverse, SoMFRotation);  
}

/*!
  Destructor.
*/
SmInverseRotation::~SmInverseRotation()
{
}

// doc from parent
void
SmInverseRotation::evaluate(void)
{
  int num = this->rotation.getNum();
  SO_ENGINE_OUTPUT(inverse, SoMFRotation, setNum(num));
  
  SbRotation tmp;
  for (int i = 0; i < num; i++) {
    tmp = this->rotation[i].inverse();
    SO_ENGINE_OUTPUT(inverse, SoMFRotation, set1Value(i, tmp));
  }
}
