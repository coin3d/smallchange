/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2000 by Systems in Motion. All rights reserved.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  http://www.sim.no support@sim.no Voice: +47 22114160 Fax: +47 22207097
 *
\**************************************************************************/

#ifndef COIN_UTMCAMERA_H
#define COIN_UTMCAMERA_H

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/fields/SoSFString.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFVec3d.h>

class SoState;

class UTMCamera : public SoPerspectiveCamera {
  typedef SoPerspectiveCamera inherited;

  SO_NODE_HEADER(UTMCamera);

public:
  static void initClass(void);
  UTMCamera(void);

  SoSFVec3d utmposition;
  SoSFBool moveTransform;

  virtual void callback(SoCallbackAction * action);
  virtual void GLRender(SoGLRenderAction * action);
  virtual void audioRender(class SoAudioRenderAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  virtual void handleEvent(SoHandleEventAction * action);
  virtual void rayPick(SoRayPickAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);
  virtual void getMatrix(SoGetMatrixAction * action);
  
  void getPosition(double & easting, double & northing, double & elevation);
    
protected:
  virtual ~UTMCamera();
  virtual void notify(SoNotList * nl);

private:
  // for backwards compatibility
  SoSFString easting;
  SoSFString northing;
  SoSFString elevation;

  void updateCameraElement(SoState * state);
  void setReferencePosition(SoState * state);
};


#endif // !COIN_UTMCAMERA_H
