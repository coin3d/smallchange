#ifndef COIN_SMHQSPHERE_H
#define COIN_SMHQSPHERE_H

/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2003 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SmallChange with software that can not be combined with the
 *  GNU GPL, and for taking advantage of the additional benefits of our
 *  support services, please contact Systems in Motion about acquiring
 *  a Coin Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org> for  more information.
 *
 *  Systems in Motion, Teknobyen, Abels Gate 5, 7030 Trondheim, NORWAY.
 *  <URL:http://www.sim.no>.
 *
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

