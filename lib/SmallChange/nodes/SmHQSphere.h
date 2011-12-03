#ifndef COIN_SMHQSPHERE_H
#define COIN_SMHQSPHERE_H

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
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/SbLinear.h>
#include <SmallChange/basic.h>

class SmHQSphereP;
class SbBSPTree;

class SMALLCHANGE_DLL_API SmHQSphere : public SoShape {
  typedef SoShape inherited;

  SO_NODE_HEADER(SmHQSphere);

public:
  static void initClass(void);
  SmHQSphere(void);

  SoSFFloat radius;
  SoSFInt32 level;

  virtual void GLRender(SoGLRenderAction * action);
  virtual void getPrimitiveCount(SoGetPrimitiveCountAction * action);
  virtual void rayPick(SoRayPickAction * action);

protected:
  virtual ~SmHQSphere();
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

private:
  SmHQSphereP * pimpl;
};

class SMALLCHANGE_DLL_API HQSphereGenerator {
public:
  HQSphereGenerator(void) {
    this->orgobject = NULL;
    this->init();
  }
  ~HQSphereGenerator(void) {
    delete this->orgobject;
  }

  class triangle {
  public:
    triangle(void) { }
    triangle(const SbVec3f & p0, const SbVec3f & p1, const SbVec3f & p2) {
      pt[0] = p0;
      pt[1] = p1;
      pt[2] = p2;
    }
  public:
    SbVec3f pt[3];
  };

  class object {
  public:
    object(int npoly, const triangle * poly) {
      this->npoly = npoly;
      this->poly = new triangle[npoly];
      if (poly) {
        memcpy(this->poly, poly, npoly*sizeof(triangle));
      }
    }
    ~object() {
      delete[] this->poly;
    }
  public:
    int npoly;    /* # of triangles in object */
    triangle * poly;     /* Triangles */
  };

  void generate(const int level, SbBSPTree & bsp, SbList <int> & idx);

  SbVec3f normalize(const SbVec3f & p);
  SbVec3f midpoint(const SbVec3f & a, const SbVec3f & b);

private:
  void convert(object * obj, SbBSPTree & bsp, SbList <int> & idx);
  void init(void);
  object * orgobject;
};

#endif // COIN_SMHQSPHERE_H
