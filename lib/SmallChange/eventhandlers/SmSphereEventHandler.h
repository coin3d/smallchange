#ifndef SMALLCHANGE_SMSPHEREEVENTHANDLER_H
#define SMALLCHANGE_SMSPHEREEVENTHANDLER_H

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

#include <SmallChange/eventhandlers/SmExaminerEventHandler.h>
#include <SmallChange/basic.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/SbTime.h>
#include <Inventor/SbVec2f.h>
#include <Inventor/SbVec2s.h>
#include <Inventor/SbRotation.h>
#include <Inventor/SbPlane.h>

class SMALLCHANGE_DLL_API SmSphereEventHandler : public SmExaminerEventHandler {
  typedef SmExaminerEventHandler inherited;

  SO_NODE_HEADER(SmSphereEventHandler);

public:
  SmSphereEventHandler(void);
  static void initClass(void);

  virtual void handleEvent(SoHandleEventAction * action);

  SoSFFloat minDistance;
  SoSFFloat maxDistance;

protected:
  virtual float clampZoom(const float val);
  virtual ~SmSphereEventHandler();
  
private:
};

#endif // SMALLCHANGE_SMSPHEREEVENTHANDLER_H
