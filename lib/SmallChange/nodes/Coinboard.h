#ifndef SMALLCHANGE_COINBOARD_H
#define SMALLCHANGE_COINBOARD_H

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

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoMFVec3f.h>
#include <Inventor/fields/SoMFVec4f.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoMFInt32.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinLeaveScope.h>
#endif // win

class SbMatrix;

class Coinboard : public SoShape {
  typedef SoShape inherited;

  SO_NODE_HEADER(Coinboard);

public:
  static void initClass(void);
  Coinboard(void);

  SoMFVec3f position;
  SoSFVec3f axisOfRotation;
  SoSFInt32 frontAxis;

  enum ShapeType {
    TRIANGLES,
    QUADS,
    TRIANGLE_STRIP,
    TRIANGLE_FAN
  };

  SoMFVec3f coord;
  SoMFVec4f texCoord;
  SoSFVec3f normal;
  SoSFEnum shapeType;
  SoMFInt32 numVertices;

  virtual void GLRender(SoGLRenderAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);

protected:
  virtual ~Coinboard();
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

private:

  void render(const SbVec3f & offset,
              const SbVec3f * coords, const int n,
              const SbVec4f * texcoords = NULL);

  void render(const SbMatrix & matrix,
              const SbVec3f * coords, const int n,
              const SbVec4f * texcoords = NULL);

  void generate(SoPrimitiveVertex * v,
                const SbVec3f & offset,
                const SbVec3f * coords, const int n,
                const SbVec4f * texcoords = NULL);
  
  void generate(SoPrimitiveVertex * v,
                const SbMatrix & transform,
                const SbVec3f * coords, const int n,
                const SbVec4f * texcoords = NULL);
};

#endif // !SMALLCHANGE_COINBOARD_H
