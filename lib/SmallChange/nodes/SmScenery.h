#ifndef SMALLCHANGE_SCENERY_H
#define SMALLCHANGE_SCENERY_H

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

#include <Inventor/SbBasic.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoSFString.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoMFInt32.h>
#include <Inventor/fields/SoMFFloat.h>

#include <SmallChange/basic.h>

typedef struct ss_system ss_system;
typedef struct ss_render_pre_cb_info ss_render_pre_cb_info;

typedef uint32_t SmSceneryTexture2CB(void * closure, double * pos, float elevation, double * spacing);

class SceneryP;

class SMALLCHANGE_DLL_API SmScenery : public SoShape {
  typedef SoShape inherited;
  SO_NODE_HEADER(SmScenery);

public:
  static void initClass(void);
  static SmScenery * createInstance(double * origo, double * spacing, int * elements, float * values, float undef = -1.0e30f);
  SmScenery(void);

  SoSFString filename;
  SoMFInt32 renderSequence;
  SoSFFloat blockRottger;
  SoSFFloat loadRottger;
  SoSFBool visualDebug;

  SoSFBool colorTexture;
  SoMFFloat colorMap; // r, g, b, a values
  SoMFFloat colorElevation;

  virtual void GLRender(SoGLRenderAction * action);
  virtual void callback(SoCallbackAction * action);
  virtual void rayPick(SoRayPickAction * action);

  void preFrame(void);
  int postFrame(void);

  void setBlockRottger(const float c);
  float getBlockRottger(void) const;
  void setLoadRottger(const float c);
  float getLoadRottger(void) const;
  void refreshTextures(const int id);

  void setAttributeTextureCB(SmSceneryTexture2CB * callback, void * closure);

protected:
  virtual ~SmScenery(void);
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

private:
  SmScenery(ss_system * system);

  SceneryP * pimpl;
  friend class SceneryP;

};

#endif // !SMALLCHANGE_SCENERY_H
