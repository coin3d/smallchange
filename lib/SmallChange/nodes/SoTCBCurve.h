#ifndef SMALLCHANGE_SOTCBCURVE_H
#define SMALLCHANGE_SOTCBCURVE_H

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

#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoMFTime.h>
#include <Inventor/fields/SoSFInt32.h>

#include <SmallChange/basic.h>


class SMALLCHANGE_DLL_API SoTCBCurve : public SoShape
{
  typedef SoShape inherited;

  SO_NODE_HEADER(SoTCBCurve);

public:
  SoMFTime timestamp;
  SoSFInt32 numControlpoints;


  static void initClass(void);
  SoTCBCurve(void);

  int getLinesPerSegment(void);

  static void TCB(const SbVec3f * vec, const SoMFTime & timestamp,
                  const int numControlpoints, const SbTime time, SbVec3f &res);

protected:
  virtual ~SoTCBCurve();

  virtual void GLRender(SoGLRenderAction * action);
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

private:
  friend class SoTCBCurveP;
  class SoTCBCurveP * pimpl;
};

#endif // !SMALLCHANGE_SOTCBCURVE_H
