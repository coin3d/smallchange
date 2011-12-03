#ifndef SMALLCHANGE_SOLODEXTRUSION_H
#define SMALLCHANGE_SOLODEXTRUSION_H

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

#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFColor.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoMFVec2f.h>
#include <Inventor/fields/SoMFVec3f.h>
#include <Inventor/fields/SoMFColor.h>
#include <Inventor/fields/SoMFRotation.h>

#include <SmallChange/basic.h>


class SMALLCHANGE_DLL_API SoLODExtrusion : public SoShape
{
  typedef SoShape inherited;
  SO_NODE_HEADER(SoLODExtrusion);

public:
  static void initClass(void);
  SoLODExtrusion(void);

  SoSFBool antiSquish;
  SoSFBool ccw;
  SoSFFloat creaseAngle;
  SoMFVec2f crossSection;
  SoMFVec3f spine;
  SoSFFloat radius;
  SoSFInt32 circleSegmentCount;
  SoSFFloat lodDistance1;
  SoSFFloat lodDistance2;
  SoSFVec3f zAxis;
  SoMFColor color;
  SoSFBool pickLines;
  SoSFColor alternateColor;
  SoSFBool doAlternateColor;

  virtual void GLRender(SoGLRenderAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);
  virtual void computeBBox(SoAction * action,
                           SbBox3f & bbox, SbVec3f & center);
  virtual void rayPick(SoRayPickAction * action);

protected:
  virtual ~SoLODExtrusion();

  virtual void notify(SoNotList * list);
  virtual void generatePrimitives( SoAction * action );

  virtual SoDetail * createTriangleDetail(SoRayPickAction * action,
                                          const SoPrimitiveVertex * v1,
                                          const SoPrimitiveVertex * v2,
                                          const SoPrimitiveVertex * v3,
                                          SoPickedPoint * pp);
private:
  void updateCache(void);
  class SoLODExtrusionP * pimpl;
};
#endif // !SMALLCHANGE_SOLODEXTRUSION_H
