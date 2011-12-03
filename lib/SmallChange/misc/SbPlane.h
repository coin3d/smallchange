#ifndef COIN_SBPLANE_H
#define COIN_SBPLANE_H

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
