#ifndef SMALLCHANGE_SOFEM_H
#define SMALLCHANGE_SOFEM_H

/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2002 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.  See the
 *  file LICENSE.GPL at the root directory of this source distribution
 *  for more details.
 *
 *  If you desire to use SmallChange with software that is incompatible
 *  licensewise with the GPL, and / or you would like to take
 *  advantage of the additional benefits with regard to our support
 *  services, please contact Systems in Motion about acquiring a Coin
 *  Professional Edition License.  See <URL:http://www.coin3d.org> for
 *  more information.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  <URL:http://www.sim.no>, <mailto:support@sim.no>
 *
\**************************************************************************/

#include <Inventor/nodekits/SoSubKit.h>
#include <Inventor/nodekits/SoBaseKit.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/SbVec3f.h>

class SoFEMKitP;
class SbColor;
class SbPlane;
class SoSensor;
class SbVec3f;

class SoFEMKit : public SoBaseKit {
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
