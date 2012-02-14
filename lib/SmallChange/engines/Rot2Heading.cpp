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

/*!
  \class Rot2Heading Rot2Heading.h
  \brief Rot2Heading is a class for transforming a rotation into a vector.

  It is a useful engine, since it can, for instance, be used to
  create a headlight in a scene graph by connecting the camera
  orientation to the input, and a directional light direction to the
  output.
*/

/*!
  \var SoSFBool Rot2Heading::inverse
  Set to \e TRUE if the rotation should be inverted.
*/

/*!
  \var SoMFRotation Rot2Heading::rotation
  The rotation input.
*/

/*!  
  \var SoEngineOutput Rot2Heading::heading
  (SoSFVec3f) The rotated vector.
*/


#include "Rot2Heading.h"

SO_ENGINE_SOURCE(Rot2Heading);

// doc from parent
void
Rot2Heading::initClass(void)
{
  static int first = 1;
  if (first) {
    first = 0;
    SO_ENGINE_INIT_CLASS(Rot2Heading, SoEngine, "Engine");
  }
}

/*!
  Constructor.
*/
Rot2Heading::Rot2Heading(void)
{
  SO_ENGINE_CONSTRUCTOR(Rot2Heading);
  
  SO_ENGINE_ADD_INPUT(rotation, (SbRotation::identity()));
  SO_ENGINE_ADD_INPUT(inverse, (FALSE));

  SO_ENGINE_ADD_OUTPUT(heading, SoMFVec3f);  
}

/*!
  Destructor.
*/
Rot2Heading::~Rot2Heading()
{
}

// doc from parent
void
Rot2Heading::evaluate(void)
{
  int num = this->rotation.getNum();
  SO_ENGINE_OUTPUT(heading, SoMFVec3f, setNum(num));
  
  SbVec3f tmp;
  for (int i = 0; i < num; i++) {
    tmp.setValue(0.0f, 0.0f, -1.0f);
    if (this->inverse.getValue()) {
      this->rotation[i].inverse().multVec(tmp, tmp);
    }
    else {
      this->rotation[i].multVec(tmp, tmp);
    }
    SO_ENGINE_OUTPUT(heading, SoMFVec3f, set1Value(i, tmp));
  }
}
