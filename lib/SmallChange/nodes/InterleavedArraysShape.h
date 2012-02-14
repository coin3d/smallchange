#ifndef INTERLEAVEDARRAYSSHAPE_H
#define INTERLEAVEDARRAYSSHAPE_H

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
#include <Inventor/nodes/SoVertexShape.h>
#include <Inventor/fields/SoMFInt32.h>
#include <Inventor/fields/SoSFEnum.h>

class SoPickedPoint;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinLeaveScope.h>
#endif // win

#include <SmallChange/basic.h>

class SMALLCHANGE_DLL_API InterleavedArraysShape : public SoVertexShape {
  typedef SoShape inherited;
  
  SO_NODE_HEADER(InterleavedArraysShape);
  
 public:
  static void initClass(void);
  InterleavedArraysShape(void);
  
  enum Type {
    POINTS,
    LINES,
    TRIANGLES,
    QUADS,
    TRIANGLE_STRIP,
    QUAD_STRIP
  };

  SoSFEnum type;
  SoMFInt32 vertexIndex;

  virtual void GLRender(SoGLRenderAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);
  virtual void rayPick(SoRayPickAction * action);

protected:
  virtual void notify(SoNotList * nl);
  virtual ~InterleavedArraysShape();
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

private:
  enum Binding {
    OVERALL,
    PER_VERTEX
  };

  SbBool shouldIncrementFaceIndex(Type type, int i) const;
  SoPickedPoint * tryPickTriangle(SoRayPickAction * action,
                                  const SbVec3f * vertices,
                                  int v0, int v1, int v2,
                                  SbVec3f & barycentric);
                       
  Binding findMaterialBinding(SoState * state) const;
  Binding findNormalBinding(SoState * state) const;
  void enableArrays(SoGLRenderAction * action, SbBool normals, SbBool colors, SbBool texcoords);
  void disableArrays(SoGLRenderAction * action, SbBool normals, SbBool colors, SbBool texcoords);
  void createVBO(uint32_t contextid);
  void createIndexVBO(uint32_t contextid);
  class Pimpl;
  Pimpl * pimpl;
  class VBO;
};

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinEnterScope.h>
#endif // win

#endif // INTERLEAVEDARRAYSSHAPE_H
