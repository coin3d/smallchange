#ifndef COIN_SBPLANE_H
#define COIN_SBPLANE_H

/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2003 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using Coin with software that can not be combined with the GNU
 *  GPL, and for taking advantage of the additional benefits of our
 *  support services, please contact Systems in Motion about acquiring
 *  a Coin Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org> for  more information.
 *
 *  Systems in Motion, Teknobyen, Abels Gate 5, 7030 Trondheim, NORWAY.
 *  <URL:http://www.sim.no>.
 *
\**************************************************************************/

#include <cstdio>

class SbLine;
class SbMatrix;

template <class Type> class SbVec3;

template <class Type>
class SbPlane {
public:
  SbPlane(void) { }
  SbPlane(const SbVec3<Type> & N, const Type D) : normal(N), distance(D) {
    this->normal.normalize();
  }

  SbPlane(const SbVec3<Type> & p0, const SbVec3<Type> & p1, const SbVec3<Type> & p2) {
    this->normal = (p1 - p0).cross(p2 - p0);
    assert(this->normal.sqrLength() != 0.0f); // points on line
    this->normal.normalize();
    this->distance = this->normal.dot(p0);
  }

  Type getDistance(const SbVec3<Type> & point) const {
    return point.dot(this->normal) - this->distance;
  }

  const SbVec3<Type> & getNormal(void) const {
    return this->normal;
  }

  void offset(const Type d) {
    this->distance += d;
  }

  int isInHalfSpace(const SbVec3<Type> & point) const {
    return this->getDistance(point) >= 0.0f;
  }

  Type getDistanceFromOrigin(void) const {
    return this->distance;
  }

private:
  SbVec3<Type> normal;
  Type distance;

};

#endif // !COIN_SBPLANE_H
