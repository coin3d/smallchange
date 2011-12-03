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
  \class UTMElement UTMElement.h SmallChange/elements/UTMElement.h
  \brief The UTMElement class is yet to be documented.

  FIXME: write doc.
*/

#include "UTMElement.h"

SO_ELEMENT_SOURCE(UTMElement);


// doc from parent
void
UTMElement::initClass()
{
  SO_ELEMENT_INIT_CLASS(UTMElement, inherited);
}

// doc from parent
void
UTMElement::init(SoState *state)
{
  inherited::init(state);
  this->easting = 0.0;
  this->northing = 0.0;
  this->elevation = 0.0;
  this->currtrans.setValue(0.0f, 0.0f, 0.0f);
  this->gtransform = SbMatrix::identity();
}

void 
UTMElement::push(SoState * state)
{
  inherited::push(state);
  UTMElement * prev = (UTMElement*) this->getNextInStack();
  this->easting = prev->easting;
  this->northing = prev->northing;
  this->elevation = prev->elevation;
  this->currtrans = prev->currtrans;
  this->gtransform = prev->gtransform;
}

/*!
  The destructor.
*/
UTMElement::~UTMElement()
{
}

void 
UTMElement::setGlobalTransform(SoState * state,
                               const SbMatrix & m)
{
  UTMElement * elem = (UTMElement*)
    SoElement::getElement(state, classStackIndex);
  elem->gtransform = m;
}

const SbMatrix & 
UTMElement::getGlobalTransform(SoState * state)
{
  const UTMElement * elem = (const UTMElement*)
    SoElement::getConstElement(state, classStackIndex);
  return elem->gtransform;
}


/*!
  Set current reference position.
*/
void 
UTMElement::setReferencePosition(SoState * const state,
                                 double easting, double northing,
                                 double elevation)
{
  UTMElement * elem = (UTMElement*)
    SoElement::getElement(state, classStackIndex);

  elem->easting = easting;
  elem->northing = northing;
  elem->elevation = elevation;
  elem->currtrans = SbVec3f(0.0f, 0.0f, 0.0f);
}
  
void 
UTMElement::getReferencePosition(SoState * const state,
                                 double & easting, double & northing,
                                 double & elevation)
{
  const UTMElement * elem = (const UTMElement*)
    SoElement::getConstElement(state, classStackIndex);
  
  easting = elem->easting;
  northing = elem->northing;
  elevation = elem->elevation;
}

SbVec3f 
UTMElement::setPosition(SoState * state,
                        double easting,
                        double northing,
                        double elevation)
{
  UTMElement * elem = (UTMElement*)
    SoElement::getElement(state, classStackIndex);
  
  SbVec3f newtrans = SbVec3f(float(easting - elem->easting), 
                             float(northing - elem->northing),
                             float(elevation - elem->elevation));
  
  SbVec3f diff = newtrans - elem->currtrans;
  elem->currtrans = newtrans;
  return newtrans;
  //return diff;
}

const SbVec3f & 
UTMElement::getCurrentTranslation(SoState * state)
{
  const UTMElement * elem = (const UTMElement*)
    SoElement::getConstElement(state, classStackIndex);
  return elem->currtrans;
}

SbBool 
UTMElement::matches(const SoElement * element) const
{
#if 1
  return TRUE;
#else
  UTMElement * elem = (UTMElement*) element;
  SbBool ret = 
    this->easting == elem->easting &&
    this->northing == elem->northing &&
    this->elevation == elem->elevation;
  if (ret == FALSE) {
    //    fprintf(stderr,"blown\n");
  }
  return ret;
#endif
}

SoElement * 
UTMElement::copyMatchInfo(void) const
{
  UTMElement * element =
    (UTMElement *)(this->getTypeId().createInstance());
  element->easting = this->easting;
  element->northing = this->northing;
  element->elevation = this->elevation;
  return (SoElement *)element;
}
