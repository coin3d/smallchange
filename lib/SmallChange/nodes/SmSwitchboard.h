#ifndef SMALLCHANGE_SWITCHBOARD_H
#define SMALLCHANGE_SWITCHBOARD_H

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

#include <Inventor/nodes/SoGroup.h>
#include <Inventor/fields/SoMFBool.h>
#include <Inventor/nodes/SoSubNode.h>

#include <SmallChange/basic.h>


class SMALLCHANGE_DLL_API SmSwitchboard : public SoGroup {
  typedef SoGroup inherited;
  SO_NODE_HEADER(SmSwitchboard);

public:
  static void initClass(void);
  SmSwitchboard(void);
  SmSwitchboard(int numchildren);

  SoMFBool enable;

  virtual void doAction(SoAction * action);
  virtual void callback(SoCallbackAction * action);
  virtual void GLRender(SoGLRenderAction * action);
  virtual void pick(SoPickAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  virtual void handleEvent(SoHandleEventAction * action);
  virtual void getMatrix(SoGetMatrixAction * action);
  virtual void search(SoSearchAction * action);

protected:
  virtual ~SmSwitchboard(void);

};

#endif // !SMALLCHANGE_SWITCHBOARD_H
