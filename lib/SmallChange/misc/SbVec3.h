#ifndef COIN_SBVEC3_H
#define COIN_SBVEC3_H

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

#include <cassert>
#include <cstddef>

template <class Type>
class SbVec3 {
public:
  SbVec3(void) { }

  SbVec3(const Type v[3]) {
    this->vec[0] = v[0]; this->vec[1] = v[1]; this->vec[2] = v[2];
  }

  SbVec3(const Type x, const Type y, const Type z) {
    this->vec[0] = x; this->vec[1] = y; this->vec[2] = z;
  }

  SbVec3<Type> & setValue(const Type v[3]) {
    this->vec[0] = v[0]; this->vec[1] = v[1]; this->vec[2] = v[2];
    return *this;
  }

  SbVec3<Type> & setValue(const Type x, const Type y, const Type z) {
    this->vec[0] = x; this->vec[1] = y; this->vec[2] = z;
    return *this;
  }

  const Type * getValue(void) const {
    return this->vec;
  }

  void getValue(Type & x, Type & y, Type & z) const {
    x = this->vec[0]; y = this->vec[1]; z = this->vec[2];
  }

  SbVec3<Type> cross(const SbVec3<Type> & v) const {
    return SbVec3<Type>(this->vec[1]*v.vec[2] - this->vec[2]*v.vec[1],
                        this->vec[2]*v.vec[0] - this->vec[0]*v.vec[2],
                        this->vec[0]*v.vec[1] - this->vec[1]*v.vec[0]);
  }

  Type dot(const SbVec3<Type> & v) const {
    return this->vec[0]*v.vec[0] + this->vec[1]*v.vec[1] + this->vec[2]*v.vec[2];
  }

  Type sqrLength(void) const {
    return this->vec[0]*this->vec[0] + this->vec[1]*this->vec[1] + this->vec[2]*this->vec[2];
  }

  double length(void) const {
    return sqrt(this->sqrLength());
  }

  double normalize(void) {
    const double len = this->length();
    if ( len != 0.0 ) {
      operator /= ((Type) len);
    }
    return len;
  }

  void negate(void) {
    this->vec[0] = -this->vec[0];
    this->vec[1] = -this->vec[1];
    this->vec[2] = -this->vec[2];
  }

  SbVec3<Type> & operator *= (const Type d) {
    this->vec[0] *= d;
    this->vec[1] *= d;
    this->vec[2] *= d;
    return *this;
  }

  SbVec3<Type> & operator /= (const Type d) {
    return operator *= (((Type) 1.0) / d);
  }

  SbVec3<Type> & operator += (const SbVec3<Type> & u) {
    this->vec[0] += u.vec[0];
    this->vec[1] += u.vec[1];
    this->vec[2] += u.vec[2];
    return *this;
  }
  SbVec3<Type> & operator -= (const SbVec3<Type> & u) {
    this->vec[0] -= u.vec[0];
    this->vec[1] -= u.vec[1];
    this->vec[2] -= u.vec[2];
    return *this;
  }

  SbVec3<Type> operator - (void) const {
    return SbVec3<Type>(-this->vec[0], -this->vec[1], -this->vec[2]);
  }

  SbVec3<Type> operator + (const SbVec3<Type> & v) const {
    return SbVec3<Type>(this->vec[0] + v.vec[0], this->vec[1] + v.vec[1], this->vec[2] + v.vec[2]);
  }

  SbVec3<Type> operator - (const SbVec3<Type> & v) const {
    return SbVec3<Type>(this->vec[0] - v.vec[0], this->vec[1] - v.vec[1], this->vec[2] - v.vec[2]);
  }

  SbVec3<Type> operator * (Type f) const {
    return SbVec3<Type>(this->vec[0] * f, this->vec[1] * f, this->vec[2] * f);
  }

  SbVec3<Type> operator / (Type f) const {
    return SbVec3<Type>(this->vec[0] / f, this->vec[1] / f, this->vec[2] / f);
  }

  Type & operator [] (int idx) {
    return this->vec[idx];
  }

  const Type & operator [] (int idx) const {
    return this->vec[idx];
  }

private:
  Type vec[3];

};

#endif // !COIN_SBVEC3_H
