#ifndef SMALLCHANGE_SMEXAMINEREVENTHANDLER_H
#define SMALLCHANGE_SMEXAMINEREVENTHANDLER_H

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

#include <SmallChange/eventhandlers/SmEventHandler.h>
#include <SmallChange/basic.h>
#include <Inventor/SbTime.h>
#include <Inventor/SbVec2f.h>
#include <Inventor/SbVec2s.h>
#include <Inventor/SbRotation.h>
#include <Inventor/SbPlane.h>
#include <Inventor/fields/SoSFBool.h>

class SbSphereSheetProjector;
class SoCamera;

class SMALLCHANGE_DLL_API SmExaminerEventHandler : public SmEventHandler {
  typedef SmEventHandler inherited;

  SO_NODE_HEADER(SmExaminerEventHandler);
  
public:
  SmExaminerEventHandler(void);
  static void initClass(void);

  SoSFBool enableSpin;

  virtual void handleEvent(SoHandleEventAction * action);
  virtual SbBool isAnimating(void);
  virtual void preRender(SoGLRenderAction * action);

protected:
  virtual ~SmExaminerEventHandler();
  
  virtual float clampZoom(const float val);
  void enableButton3Movement(const SbBool onoff);

private:
  void setMotion3OnCamera(SbBool enable);
  SbBool getMotion3OnCamera(void) const;

  float rotXWheelMotion(float value, float old);
  float rotYWheelMotion(float value, float old);

  void reorientCamera(const SbRotation & rotation);
  void spin(const SbVec2f & mousepos);
  void pan(const SbVec2f & mousepos, const SbVec2f & prevpos);
  void zoom(SoCamera * camera, const float diffvalue);
  void zoomByCursor(const SbVec2f & mousepos, const SbVec2f & prevpos);
  void pan(SoCamera * cam,
           float aspectratio, const SbPlane & panningplane,
           const SbVec2f & currpos, const SbVec2f & prevpos);

  SbVec2f lastmouseposition;
  SbPlane panningplane;

  SbBool spinanimatingallowed;
  SbVec2f lastspinposition;
  int spinsamplecounter;
  SbRotation spinincrement;
  SbSphereSheetProjector * spinprojector;

  SbRotation spinRotation;

  SbBool axiscrossEnabled;
  int axiscrossSize;


  struct { // tracking mouse movement in a log
    short size;
    short historysize;
    SbVec2s * position;
    SbTime * time;
  } log;

  // The Microsoft Visual C++ v6.0 compiler needs a name on this class
  // to be able to generate a constructor (which it wants to have for
  // running the the SbVec2s constructors). So don't try to be clever
  // and make it anonymous.
  struct Pointer {
    SbVec2s now, then;
  } pointer;

  SbBool button1down;
  SbBool button3down;
  SbBool ctrldown, shiftdown;

  void clearLog(void);
  void addToLog(const SbVec2s pos, const SbTime time);

  SbTime prevRedrawTime;

  SbBool motion3OnCamera;

  enum ViewerMode {
    IDLE,
    INTERACT,
    ZOOMING,
    PANNING,
    DRAGGING,
    SPINNING,
    SEEK_WAIT_MODE,
    SEEK_MODE
  } mode;

  ViewerMode currentmode;
  void setMode(const ViewerMode mode);

  void setCursorRepresentation(int mode);

  SbBool button3enabled;
};


#endif // SMALLCHANGE_SMEXAMINEREVENTHANDLER_H 
