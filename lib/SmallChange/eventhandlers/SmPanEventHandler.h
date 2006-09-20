
#ifndef SMALLCHANGE_PAN_EVENTHANDLER_H
#define SMALLCHANGE_PAN_EVENTHANDLER_H

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

class SmPanEventHandlerP;
class SbSphereSheetProjector;

class SMALLCHANGE_DLL_API SmPanEventHandler : public SmEventHandler {
  typedef SmEventHandler inherited;

  SO_NODE_HEADER(SmPanEventHandler);
  
public:
  SmPanEventHandler(void);
  virtual void handleEvent(SoHandleEventAction * action);
  static void initClass(void);

protected:
  virtual ~SmPanEventHandler();
  void pan(const SbVec2f & currpos, 
           const SbVec2f & prevpos);

  void spin(const SbVec2f & currpos, 
            const SbVec2f & prevpos);

  void zoom(const SbVec2f & currpos,
            const SbVec2f & prevpos);

private:
  SmPanEventHandlerP * pimpl;
};

#endif // !SMALLCHANGE_PAN_EVENTHANDLER_H