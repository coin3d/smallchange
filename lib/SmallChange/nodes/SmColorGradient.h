#ifndef SMALLCHANGE_COLORGRADIENT_H
#define SMALLCHANGE_COLORGRADIENT_H

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

#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoMFFloat.h>
#include <Inventor/fields/SoMFColor.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/fields/SoSFString.h>
#include <SmallChange/basic.h>
#include <SmallChange/elements/SmColorGradientElement.h>

class SoFieldSensor;
class SoSensor;

class SMALLCHANGE_DLL_API SmColorGradient : public SoNode {
  typedef SoNode inherited;

  SO_NODE_HEADER(SmColorGradient);

public:
  static void initClass(void);
  SmColorGradient(void);

  enum Mapping {
    RELATIVE = SmColorGradientElement::RELATIVE,
    ABSOLUTE = SmColorGradientElement::ABSOLUTE
  };
  
  SoSFEnum mapping;
  SoMFColor color;
  SoMFFloat parameter;
  
  SoSFString filename;

  virtual void doAction(SoAction * action);
  virtual void GLRender(SoGLRenderAction * action);
  virtual void callback(SoCallbackAction * action);

protected:
  virtual ~SmColorGradient();
  virtual SbBool readInstance(SoInput * in, unsigned short flags);
  virtual void notify(SoNotList * l);

private:
  SbBool loadFilename(void);
  SoFieldSensor * filenamesensor;
  static void filenameSensorCB(void * data, SoSensor *);

};

#endif // !SMALLCHANGE_COLORGRADIENT_H
