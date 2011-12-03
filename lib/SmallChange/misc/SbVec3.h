#ifndef COIN_SBVEC3_H
#define COIN_SBVEC3_H

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
