#ifndef SMALLCHANGE_SOLODEXTRUSION_H
#define SMALLCHANGE_SOLODEXTRUSION_H

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
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFFloat.h>
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
