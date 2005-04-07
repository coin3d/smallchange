#ifndef SMALLCHANGE_TOVERTEXARRAYSHAPEACTION_H
#define SMALLCHANGE_TOVERTEXARRAYSHAPEACTION_H

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

#include <Inventor/actions/SoAction.h>
#include <Inventor/actions/SoSubAction.h>

#include <SmallChange/basic.h>

class SmToVertexArrayShapeActionP;

class SMALLCHANGE_DLL_API SmToVertexArrayShapeAction : public SoAction {
  typedef SoAction inherited;

  SO_ACTION_HEADER(SmToVertexArrayShapeAction);
  
public:
  SmToVertexArrayShapeAction(void);
  virtual ~SmToVertexArrayShapeAction();

  static void initClass(void);

  void useIndexedFaceSet(const SbBool onoff);

  virtual void apply(SoNode * node);
  virtual void apply(SoPath * path);
  virtual void apply(const SoPathList & pathlist, SbBool obeysrules = FALSE);
  
protected:
  virtual void beginTraversal(SoNode * node);
private:
  SmToVertexArrayShapeActionP * pimpl;
};


#endif // SMALLCHANGE_TOVERTEXARRAYSHAPEACTION
