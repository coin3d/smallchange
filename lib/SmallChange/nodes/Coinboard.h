/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

#ifndef COIN_COINBOARD_H
#define COIN_COINBOARD_H

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

  void generate(const SbVec3f & offset,
                const SbVec3f * coords, const int n,
                const SbVec4f * texcoords = NULL);

  void generate(const SbMatrix & transform,
                const SbVec3f * coords, const int n,
                const SbVec4f * texcoords = NULL);

};

#endif // COIN_COINBOARD_H
