#ifndef SMALLCHANGE_AXISDISPLAYKIT_H
#define SMALLCHANGE_AXISDISPLAYKIT_H

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

#include <Inventor/nodekits/SoSubKit.h>
#include <Inventor/nodekits/SoBaseKit.h>
#include <Inventor/fields/SoSFRotation.h>
#include <Inventor/fields/SoMFBool.h>
#include <Inventor/fields/SoMFVec3f.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/fields/SoMFColor.h>
#include <SmallChange/basic.h>

class SMALLCHANGE_DLL_API SmAxisDisplayKit : public SoBaseKit
{
  typedef SoBaseKit inherited;

  SO_KIT_HEADER(SmAxisDisplayKit);

  SO_KIT_CATALOG_ENTRY_HEADER(topSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(callback);
  SO_KIT_CATALOG_ENTRY_HEADER(camera);
  SO_KIT_CATALOG_ENTRY_HEADER(depthBuffer);
  SO_KIT_CATALOG_ENTRY_HEADER(viewportRegion);
  SO_KIT_CATALOG_ENTRY_HEADER(drawstyle);
  SO_KIT_CATALOG_ENTRY_HEADER(lightmodel);
  SO_KIT_CATALOG_ENTRY_HEADER(matbinding);
  SO_KIT_CATALOG_ENTRY_HEADER(axessep);

  SoSFRotation orientation;
  SoMFVec3f axes;
  SoMFColor colors;
  SoMFBool enableArrows;
  SoMFString annotations;

public:
  SmAxisDisplayKit(void);

  static void initClass(void);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);

protected:
  virtual ~SmAxisDisplayKit();
  virtual void notify(SoNotList * l);
  virtual void setDefaultOnNonWritingFields(void);
  
private:
  class SmAxisDisplayKitP * pimpl;
  friend class SmAxisDisplayKitP;
};

#endif // !SMALLCHANGE_AXISDISPLAYKIT_H
