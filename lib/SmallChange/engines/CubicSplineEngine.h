#ifndef CUBICSPLINEENGINE_H
#define CUBICSPLINEENGINE_H

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

#include <Inventor/engines/SoSubEngine.h>
#include <Inventor/engines/SoEngineOutput.h>
#include <Inventor/fields/SoMFVec3f.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFTime.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoMFFloat.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFMatrix.h>
#include <Inventor/fields/SoMFRotation.h>
#include <Inventor/fields/SoSFVec3d.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/lists/SoCallbackList.h>
#include <Inventor/SbMatrix.h>
#include <Inventor/lists/SbList.h>
#include <SmallChange/misc/SbCubicSpline.h>

class CubicSplineEngine : public SoEngine {
  typedef SoEngine inherited;
  
  SO_ENGINE_HEADER(CubicSplineEngine);
  
public:

  enum Type {
    CATMULL_ROM = 0,
    B_SPLINE,
    BEZIER,
    GENERIC
  };
  
  void setVerifyCallback(void (*cb)(SbVec3d &)); 
  void setLinear(const SbBool onoff) {
    this->linear = onoff;
  }

  SoSFTime timeIn;
  
  SoMFVec3f controlpoint;
  SoMFRotation orientation;
  SoMFFloat orientationTime;
  SoSFMatrix basisMatrix; // only valid when type == GENERIC
  SoSFFloat duration;
  SoSFBool on;
  SoSFBool loop;
  SoSFEnum type;
  SoSFVec3d offset;

  SoEngineOutput pointOut;        // SoSFVec3d
  SoEngineOutput tangentOut;      // SoSFVec3f
  SoEngineOutput orientationOut;  // SoSFRotation
  SoEngineOutput syncOut;         // SoSFTrigger

  static void initClass(void);
  CubicSplineEngine(void);

  void addEvaluateCallback(void (*callback)(void *, CubicSplineEngine *), void * userdata);
  void removeEvaluateCallback(void (*callback)(void *, CubicSplineEngine *), void * userdata);
  void clearEvaluateCallbacks(void);

protected:

  virtual ~CubicSplineEngine();

private:
  
  virtual void evaluate();
  virtual void inputChanged(SoField * which);

  void activate(const SbBool onoff);
  SbCubicSpline currspline;
  double currtime;
  double currduration;
  double starttime;
  SoCallbackList cblist;
  SbList <SbVec3f> currpt;
  SbBool first;
  SbBool linear;

  void (*verifycb)(SbVec3d & pos);
};


#endif // CUBICSPLINEENGINE_H
