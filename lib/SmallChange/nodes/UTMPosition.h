/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2000 by Systems in Motion. All rights reserved.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  http://www.sim.no support@sim.no Voice: +47 22114160 Fax: +47 22207097
 *
\**************************************************************************/

#ifndef COIN_UTMPOSITION_H
#define COIN_UTMPOSITION_H

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoTransformation.h>
#include <Inventor/fields/SoSFString.h>
#include <Inventor/fields/SoSFVec3d.h>

class SbMatrix;
class SoState;

class UTMPosition : public SoTransformation {
  typedef SoTransformation inherited;

  SO_NODE_HEADER(UTMPosition);

public:
  static void initClass(void);
  UTMPosition(void);

  SoSFVec3d utmposition;

  virtual void doAction(SoAction * action);
  virtual void callback(SoCallbackAction * action);
  virtual void GLRender(SoGLRenderAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  virtual void getMatrix(SoGetMatrixAction * action);
  virtual void pick(SoPickAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);

protected:
  virtual ~UTMPosition();
  virtual void notify(SoNotList * nl);

private:
  // for backwards compatibility
  SoSFString easting;
  SoSFString northing;
  SoSFString elevation;

};

#endif // !COIN_UTMPOSITION_H
