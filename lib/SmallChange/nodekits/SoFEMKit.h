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

class SoFEMKitP;
class SbColor;

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
  
  static void initClass(void);

protected:
  virtual ~SoFEMKit();
public:
  
  virtual void getBoundingBox(SoGetBoundingBoxAction * action);
  virtual void GLRender(SoGLRenderAction * action);

  void reset(void);

  void removeHidden(const SbBool onoff);
  
  void addNode(const int nodeidx, const SbVec3f & xyz);
  void add3DElement(const int elementidx, const int32_t * nodes);
  void add2DElement(const int elementidx, const int32_t * nodes);

  void setNodeColor(const int nodeidx, const SbColor & color);
  void setElementColor(const int elementidx, const SbColor & color);

  void create3DIndices(int32_t * idxarray, const int32_t * nodes); 
  void create2DIndices(int32_t * idxarray, const int32_t * nodes); 
  
private:

  void updateScene(void);
  
  SoFEMKitP * pimpl;

};

#endif // COIN_SOFEM_H






