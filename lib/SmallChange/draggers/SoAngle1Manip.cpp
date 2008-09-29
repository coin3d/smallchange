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
  \class SoAngle1Manip SoAngle1Manip.h 
  \brief The SoAngle1Manip class is for rotating geometry around a single axis.
  \ingroup draggers

  Use an instance of this dragger class in your scenegraph to let the
  end-users of your application rotate geometry around a pre-defined
  axis vector in 3D.

  For the dragger orientation and positioning itself, use some kind of
  transformation node in your scenegraph, as usual.
*/

#include "SoAngle1Manip.h"
#include "SoAngle1Dragger.h"
#include <Inventor/nodes/SoSurroundScale.h>


SO_NODE_SOURCE(SoAngle1Manip);


void SoAngle1Manip::initClass()
{
  static int first = 0;
  if (first == 1) return;
  first = 1;

  SO_NODE_INIT_CLASS(SoAngle1Manip, SoTransformManip, "TransformManip");

  SoAngle1Dragger::initClass();
}//initClass


SoAngle1Manip::SoAngle1Manip()
{
  SO_NODE_CONSTRUCTOR(SoAngle1Manip);

  setDragger(new SoAngle1Dragger);
}//Constructor


SoAngle1Manip::~SoAngle1Manip()
{
}//Destructor
