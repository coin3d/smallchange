#ifndef SMALLCHANGE_COINBOARD_H
#define SMALLCHANGE_COINBOARD_H

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
#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoMFVec3f.h>
#include <Inventor/fields/SoMFVec4f.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoMFInt32.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <SoWinLeaveScope.h>
#endif // win

#include <SmallChange/basic.h>

class SbMatrix;


class SMALLCHANGE_DLL_API Coinboard : public SoShape {
  typedef SoShape inherited;

  SO_NODE_HEADER(Coinboard);

public:
  static void initClass(void);
  Coinboard(void);

  SoMFVec3f position;
  SoSFVec3f axisOfRotation;
  SoSFInt32 frontAxis;

  enum ShapeType {
    TRIANGLES,
    QUADS,
    TRIANGLE_STRIP,
    TRIANGLE_FAN
  };

  SoMFVec3f coord;
  SoMFVec4f texCoord;
  SoSFVec3f normal;
  SoSFEnum shapeType;
  SoMFInt32 numVertices;

  virtual void GLRender(SoGLRenderAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);

protected:
  virtual ~Coinboard();
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

private:

  void render(const SbVec3f & offset,
              const SbVec3f * coords, const int n,
              const SbVec4f * texcoords = NULL);

  void render(const SbMatrix & matrix,
              const SbVec3f * coords, const int n,
              const SbVec4f * texcoords = NULL);

  void generate(SoPrimitiveVertex * v,
                const SbVec3f & offset,
                const SbVec3f * coords, const int n,
                const SbVec4f * texcoords = NULL);

  void generate(SoPrimitiveVertex * v,
                const SbMatrix & transform,
                const SbVec3f * coords, const int n,
                const SbVec4f * texcoords = NULL);
};

#endif // !SMALLCHANGE_COINBOARD_H
