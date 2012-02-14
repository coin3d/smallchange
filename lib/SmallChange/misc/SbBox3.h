#ifndef COIN_SBBOX3_H
#define COIN_SBBOX3_H

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
