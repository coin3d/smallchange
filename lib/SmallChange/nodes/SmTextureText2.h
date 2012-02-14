#ifndef SM_TEXTURE_TEXT2_H
#define SM_TEXTURE_TEXT2_H

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
#include <Inventor/fields/SoMFVec3f.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoMFFloat.h>
#include <Inventor/fields/SoMFInt32.h>
#include <Inventor/fields/SoMFString.h>
#include <Inventor/fields/SoSFVec3f.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoSFBool.h>
#include <SmallChange/basic.h>
#include <SmallChange/nodes/SmTextureFont.h>

class SbViewVolume;
class SbViewportRegion;
class SbMatrix;

class SMALLCHANGE_DLL_API SmTextureText2 : public SoShape {
  typedef SoShape inherited;

  SO_NODE_HEADER(SmTextureText2);

public:
  static void initClass(void);
  SmTextureText2(void);

  enum Justification {
    LEFT = 1,
    RIGHT,
    CENTER
  };

  enum VerticalJustification {
    BOTTOM = 1,
    TOP,
    VCENTER
  };

  SoMFString string;
  SoSFEnum justification;
  SoSFEnum verticalJustification;
  SoMFVec3f position;
  SoSFFloat maxRange;
  SoSFVec3f offset;
  SoMFFloat rotation;

  SoMFInt32 stringIndex;
  SoSFBool pickOnPixel;


  virtual void GLRender(SoGLRenderAction * action);
  virtual void rayPick(SoRayPickAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);
  virtual void buildStringQuad(SoAction * action, int idx, SbVec3f & p0, SbVec3f & p1, SbVec3f & p2, SbVec3f & p3);

protected:
  virtual ~SmTextureText2();

  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

  virtual int getStrings(SoState * state, const SbString * & strings) const;
  virtual int getPositions(SoState * state, const SbVec3f * & positions) const;
  virtual int getRotations(SoState * state, const float * & rotations) const;
  virtual int getStringIndices(SoState * state, const int32_t * & indices) const;

private:
  void renderString(const SmTextureFontBundle & bundle,
                    const SbString * s,
                    const int numstring,
                    const SbVec3f & pos,
                    const SbViewVolume & vv,
                    const SbViewportRegion & vp,
                    const SbMatrix & projmatrix,
                    const SbMatrix & modelmatrix,
                    const SbMatrix & invmodelmatrix,
                    const float rotation);

  static void render_text(unsigned char * dst,
                          const int idx,
                          const unsigned char value,
                          const unsigned char alpha);

};

#endif
