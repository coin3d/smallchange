#ifndef COIN_SOFEM_H
#define COIN_SOFEM_H

/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion. All rights reserved.
 *
 *  Systems in Motion, Prof Brochs gate 6, 7030 Trondheim, NORWAY
 *  http://www.sim.no support@sim.no Voice: +47 22114160 Fax: +47 22207097
 *
\**************************************************************************/

#include <Inventor/nodekits/SoSubKit.h>
#include <Inventor/nodekits/SoBaseKit.h>
#include <Inventor/fields/SoSFBool.h>

class SoFEMKitP;
class SbColor;
class SbPlane;

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

  static void initClass(void);

protected:
  virtual ~SoFEMKit();

public:
  
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

#endif // COIN_SOFEM_H






