#ifndef COIN_SBBOX3_H
#define COIN_SBBOX3_H

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

template <class Type> class SbVec3;


template <class Type>
class SbBox3 {
public:
  SbBox3(void) { }
  SbBox3(const Type minx, const Type miny, const Type minz,
         const Type maxx, const Type maxy, const Type maxz)
    : minvec(minx, miny, minz), maxvec(maxx, maxy, maxz) { }
  SbBox3(const SbVec3<Type> & minv, const SbVec3<Type> & maxv)
    : minvec(minv), maxvec(maxv) { }

  const SbVec3<Type> & getMin(void) const { return this->minvec; }
  const SbVec3<Type> & getMax(void) const { return this->maxvec; }
  SbVec3<Type> & getMin(void) { return this->minvec; }
  SbVec3<Type> & getMax(void) { return this->maxvec; }

  void extendBy(const SbVec3<Type> & point) {
    this->minvec.setValue(SbBoxMin(point[0], this->minvec[0]),
                          SbBoxMin(point[1], this->minvec[1]),
                          SbBoxMin(point[2], this->minvec[2]));
    this->maxvec.setValue(SbBoxMax(point[0], this->maxvec[0]),
			  SbBoxMax(point[1], this->maxvec[1]),
			  SbBoxMax(point[2], this->maxvec[2]));
  }

private:
  SbVec3<Type> minvec, maxvec;

  static Type SbBoxMin(Type a, Type b) { return (a < b) ? a : b; }
  static Type SbBoxMax(Type a, Type b) { return (a > b) ? a : b; }

};

#endif // !COIN_SBBOX3_H
