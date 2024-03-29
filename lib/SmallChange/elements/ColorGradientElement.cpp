/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2003 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using SmallChange with software that can not be combined with the
 *  GNU GPL, and for taking advantage of the additional benefits of our
 *  support services, please contact Systems in Motion about acquiring
 *  a Coin Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org> for  more information.
 *
 *  Systems in Motion, Teknobyen, Abels Gate 5, 7030 Trondheim, NORWAY.
 *  <URL:http://www.sim.no>.
 *
\**************************************************************************/

/*!
  \class ColorGradientElement ColorGradientElement.h SmallChange/elements/ColorGradientElement.h
  \brief The ColorGradientElement class is yet to be documented.

  FIXME: write doc.
*/

#include "SmColorGradientElement.h"

// avoid problems with Microsoft Win32 API headers (they define
// RELATIVE and ABSOLUTE in their header files).
#undef RELATIVE
#undef ABSOLUTE

SO_ELEMENT_SOURCE(SmColorGradientElement);


// doc from parent
void
SmColorGradientElement::initClass()
{
  static int initialized = 0;
  if (!initialized) {
    SO_ELEMENT_INIT_CLASS(SmColorGradientElement, inherited);
    initialized = 1;
  }
}

// doc from parent
void
SmColorGradientElement::init(SoState *state)
{
  inherited::init(state);
  this->numparams = 0;
  this->params = NULL;
  this->colors = NULL;
  this->mapping = RELATIVE;
  
}

void 
SmColorGradientElement::push(SoState * state)
{
  inherited::push(state);
  //   SmColorGradientElement * prev = (SmColorGradientElement*) this->getNextInStack();
}

/*!
  The destructor.
*/
SmColorGradientElement::~SmColorGradientElement()
{
}

/*!
  Set current reference position.
*/
void 
SmColorGradientElement::set(SoState * state,
                            SoNode * node,
                            const Mapping & mapping,
                            const int numparams,
                            const float * params,
                            const SbColor * colors)
{
  if (numparams) {
    SmColorGradientElement * elem = (SmColorGradientElement*)
      SoReplacedElement::getElement(state, classStackIndex, node);
    
    elem->mapping = mapping;
    elem->numparams = numparams;
    elem->params = params;
    elem->colors = colors;
  }
  else {
    SmColorGradientElement * elem = (SmColorGradientElement*)
      SoReplacedElement::getElement(state, classStackIndex, NULL);
    elem->mapping = mapping;
    elem->numparams = 0;
    elem->params = NULL;
    elem->colors = NULL;
  }
}

  
void 
SmColorGradientElement::get(SoState * const state,
                            Mapping & mapping,
                            int & numparams,
                            const float *& params,
                            const SbColor *& colors)
{
  const SmColorGradientElement * elem = (const SmColorGradientElement*)
    SoElement::getConstElement(state, classStackIndex);

  mapping = elem->mapping;
  numparams = elem->numparams;
  params = elem->params;
  colors = elem->colors;
}

