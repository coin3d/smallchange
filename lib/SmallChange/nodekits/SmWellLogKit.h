#ifndef SMALLCHANGE_SMWELLLOGKIT_H
#define SMALLCHANGE_SMWELLLOGKIT_H

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
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFTime.h>
#include <Inventor/fields/SoSFString.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/fields/SoMFFloat.h>
#include <Inventor/fields/SoMFVec3d.h>
#include <SmallChange/basic.h>

class SmWellLogKitP;
class SoSensor;

class SMALLCHANGE_DLL_API SmWellLogKit : public SoBaseKit {
  typedef SoBaseKit inherited;

  SO_KIT_HEADER(SmWellLogKit);

  SO_KIT_CATALOG_ENTRY_HEADER(topSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(utm);
  SO_KIT_CATALOG_ENTRY_HEADER(transform);
  SO_KIT_CATALOG_ENTRY_HEADER(topLod);
  SO_KIT_CATALOG_ENTRY_HEADER(topLodGroup);
  SO_KIT_CATALOG_ENTRY_HEADER(shapeHints);
  SO_KIT_CATALOG_ENTRY_HEADER(wellMaterial);
  SO_KIT_CATALOG_ENTRY_HEADER(well);
  SO_KIT_CATALOG_ENTRY_HEADER(lightModel);
  SO_KIT_CATALOG_ENTRY_HEADER(lod);
  SO_KIT_CATALOG_ENTRY_HEADER(lodSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(callback);
  SO_KIT_CATALOG_ENTRY_HEADER(coord);
  SO_KIT_CATALOG_ENTRY_HEADER(materialBinding);
  SO_KIT_CATALOG_ENTRY_HEADER(faceSetColor);
  SO_KIT_CATALOG_ENTRY_HEADER(pickStyle);
  SO_KIT_CATALOG_ENTRY_HEADER(faceSet);
  SO_KIT_CATALOG_ENTRY_HEADER(info);
  SO_KIT_CATALOG_ENTRY_HEADER(topInfo);
  
public:
  SmWellLogKit(void);
  static void initClass(void);

  SoSFFloat undefVal;
  SoSFString name;
  SoMFVec3d wellCoord;
  
  SoMFString curveNames;
  SoMFFloat curveData;
  SoSFFloat leftSize;
  SoSFFloat rightSize;

  SoSFInt32 leftCurveIndex;
  SoSFInt32 rightCurveIndex;

  SoSFBool leftUseLog;
  SoSFBool rightUseLog;

  SoSFFloat lodDistance1;
  SoSFFloat lodDistance2;
  SoSFFloat wellRadius;

  virtual void getBoundingBox(SoGetBoundingBoxAction * action);

protected:
  virtual void notify(SoNotList * l);
  virtual ~SmWellLogKit();
  
public:
  
private:
  
  int getNumCurves(void) const;
  int getNumCurveValues(void) const;
  float getDepth(const int idx) const;
  float getLeftCurveData(const int idx) const;
  float getRightCurveData(const int idx) const;

  friend class SmWellLogKitP;
  SmWellLogKitP * pimpl;
};

#endif // SMALLCHANGE_SMWELLLOGKIT_H
