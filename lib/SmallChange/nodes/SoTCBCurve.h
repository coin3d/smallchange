#ifndef SMALLCHANGE_SOTCBCURVE_H
#define SMALLCHANGE_SOTCBCURVE_H

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

#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoMFTime.h>
#include <Inventor/fields/SoMFVec3f.h>
#include <Inventor/fields/SoSFInt32.h>

class SoTCBCurve : public SoShape
{
  typedef SoShape inherited;

  SO_NODE_HEADER(SoTCBCurve);

public:

  // Fields
  SoMFTime  timestamp;
  SoSFInt32 numControlpoints;

  static void initClass(void);
  SoTCBCurve(void);

  int getLinesPerSegment();

  static void TCB(  const float coords[][3], 
                    const float tStamps[], 
                    int numControlPoints, 
                    float time, 
                    float &x, 
                    float &y, 
                    float &z);

  static void TCB(  const SoMFVec3f &vec, 
                    const SoMFTime &timestamp, 
                    const SbTime &time, 
                    SbVec3f &res);

  static void TCB(  const SbVec3f * vec, 
                    const SoMFTime &timestamp, 
                    int numControlpoints,
                    const SbTime &time, 
                    SbVec3f &res);


protected:

  virtual ~SoTCBCurve();

  virtual void GLRender(SoGLRenderAction *action);
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction *action, SbBox3f &box, SbVec3f &center);

private:
  friend class SoTCBCurveP;
  class SoTCBCurveP * pimpl;
};

#endif // !SMALLCHANGE_SOTCBCURVE_H
