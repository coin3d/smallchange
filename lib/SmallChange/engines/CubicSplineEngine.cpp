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

#include "CubicSplineEngine.h"
#include <Inventor/SoDB.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/fields/SoSFTrigger.h>
#include <Inventor/fields/SoSFRotation.h>
#include <cfloat>

SO_ENGINE_SOURCE(CubicSplineEngine);

// doc from parent
void
CubicSplineEngine::initClass(void)
{
  SO_ENGINE_INIT_CLASS(CubicSplineEngine, SoEngine, "Engine");
}

CubicSplineEngine::CubicSplineEngine(void)
{
  SO_ENGINE_CONSTRUCTOR(CubicSplineEngine);
  
  SO_ENGINE_ADD_INPUT(timeIn, (SbTime::zero()));
  SO_ENGINE_ADD_INPUT(controlpoint, (0.0f, 0.0f, 0.0f));
  SO_ENGINE_ADD_INPUT(basisMatrix, (SbMatrix::identity()));
  SO_ENGINE_ADD_INPUT(duration, (1.0f));
  SO_ENGINE_ADD_INPUT(on, (FALSE));
  SO_ENGINE_ADD_INPUT(loop, (FALSE));
  SO_ENGINE_ADD_INPUT(type, (BEZIER));
  SO_ENGINE_ADD_INPUT(orientation, (SbRotation::identity()));
  SO_ENGINE_ADD_INPUT(orientationTime, (0.0f));
  SO_ENGINE_ADD_INPUT(offset, (0.0, 0.0, 0.0));
  
  SO_ENGINE_DEFINE_ENUM_VALUE(Type, BEZIER);
  SO_ENGINE_DEFINE_ENUM_VALUE(Type, B_SPLINE);
  SO_ENGINE_DEFINE_ENUM_VALUE(Type, CATMULL_ROM);
  
  SO_ENGINE_SET_SF_ENUM_TYPE(type, Type);

  SO_ENGINE_ADD_OUTPUT(pointOut, SoSFVec3d);
  SO_ENGINE_ADD_OUTPUT(tangentOut, SoSFVec3f);
  SO_ENGINE_ADD_OUTPUT(orientationOut, SoSFRotation);
  SO_ENGINE_ADD_OUTPUT(syncOut, SoSFTrigger);

  this->verifycb = NULL;
  this->linear = FALSE;
  
  this->syncOut.enable(FALSE);
  this->pointOut.enable(FALSE);
  this->tangentOut.enable(FALSE);
  this->orientationOut.enable(FALSE);
  this->first = TRUE;

  this->timeIn.enableNotify(FALSE);
  SoField * realtime = SoDB::getGlobalField("realTime");
  this->timeIn.connectFrom(realtime);
}

CubicSplineEngine::~CubicSplineEngine()
{
}

void 
CubicSplineEngine::setVerifyCallback(void (*cb)(SbVec3d &))
{
  this->verifycb = cb;
}

void 
CubicSplineEngine::addEvaluateCallback(void (*callback)(void *, CubicSplineEngine *), 
                                      void * userdata)
{
  this->cblist.addCallback((SoCallbackListCB*) callback, userdata);
}

void 
CubicSplineEngine::removeEvaluateCallback(void (*callback)(void *, CubicSplineEngine *), 
                                         void * userdata)
{
  this->cblist.removeCallback((SoCallbackListCB*)callback, userdata);
}

void 
CubicSplineEngine::clearEvaluateCallbacks(void)
{
  cblist.clearCallbacks();
}

void
CubicSplineEngine::evaluate(void)
{
  SbMatrix m;
  float t = (float) (this->currtime / this->currduration);
  if (!this->linear) {
    t = (1.0f - float(cos(double(t)*M_PI))) * 0.5f;
  }
  if (this->pointOut.isEnabled()) {
    SbVec3f v = this->currspline.getPoint(t);
    SbVec3d res = this->offset.getValue();
    res[0] += v[0];
    res[1] += v[1];
    res[2] += v[2];
    
    if (this->verifycb) this->verifycb(res);

    SO_ENGINE_OUTPUT(pointOut, SoSFVec3d, setValue(res));
  }
  if (this->tangentOut.isEnabled()) {
    SbVec3f v = this->currspline.getTangent(t);
    SO_ENGINE_OUTPUT(tangentOut, SoSFVec3f, setValue(v));
  }
  if (this->orientationOut.isEnabled()) {
    int i = 0, n = this->orientationTime.getNum()-1; 
    while (this->orientationTime[i+1] < t && i < n) i++;
    SbRotation rot0 = this->orientation[SbMin(i, this->orientation.getNum()-1)];
    SbRotation rot1 = this->orientation[SbMin(i+1, this->orientation.getNum()-1)];
    float delta = this->orientationTime[i+1] - this->orientationTime[i];
    float diff = t - this->orientationTime[i];
#if 0 // testing non-linear spline-exec
    float rt = (1.0f - float(cos(double(diff/delta)*M_PI))) * 0.5f;
#else
    float rt = diff/delta;
#endif

    SbRotation rot = 
      SbRotation::slerp(rot0, rot1, rt);
    SO_ENGINE_OUTPUT(orientationOut, SoSFRotation, setValue(rot));      
  }

  this->cblist.invokeCallbacks(this);
}

