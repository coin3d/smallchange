#ifndef SMALLCHANGE_UTMCOORDINATE_H
#define SMALLCHANGE_UTMCOORDINATE_H

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
#include <Inventor/fields/SoMFVec3f.h>
#include <Inventor/lists/SbList.h>

class SoState;

class UTMCoordinate : public SoNode {
  typedef SoNode inherited;

  SO_NODE_HEADER(UTMCoordinate);

public:
  static void initClass(void);
  UTMCoordinate(void);

  // coordinates in this order: [easting, northing, elevation]
  SoMFVec3f point; // FIXME: need SoMFVec3d for better precision

  virtual void doAction(SoAction * action);
  virtual void GLRender(SoGLRenderAction * action);
  virtual void callback(SoCallbackAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  virtual void pick(SoPickAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);

protected:
  virtual ~UTMCoordinate();
  virtual void notify(SoNotList * nl);

private:
  void updateCoords(SoState * state);
  SbList <SbVec3f> coords;
  double refpos[3];
  SbVec3f trans;
  SbBool dirty;
};

#endif // !SMALLCHANGE_UTMCOORDINATE_H
