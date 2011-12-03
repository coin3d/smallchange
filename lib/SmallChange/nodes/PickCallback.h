#ifndef PICKCALLBACK_H
#define PICKCALLBACK_H

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

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFTrigger.h>
#include <Inventor/fields/SoSFString.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/lists/SoCallbackList.h>
#include <Inventor/SbVec2s.h>
#include <Inventor/SbViewportRegion.h>

#include <SmallChange/basic.h>

class SoPath;
class SoPickedPoint;
class SoEvent;
class pc_sensordata;

class SMALLCHANGE_DLL_API PickCallback : public SoGroup {
  typedef SoGroup inherited;

  SO_NODE_HEADER(PickCallback);

public:
  static void initClass(void);
  PickCallback(void);

  SoSFBool pickable;
  SoSFTrigger trigger;
  SoSFBool onMousePress;
  SoSFBool onMouseRelease;
  SoSFBool onButton1;
  SoSFBool onButton2;
  SoSFBool onButton3;
  SoSFString schemeFile;
  SoSFString schemeScript;

  SoSFVec3f objectSpacePickedPoint;
  SoSFVec3f worldSpacePickedPoint;

  SoSFBool delayTrigger;

  const SoPickedPoint * getCurrentPickedPoint(void) const;
  SbBool isButton1(void) const;
  SbBool isButton2(void) const;

  static void setSchemeEvalFunctions(int (*scriptcb)(const char *),
                                     void (*filecb)(const char *));

  void addCallback(void (*callback)(void *, SoPath *), void * userdata);
  void removeCallback(void (*callback)(void *, SoPath *), void * userdata);

  SbVec2s getEventPosition(void) const;
  SbBool currentIsMouseDown(void) const;
  const SbViewportRegion & getEventViewportRegion(void) const;

  // obsoleted
  // const SoEvent * getCurrentEvent(void) const;
  // SoHandleEventAction * getCurrentAction(void) const;

protected:
  virtual ~PickCallback();
  virtual void handleEvent(SoHandleEventAction * action);

private:
  friend class pc_sensordata;
  SbBool testPick(SoHandleEventAction * action, const SbBool mousepress);
  SoCallbackList cblist;
  const SoPickedPoint * pickedpoint;
  SbBool mousepress;
  int buttonnum;
  SbViewportRegion viewport;
  SbVec2s eventpos;
  pc_sensordata * current;
};

#endif // !PICKCALLBACK_H
