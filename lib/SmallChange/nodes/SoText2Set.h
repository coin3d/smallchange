#ifndef SMALLCHANGE_SOTEXT2SET_H
#define SMALLCHANGE_SOTEXT2SET_H

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

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoMFFloat.h>
#include <Inventor/fields/SoMFTime.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFVec2f.h>
#include <Inventor/SbVec2f.h>

class SoText2Set : public SoShape
{
  typedef SoShape inherited;

  SO_NODE_HEADER(SoText2Set);

public:

  enum Justification {
    LEFT = 1,
    RIGHT,
    CENTER
  };

  // Fields
  SoSFEnum    justification;
  SoMFString  strings;
  SoSFVec2f   displacement;



  static void initClass(void);
  virtual void GLRender(SoGLRenderAction * action);
  virtual void rayPick(SoRayPickAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);

  SoText2Set(void);

protected:
  virtual ~SoText2Set();

  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

private:
  friend class SoText2SetP;
  class SoText2SetP * pimpl;
};

#endif // !SMALLCHANGE_SOTEXT2SET_H
