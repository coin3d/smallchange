#ifndef SMALLCHANGE_SOLODEXTRUSION_H
#define SMALLCHANGE_SOLODEXTRUSION_H

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
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoMFVec2f.h>
#include <Inventor/fields/SoMFVec3f.h>
#include <Inventor/fields/SoMFColor.h>
#include <Inventor/fields/SoMFRotation.h>

class SoLODExtrusion : public SoShape
{
  typedef SoShape inherited;
  SO_NODE_HEADER(SoLODExtrusion);

public:
  static void initClass(void);
  SoLODExtrusion(void);

  SoSFBool beginCap;
  SoSFBool ccw;
  SoSFFloat creaseAngle;
  SoMFVec2f crossSection;
  SoSFBool endCap;
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
