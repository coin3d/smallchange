#ifndef SMALLCHANGE_SMHELICOPTEREVENTHANDLER_H
#define SMALLCHANGE_SMHELICOPTEREVENTHANDLER_H

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
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/SbTime.h>
#include <Inventor/SbVec2f.h>
#include <Inventor/SbVec2s.h>
#include <Inventor/SbRotation.h>
#include <Inventor/SbPlane.h>

class SMALLCHANGE_DLL_API SmHelicopterEventHandler : public SmEventHandler {
  typedef SmEventHandler inherited;

  SO_NODE_HEADER(SmHelicopterEventHandler);

public:
  SmHelicopterEventHandler(void);
  static void initClass(void);

  SoSFFloat speed;
  SoSFBool resetRoll;

  virtual void handleEvent(SoHandleEventAction * action);
  virtual void pulse(void);

  virtual SbBool isAnimation(void);

protected:
  virtual ~SmHelicopterEventHandler();

  void moveCamera(const SbVec3f & vec, const SbBool dorotate);

private:
  int state;
  int flydirection;
  SbVec2s prevpos;
  SbVec2s mousedownpos;
  SbVec2s mousepos;
  float relspeedfly;
};

#endif // SMALLCHANGE_SMHELICOPTEREVENTHANDLER_H
