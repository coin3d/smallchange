/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2000 by Systems in Motion. All rights reserved.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  http://www.sim.no support@sim.no Voice: +47 22114160 Fax: +47 22207097
 *
\**************************************************************************/

#ifndef COIN_UTMCOORDINATE_H
#define COIN_UTMCOORDINATE_H

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

#endif // !COIN_UTMCOORDINATE_H
