/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

#ifndef COIN_SOTCBCURVE_H
#define COIN_SOTCBCURVE_H

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

#endif // !COIN_SOTCBCURVE_H
