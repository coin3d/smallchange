#ifndef SMALLCHANGE_SMHELICOPTEREVENTHANDLER_H
#define SMALLCHANGE_SMHELICOPTEREVENTHANDLER_H

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

