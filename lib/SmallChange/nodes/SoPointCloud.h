#ifndef SMALLCHANGE_SOPOINTCLOUD_H
#define SMALLCHANGE_SOPOINTCLOUD_H

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

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoNonIndexedShape.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoMFFloat.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFEnum.h>

#include <SmallChange/basic.h>

class SoPointCloudP;

class SMALLCHANGE_DLL_API SoPointCloud : public SoNonIndexedShape {
  typedef SoNonIndexedShape inherited;

  SO_NODE_HEADER(SoPointCloud);

public:
  static void initClass(void);
  SoPointCloud(void);

  SoSFInt32 numPoints;
  SoSFFloat detailDistance;
  SoSFFloat itemSize;

  enum Mode {
    ALWAYS_POINTS,
    DISTANCE_BASED,
    ALWAYS_SHAPE
  };
  SoSFEnum mode;

  enum Shape {
    BILLBOARD_DIAMOND,
    CUBE,
    SPHERE
  };
  SoSFEnum shape;

  virtual void GLRender(SoGLRenderAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);

protected:
  virtual ~SoPointCloud();

  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

  virtual SbBool generateDefaultNormals(SoState *, SoNormalCache * nc);
  virtual SbBool generateDefaultNormals(SoState * state,
                                        SoNormalBundle * bundle);

private:
  enum Binding {
    OVERALL = 0,
    PER_VERTEX
  };

  Binding findMaterialBinding(SoState * const state) const;

  SoPointCloudP * pimpl;
};

#endif // !SMALLCHANGE_SOPOINTSET_H
