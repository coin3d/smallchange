#ifndef SMALLCHANGE_COLORGRADIENTELEMENT_H
#define SMALLCHANGE_COLORGRADIENTELEMENT_H

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

#include <Inventor/elements/SoReplacedElement.h>
#include <Inventor/elements/SoSubElement.h>
#include <Inventor/SbLinear.h>

// Avoid problem with Microsoft Visual C++ Win32 API headers (they
// #define RELATIVE/ABSOLUTE in their header files (WINGDI.H)
#ifdef RELATIVE
#define SMCOLORGRADIENTELEMENT_RELATIVE_DEFINED
#undef RELATIVE
#endif // RELATIVE

#ifdef ABSOLUTE
#define SMCOLORGRADIENTELEMENT_ABSOLUTE_DEFINED
#undef ABSOLUTE
#endif // ABSOLUTE

#include <SmallChange/basic.h>
class SbColor;

class SMALLCHANGE_DLL_API SmColorGradientElement : public SoReplacedElement {
  typedef SoReplacedElement inherited;

  SO_ELEMENT_HEADER(SmColorGradientElement);

public:

  enum Mapping {
    RELATIVE,
    ABSOLUTE
  };

  static void initClass(void);

  virtual void init(SoState * state);
  virtual void push(SoState * state);

  static void set(SoState * state, SoNode * node,
                  const Mapping & mapping,
                  const int numparams,
                  const float * params,
                  const SbColor * colors);
  
  static void get(SoState * state,
                  Mapping & mapping,
                  int & numparams,
                  const float *& params,
                  const SbColor *& colors);

protected:
  virtual ~SmColorGradientElement();

private:
  Mapping mapping;
  int numparams;
  const float * params;
  const SbColor * colors;
  
};

// fix for Windows header files (see above)
#ifdef SMCOLORGRADIENTELEMENT_RELATIVE_DEFINED
#define RELATIVE 2
#undef SMCOLORGRADIENTELEMENT_RELATIVE_DEFINED
#endif // SMCOLORGRADIENTELEMENT_RELATIVE_DEFINED

#ifdef SMCOLORGRADIENTELEMENT_ABSOLUTE_DEFINED
#define ABSOLUTE 1
#undef SMCOLORGRADIENTELEMENT_ABSOLUTE_DEFINED
#endif // SMCOLORGRADIENTELEMENT_ABSOLUTE_DEFINED

#endif // !SMALLCHANGE_COLORGRADIENTELEMENT_H
