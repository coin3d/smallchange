#ifndef CUBICSPLINEENGINE_H
#define CUBICSPLINEENGINE_H

/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

#include <SmallChange/basic.h>


class SMALLCHANGE_DLL_API CubicSplineEngine : public SoEngine {
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
