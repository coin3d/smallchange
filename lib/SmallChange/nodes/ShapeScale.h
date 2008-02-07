#ifndef COIN_SHAPESCALE_H
#define COIN_SHAPESCALE_H

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
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFBool.h>

#include <SmallChange/basic.h>

class SbViewport;
class SoState;
class SbColor;
class SbVec2s;
class SoCache;

class SMALLCHANGE_DLL_API ShapeScale : public SoBaseKit {
  typedef SoBaseKit inherited;

  SO_KIT_HEADER(ShapeScale);

  SO_KIT_CATALOG_ENTRY_HEADER(topSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(scale);
  SO_KIT_CATALOG_ENTRY_HEADER(shape);

public:
  static void initClass(void);
  ShapeScale(void);

  SoSFFloat active;
  SoSFFloat projectedSize;

  void preRender(SoAction * action);

protected:
  virtual void GLRender(SoGLRenderAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  virtual void notify(SoNotList * nl);
  virtual ~ShapeScale();

private:
  SbBool didrender;
  SoCache * cache;
};

#endif // ! SHAPESCALE_H
