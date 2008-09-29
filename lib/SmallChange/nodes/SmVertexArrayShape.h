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

#ifndef COIN_VERTEXARRAYSHAPE_H
#define COIN_VERTEXARRAYSHAPE_H

#include <Inventor/nodes/SoShape.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/fields/SoMFInt32.h>
#include <Inventor/fields/SoSFNode.h>
#include <Inventor/fields/SoSFEnum.h>

#include <SmallChange/basic.h>

class SmVertexArrayShapeP;

class SMALLCHANGE_DLL_API SmVertexArrayShape : public SoShape {
  typedef SoShape inherited;

  SO_NODE_HEADER(SmVertexArrayShape);

public:
  static void initClass(void);
  SmVertexArrayShape(void);

  enum ShapeType {
    POINTS = -1,
    LINES = -2,
    LINE_LOOP = -3,
    LINE_STRIP = -4,
    TRIANGLES = -5,
    TRIANGLE_STRIP = -6,
    TRIANGLE_FAN = -7,
    QUADS = -8,
    QUAD_STRIP = -9,
    POLYGON = -10
  };

  enum RenderAsVertexBufferObjects {
    OFF, ON, AUTO
  };

  SoSFEnum renderAsVertexBufferObject;
  SoMFInt32 vertexIndex;
  SoSFNode vertexCoord;
  SoSFNode vertexNormal;
  SoSFNode vertexTexCoord;
  SoSFNode vertexColor;

  virtual void GLRender(SoGLRenderAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);

protected:
  virtual ~SmVertexArrayShape();

  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);
  virtual void notify(SoNotList * list);

private:
  SmVertexArrayShapeP * pimpl;
};

#endif // COIN_VERTEXARRAYSHAPE_H
