/**************************************************************************\
 *
 *  Copyright (C) 1998-2000 by Systems in Motion.  All rights reserved.
 *
 *  This file is part of the Coin library.
 *
 *  This file may be distributed under the terms of the Q Public License
 *  as defined by Troll Tech AS of Norway and appearing in the file
 *  LICENSE.QPL included in the packaging of this file.
 *
 *  If you want to use Coin in applications not covered by licenses
 *  compatible with the QPL, you can contact SIM to aquire a
 *  Professional Edition license for Coin.
 *
 *  Systems in Motion AS, Prof. Brochs gate 6, N-7030 Trondheim, NORWAY
 *  http://www.sim.no/ sales@sim.no Voice: +47 22114160 Fax: +47 67172912
 *
\**************************************************************************/

/*!
  \class UTMElement Inventor/elements/UTMElement.h
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
