#ifndef COIN_SOPOINTCLOUD_H
#define COIN_SOPOINTCLOUD_H

/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoNonIndexedShape.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoSFFloat.h>

class COIN_DLL_API SoPointCloud : public SoNonIndexedShape {
  typedef SoNonIndexedShape inherited;

  SO_NODE_HEADER(SoPointCloud);

public:
  static void initClass(void);
  SoPointCloud(void);

  SoSFInt32 numPoints;
  SoSFFloat detailDistance;
  SoSFFloat radius;

  virtual void GLRender(SoGLRenderAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);

protected:
  virtual ~SoPointCloud();

  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

  virtual SbBool generateDefaultNormals(SoState *, SoNormalCache * nc);
  virtual SbBool generateDefaultNormals(SoState * state,
                                        SoNormalBundle * bundle);

private:
  enum Binding {
    OVERALL = 0,
    PER_VERTEX
  };

  Binding findMaterialBinding(SoState * const state) const;
};

#endif // !COIN_SOPOINTSET_H
