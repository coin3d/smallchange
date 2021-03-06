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

#ifndef COIN_VERTEXARRAYSHAPE_H
#define COIN_VERTEXARRAYSHAPE_H

#include <Inventor/nodes/SoShape.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/fields/SoMFInt32.h>
#include <Inventor/fields/SoSFNode.h>
#include <Inventor/fields/SoSFEnum.h>

#include <SmallChange/basic.h>

class SmVertexArrayShapeP;

class SMALLCHANGE_DLL_API SmVertexArrayShape : public SoShape {
  typedef SoShape inherited;

  SO_NODE_HEADER(SmVertexArrayShape);

public:
  static void initClass(void);
  SmVertexArrayShape(void);

  enum ShapeType {
    POINTS = -1,
    LINES = -2,
    LINE_LOOP = -3,
    LINE_STRIP = -4,
    TRIANGLES = -5,
    TRIANGLE_STRIP = -6,
    TRIANGLE_FAN = -7,
    QUADS = -8,
    QUAD_STRIP = -9,
    POLYGON = -10
  };

  enum RenderAsVertexBufferObjects {
    OFF, ON, AUTO
  };

  SoSFEnum renderAsVertexBufferObject;
  SoMFInt32 vertexIndex;
  SoSFNode vertexCoord;
  SoSFNode vertexNormal;
  SoSFNode vertexTexCoord;
  SoSFNode vertexColor;

  virtual void GLRender(SoGLRenderAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);

protected:
  virtual ~SmVertexArrayShape();

  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);
  virtual void notify(SoNotList * list);

private:
  SmVertexArrayShapeP * pimpl;
};

#endif // COIN_VERTEXARRAYSHAPE_H
