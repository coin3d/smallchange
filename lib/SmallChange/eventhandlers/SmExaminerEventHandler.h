#ifndef SMALLCHANGE_SMEXAMINEREVENTHANDLER_H
#define SMALLCHANGE_SMEXAMINEREVENTHANDLER_H

/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
  virtual void resetCameraFocalDistance(const SbViewportRegion & vpr);

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
