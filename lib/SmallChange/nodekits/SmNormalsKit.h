#ifndef SMALLCHANGE_NORMALSKIT_H
#define SMALLCHANGE_NORMALSKIT_H

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

#include <Inventor/nodekits/SoBaseKit.h>
#include <Inventor/nodekits/SoSubKit.h>
#include <Inventor/fields/SoSFNode.h>
#include <Inventor/fields/SoSFFloat.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinLeaveScope.h>
#endif // win

#include <SmallChange/basic.h>

class SMALLCHANGE_DLL_API SmNormalsKit : public SoBaseKit {
  typedef SoBaseKit inherited;
  SO_KIT_HEADER(SmNormalsKit);
  SO_KIT_CATALOG_ENTRY_HEADER(topSeparator);

public:
  static void initClass(void);
  SmNormalsKit(void);

  SoSFNode scene;
  SoSFFloat length;

protected:
  virtual ~SmNormalsKit(void);

private:
  class NormalsKitP * pimpl;

};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinEnterScope.h>
#endif // win

#endif // !SMALLCHANGE_NORMALSKIT_H
