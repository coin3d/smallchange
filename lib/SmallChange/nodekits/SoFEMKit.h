#ifndef SMALLCHANGE_SOFEM_H
#define SMALLCHANGE_SOFEM_H

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
  static void update_cb(void * data, SoSensor * sensor);
  SoFEMKitP * pimpl;

};

#endif // !SMALLCHANGE_SOFEM_H
