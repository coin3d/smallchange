#ifndef SMALLCHANGE_SMWELLLOGKIT_H
#define SMALLCHANGE_SMWELLLOGKIT_H

/**************************************************************************\
 * Copyright (c) Kongsberg Oil & Gas Technologies AS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
\**************************************************************************/

#include <Inventor/nodekits/SoSubKit.h>
#include <Inventor/nodekits/SoBaseKit.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoSFTime.h>
#include <Inventor/fields/SoSFString.h>
#include <Inventor/fields/SoSFColor.h>
#include <Inventor/fields/SoMFColor.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/fields/SoMFFloat.h>
#include <Inventor/fields/SoMFVec3d.h>
#include <SmallChange/basic.h>

class SmWellLogKitP;
class SoSensor;
class SbString;
class SmTooltipKit;

class SMALLCHANGE_DLL_API SmWellLogKit : public SoBaseKit {
  typedef SoBaseKit inherited;

  SO_KIT_HEADER(SmWellLogKit);

  SO_KIT_CATALOG_ENTRY_HEADER(topSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(tooltip);
  SO_KIT_CATALOG_ENTRY_HEADER(utm);
  SO_KIT_CATALOG_ENTRY_HEADER(transform);
  SO_KIT_CATALOG_ENTRY_HEADER(topLod);
  SO_KIT_CATALOG_ENTRY_HEADER(topLodGroup);
  SO_KIT_CATALOG_ENTRY_HEADER(shapeHints);
  SO_KIT_CATALOG_ENTRY_HEADER(wellBaseColor);
  SO_KIT_CATALOG_ENTRY_HEADER(wellDrawStyle);
  SO_KIT_CATALOG_ENTRY_HEADER(well);
  SO_KIT_CATALOG_ENTRY_HEADER(wellNameSep);
  SO_KIT_CATALOG_ENTRY_HEADER(wellNameColor);
  SO_KIT_CATALOG_ENTRY_HEADER(wellName);
  SO_KIT_CATALOG_ENTRY_HEADER(lightModel);
  SO_KIT_CATALOG_ENTRY_HEADER(lod);
  SO_KIT_CATALOG_ENTRY_HEADER(lodSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(callbackNode);
  SO_KIT_CATALOG_ENTRY_HEADER(coord);
  SO_KIT_CATALOG_ENTRY_HEADER(materialBinding);
  SO_KIT_CATALOG_ENTRY_HEADER(faceSetColor);
  SO_KIT_CATALOG_ENTRY_HEADER(pickStyle);
  SO_KIT_CATALOG_ENTRY_HEADER(drawStyleSwitch);
  SO_KIT_CATALOG_ENTRY_HEADER(lineSet);
  SO_KIT_CATALOG_ENTRY_HEADER(faceSet);
  SO_KIT_CATALOG_ENTRY_HEADER(info);
  SO_KIT_CATALOG_ENTRY_HEADER(topsSep);
  SO_KIT_CATALOG_ENTRY_HEADER(topsFontStyle);
  SO_KIT_CATALOG_ENTRY_HEADER(topsBaseColor);
  SO_KIT_CATALOG_ENTRY_HEADER(topsList);
  SO_KIT_CATALOG_ENTRY_HEADER(topInfo);
  
public:
  SmWellLogKit(void);
  static void initClass(void);

  SoSFFloat undefVal;
  SoSFString name;
  SoMFVec3d wellCoord;

  SoMFString curveNames;
  SoMFString curveDescription;
  SoMFString curveUnits;
  SoMFFloat curveData;
  SoSFFloat leftSize;
  SoSFFloat rightSize;

  SoSFInt32 leftCurveIndex;
  SoSFInt32 rightCurveIndex;
  
  SoSFColor leftColor;
  SoSFColor rightColor;

  SoSFBool leftUseLog;
  SoSFBool rightUseLog;

  SoSFFloat lodDistance1;
  SoSFFloat lodDistance2;
  SoSFFloat wellRadius;
  SoMFColor wellColor;

  SoMFFloat topsDepths;
  SoMFString topsNames;
  SoSFColor topsColor;
  SoSFFloat topsSize;

  SoSFFloat leftCurveMin;
  SoSFFloat leftCurveMax;

  SoSFFloat rightCurveMin;
  SoSFFloat rightCurveMax;

  virtual void GLRender(SoGLRenderAction * action);
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  virtual void handleEvent(SoHandleEventAction * action);

  virtual void callback(SoCallbackAction * action);
  virtual void getMatrix(SoGetMatrixAction * action);
  virtual void rayPick(SoRayPickAction * action);
  virtual void search(SoSearchAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);

  void addTooltipInfo(const char * name,
                      const int curveidx,
                      const int numvalues,
                      const float * data,
                      const char ** datatext);

protected:
  virtual SbBool readInstance(SoInput * in, unsigned short flags);
  virtual void notify(SoNotList * l);
  virtual ~SmWellLogKit();
  virtual void setDefaultOnNonWritingFields(void);
  
public:
  
private:
  
  int findPickIdx(const SbVec3f & p) const;
  SbBool setTooltipInfo(const int idx, SmTooltipKit * tooltip); 
  void connectNodes(void);
  int getNumCurves(void) const;
  int getNumCurveValues(void) const;
  float getDepth(const int idx) const;
  float getLeftCurveData(const int idx) const;
  float getRightCurveData(const int idx) const;

  friend class SmWellLogKitP;
  SmWellLogKitP * pimpl;
};

#endif // SMALLCHANGE_SMWELLLOGKIT_H
