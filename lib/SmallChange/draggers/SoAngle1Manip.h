#ifndef SMALLCHANGE_SOANGLE1MANIP_H
#define SMALLCHANGE_SOANGLE1MANIP_H

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

#include <Inventor/manips/SoTransformManip.h>
#include <Inventor/draggers/SoDragger.h>
#include <Inventor/fields/SoSFRotation.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <Inventor/manips/SoTransformManip.h>

#include <SmallChange/basic.h>


class SMALLCHANGE_DLL_API SoAngle1Manip : public SoTransformManip {

  SO_NODE_HEADER(SoAngle1Manip);

public:

  static void initClass(void);
  SoAngle1Manip(void);

protected:
  ~SoAngle1Manip();

private:
};

#endif // !SMALLCHANGE_SOANGLE1MANIP_H
