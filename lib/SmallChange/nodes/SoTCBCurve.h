/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2002 by Systems in Motion. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  version 2.1 as published by the Free Software Foundation. See the
 *  file LICENSE.LGPL at the root directory of the distribution for
 *  more details.
 *
 *  If you want to use Coin for applications not compatible with the
 *  LGPL, please contact SIM to acquire a Professional Edition license.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  http://www.sim.no support@sim.no Voice: +47 22114160 Fax: +47 22207097
 *
\**************************************************************************/

#ifndef COIN_SOTCBCURVE_H
#define COIN_SOTCBCURVE_H

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoMFFloat.h>
#include <Inventor/fields/SoMFTime.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoMFVec3f.h>

class SoTCBCurve : public SoShape
{
  typedef SoShape inherited;

  SO_NODE_HEADER(SoTCBCurve);

public:

  // Fields
  SoSFInt32 numControlpoints;

  static void initClass(void);
  SoTCBCurve(void);

  void setCoordinateSrc(const SoMFVec3f *);
  void setTimestampSrc(const SoMFTime *);
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



protected:

  virtual ~SoTCBCurve();

  virtual void GLRender(SoGLRenderAction *action);
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction *action, SbBox3f &box, SbVec3f &center);

private:
  friend class SoTCBCurveP;
  class SoTCBCurveP * pimpl;
};

#endif // !COIN_SOTCBCURVE_H
