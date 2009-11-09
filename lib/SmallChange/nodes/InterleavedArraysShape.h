#ifndef INTERLEAVEDARRAYSSHAPE_H
#define INTERLEAVEDARRAYSSHAPE_H

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
#include <Inventor/nodes/SoVertexShape.h>
#include <Inventor/fields/SoMFInt32.h>
#include <Inventor/fields/SoSFEnum.h>

class SoPickedPoint;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinLeaveScope.h>
#endif // win

#include <SmallChange/basic.h>

class SMALLCHANGE_DLL_API InterleavedArraysShape : public SoVertexShape {
  typedef SoShape inherited;
  
  SO_NODE_HEADER(InterleavedArraysShape);
  
 public:
  static void initClass(void);
  InterleavedArraysShape(void);
  
  enum Type {
    POINTS,
    LINES,
    TRIANGLES,
    QUADS,
    TRIANGLE_STRIP,
    QUAD_STRIP
  };

  SoSFEnum type;
  SoMFInt32 vertexIndex;

  virtual void GLRender(SoGLRenderAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);
  virtual void rayPick(SoRayPickAction * action);

protected:
  virtual void notify(SoNotList * nl);
  virtual ~InterleavedArraysShape();
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

private:
  enum Binding {
    OVERALL,
    PER_VERTEX
  };

  SbBool shouldIncrementFaceIndex(Type type, int i) const;
  SoPickedPoint * tryPickTriangle(SoRayPickAction * action,
                                  const SbVec3f * vertices,
                                  int v0, int v1, int v2,
                                  SbVec3f & barycentric);
                       
  Binding findMaterialBinding(SoState * state) const;
  Binding findNormalBinding(SoState * state) const;
  void enableArrays(SoGLRenderAction * action, SbBool normals, SbBool colors, SbBool texcoords);
  void disableArrays(SoGLRenderAction * action, SbBool normals, SbBool colors, SbBool texcoords);
  void createVBO(uint32_t contextid);
  void createIndexVBO(uint32_t contextid);
  class Pimpl;
  Pimpl * pimpl;
  class VBO;
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinEnterScope.h>
#endif // win

#endif // INTERLEAVEDARRAYSSHAPE_H
