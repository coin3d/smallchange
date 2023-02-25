#ifndef SMALLCHANGE_SMOCEANKIT_H
#define SMALLCHANGE_SMOCEANKIT_H

#include <Inventor/SbBasic.h>
#if defined(__COIN__) && (COIN_MAJOR_VERSION >= 3)

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
#include <Inventor/fields/SoSFVec2f.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/SbVec2f.h>
#include <SmallChange/basic.h>

class SmOceanKitP;
class SoSensor;
class SoPickedPoint;
class SbViewportRegion;

class SMALLCHANGE_DLL_API SmOceanKit : public SoBaseKit {
  typedef SoBaseKit inherited;

  SO_KIT_HEADER(SmOceanKit);
  SO_KIT_CATALOG_ENTRY_HEADER(topSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(utmposition);
  SO_KIT_CATALOG_ENTRY_HEADER(material);
  SO_KIT_CATALOG_ENTRY_HEADER(shapeHints);
  SO_KIT_CATALOG_ENTRY_HEADER(programSwitch);
  SO_KIT_CATALOG_ENTRY_HEADER(callback);
  SO_KIT_CATALOG_ENTRY_HEADER(waveTexture);
  SO_KIT_CATALOG_ENTRY_HEADER(debugTexture);
  SO_KIT_CATALOG_ENTRY_HEADER(debugCube);
  SO_KIT_CATALOG_ENTRY_HEADER(envMapSwitch);
  SO_KIT_CATALOG_ENTRY_HEADER(envMapUnit);
  SO_KIT_CATALOG_ENTRY_HEADER(envMap);
  SO_KIT_CATALOG_ENTRY_HEADER(resetUnit);
  SO_KIT_CATALOG_ENTRY_HEADER(shader);
  SO_KIT_CATALOG_ENTRY_HEADER(oceanShape);
  
public:
  SmOceanKit(void);
  static void initClass(void);
  float getElevation(float x, float y);

  SoSFBool enableEffects;
  SoSFVec2f size;

  SoSFFloat gravConst;
  SoSFFloat chop;
  SoSFFloat angleDeviation;
  SoSFVec2f windDirection;
  SoSFFloat minWaveLength;
  SoSFFloat maxWaveLength;
  SoSFFloat amplitudeRatio;
  SoSFFloat frequency;
  
  SoSFFloat specAttenuation;
  SoSFFloat specEnd;
  SoSFFloat specTrans;
  
  SoSFFloat envHeight;
  SoSFFloat envRadius;
  SoSFFloat waterLevel;
  SoSFFloat transitionSpeed;
  SoSFFloat sharpness;
  SoSFVec3f lightDirection;
  SoSFVec3f distanceAttenuation;
  
  SoSFFloat gridDensity;

  virtual void GLRender(SoGLRenderAction * action);

protected:

  virtual void setDefaultOnNonWritingFields(void);
  virtual ~SmOceanKit();
  
public:
  friend class SmOceanKitP;
  SmOceanKitP * pimpl;
};

#endif // temporary compile fix
#endif // SMALLCHANGE_SMOCEANKIT_H

