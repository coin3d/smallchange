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
  \class SoAngle1Manip SoAngle1Manip.h 
  \brief The SoAngle1Manip class is for rotating geometry around a single axis.
  \ingroup draggers

  Use an instance of this dragger class in your scenegraph to let the
  end-users of your application rotate geometry around a pre-defined
  axis vector in 3D.

  For the dragger orientation and positioning itself, use some kind of
  transformation node in your scenegraph, as usual.
*/

#include "SoAngle1Manip.h"
#include "SoAngle1Dragger.h"
#include <Inventor/nodes/SoSurroundScale.h>


SO_NODE_SOURCE(SoAngle1Manip);


void SoAngle1Manip::initClass()
{
  static int first = 0;
  if (first == 1) return;
  first = 1;

  SO_NODE_INIT_CLASS(SoAngle1Manip, SoTransformManip, "TransformManip");

  SoAngle1Dragger::initClass();
}//initClass


SoAngle1Manip::SoAngle1Manip()
{
  SO_NODE_CONSTRUCTOR(SoAngle1Manip);

  setDragger(new SoAngle1Dragger);
}//Constructor


SoAngle1Manip::~SoAngle1Manip()
{
}//Destructor
