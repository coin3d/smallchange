#ifndef SMALLCHANGE_SOPOINTCLOUD_H
#define SMALLCHANGE_SOPOINTCLOUD_H

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

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoNonIndexedShape.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFEnum.h>

class SoPointCloud : public SoNonIndexedShape {
  typedef SoNonIndexedShape inherited;

  SO_NODE_HEADER(SoPointCloud);

public:
  static void initClass(void);
  SoPointCloud(void);

  SoSFInt32 numPoints;
  SoSFFloat detailDistance;
  SoSFFloat xSize;
  SoSFFloat ySize;
  SoSFFloat zSize;

  enum Mode {
    ALWAYS_POINTS,
    DISTANCE_BASED,
    ALWAYS_SHAPE
  };
  SoSFEnum mode;

  enum Shape {
    BILLBOARD_DIAMOND,
    CUBE
  };
  SoSFEnum shape;

  virtual void GLRender(SoGLRenderAction * action);
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

#endif // !SMALLCHANGE_SOPOINTSET_H