void
CubicSplineEngine::inputChanged(SoField * which)
{
  if (which == &this->timeIn) {
    this->currtime = this->timeIn.getValue().getValue();
    if (this->first) {
      this->starttime = this->currtime;
      this->first = FALSE;
    }
    this->currtime -= this->starttime;
    if (this->currtime > this->currduration) {
      this->syncOut.enable(TRUE);
      SO_ENGINE_OUTPUT(syncOut, SoSFTrigger, setValue());
      this->syncOut.enable(FALSE);
      if (this->loop.getValue()) {
        double num = this->currtime / this->currduration;
        this->starttime += this->currduration * floor(num);
        this->currtime = this->currtime - this->starttime;
      }
      else {
        this->currtime = this->currduration;
        this->evaluate(); // force output before disabling
        this->timeIn.enableNotify(FALSE);
        this->on = FALSE;
        this->pointOut.enable(FALSE);
        this->tangentOut.enable(FALSE);
        this->orientationOut.enable(FALSE);
      }
    }
  }
  else if (which == &this->on) {
    this->activate(this->on.getValue());
  }
}

void
CubicSplineEngine::activate(const SbBool onoff)
{
  if (onoff) {
    this->currpt.truncate(0);
    switch (this->type.getValue()) {
    case CATMULL_ROM:
      this->currspline.setBasisMatrix(SbCubicSpline::CATMULL_ROM);
      break;
    case B_SPLINE:
      this->currspline.setBasisMatrix(SbCubicSpline::B_SPLINE);
      break;
    case BEZIER:
      this->currspline.setBasisMatrix(SbCubicSpline::BEZIER);
      break;
    case GENERIC:
      this->currspline.setBasisMatrix(this->basisMatrix.getValue());
      break;
    }
    this->currduration = (double) this->duration.getValue();
    this->currspline.setLoop(this->loop.getValue());

    int n = this->controlpoint.getNum();
    const SbVec3f * pts = this->controlpoint.getValues(0);
    float minlen = FLT_MAX;

    SbList <SbVec3f> tmppts;
    tmppts.append(pts[0]);

    int i;

    float totallen = 0.0f;
    for (i = 1; i < n; i++) {
      totallen += (pts[i]-pts[i-1]).length();
    }
    SbBool doorienttime = this->orientationTime.getNum() != n && this->orientation.getNum() > 1;
    if (doorienttime) {
      this->orientationTime.setNum(n);
      this->orientationTime.set1Value(0, 0.0f);
    }

    float acclen = 0.0f;
    for (i = 1; i < n; i++) {
      tmppts.append(pts[i]);
      float len = (pts[i]-pts[i-1]).length();
      if (len > 0.0f && len < minlen) {
        minlen = len;
      }
      if (doorienttime) {
        acclen += len;
        this->orientationTime.set1Value(i, SbClamp(acclen/totallen, 0.0f, 1.0f));
      }
    }
    if (this->loop.getValue()) {
      tmppts.append(pts[0]);
      float len = (pts[n-1]-pts[0]).length();
      if (len > 0.0f && len < minlen) {
        minlen = len;
      }
    }
    
    n = tmppts.getLength();
    
#if 0 // disabled, 2003-09-14, pederb. not such a good idea
    for (i = 0; i < n-1; i++) {
      SbVec3f p0 = tmppts[i];
      SbVec3f p1 = tmppts[i+1];
      float len = (p1-p0).length();
      int div = (int) (len / minlen);
      if (div >= 3) { 
        float add = 1.0f / float(div);
        len = 0.0f;
        if (i == n-2 && !this->loop.getValue()) div++;
        for (int j = 0; j < div; j ++) {
          SbVec3f p = p0 + (p1-p0)*len;
          this->currpt.append(p);
          len += add;
        }
      }
      else {
        this->currpt.append(p0);
      }
    }
    this->currpt.append(tmppts[n-1]);
#else // disabled code
    for (i = 0; i < n; i++) {
      this->currpt.append(tmppts[i]);
    }
#endif // working code

    /*
    myfprintf(stderr,"currpt: %d, currot: %d\n", 
            this->currpt.getLength(),
            this->currot.getLength());
    */
    this->currspline.setControlPoints(this->currpt.getArrayPtr(),
                                      this->currpt.getLength());
    
    this->starttime = this->timeIn.getValue().getValue();
    this->currtime = 0.0;
    this->timeIn.enableNotify(TRUE);
    this->pointOut.enable(TRUE);
    this->tangentOut.enable(TRUE);
    if (this->orientation.getNum() > 1) {
      this->orientationOut.enable(TRUE);
    }
    this->first = TRUE;
  }
  else {
    this->timeIn.enableNotify(FALSE);
    this->pointOut.enable(FALSE);
    this->tangentOut.enable(FALSE);
    this->orientationOut.enable(FALSE);
  }
}
