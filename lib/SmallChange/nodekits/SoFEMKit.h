#ifndef SMALLCHANGE_SOFEM_H
#define SMALLCHANGE_SOFEM_H

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
#include <Inventor/SbVec3f.h>

#include <SmallChange/basic.h>

class SoFEMKitP;
class SbColor;
class SbPlane;
class SoSensor;
class SbVec3f;


class SMALLCHANGE_DLL_API SoFEMKit : public SoBaseKit {
  typedef SoBaseKit inherited;

  SO_KIT_HEADER(SoFEMKit);
  
  SO_KIT_CATALOG_ENTRY_HEADER(topSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(shapehints);
  SO_KIT_CATALOG_ENTRY_HEADER(mbind);
  SO_KIT_CATALOG_ENTRY_HEADER(nbind);
  SO_KIT_CATALOG_ENTRY_HEADER(nodes);
  SO_KIT_CATALOG_ENTRY_HEADER(colors);
  SO_KIT_CATALOG_ENTRY_HEADER(normals);
  SO_KIT_CATALOG_ENTRY_HEADER(faceset);
  
public:
  SoFEMKit(void);
  
  SoSFBool ccw;
  SoSFBool threadSafe;

  static void initClass(void);

protected:
  virtual ~SoFEMKit();

public:
  
  void preRender(SoAction * action);

  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  virtual void GLRender(SoGLRenderAction * action);

  void reset(void);

  void removeHiddenFaces(const SbBool onoff);
  
  void addNode(const int nodeidx, const SbVec3f & xyz);
  void add3DElement(const int elementidx, const int32_t * nodes, const int layerindex = 0);
  void add2DElement(const int elementidx, const int32_t * nodes, const int layerindex = 0);

  void setNodeColor(const int nodeidx, const SbColor & color);
  void setElementColor(const int elementidx, const SbColor & color);

  void enableAllElements(const SbBool onoroff);
  void enableElement(const int elementidx, const SbBool onoroff);
  void enableElements(const SbPlane & plane, const SbBool onoroff);
  void enableLayer(const int layerindex, const SbBool onoroff);

  void create3DIndices(int32_t * idxarray, const int32_t * nodes); 
  void create2DIndices(int32_t * idxarray, const int32_t * nodes); 
  
private:

  void updateScene(void);

  static void ccw_cb(void * data, SoSensor * sensor);
  SoFEMKitP * pimpl;

};

#endif // !SMALLCHANGE_SOFEM_H
