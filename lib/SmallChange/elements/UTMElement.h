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

#ifndef COIN_UTMELEMENT_H
#define COIN_UTMELEMENT_H

#include <Inventor/elements/SoElement.h>
#include <Inventor/elements/SoSubElement.h>
#include <Inventor/SbLinear.h>

class UTMElement : public SoElement {
  typedef SoElement inherited;

  SO_ELEMENT_HEADER(UTMElement);
public:
  static void initClass(void);
protected:
  virtual ~UTMElement();

public:
  virtual void init(SoState * state);
  virtual void push(SoState * state);

  static void setGlobalTransform(SoState * state,
                                 const SbMatrix & m);
  static const SbMatrix & getGlobalTransform(SoState * state);

  static void setReferencePosition(SoState * state,
                                   double easting, double northing,
                                   double elevation);
  
  static void getReferencePosition(SoState * state,
                                   double & easting, double & northing,
                                   double & elevation);

  static SbVec3f setPosition(SoState * state,
                             double easting,
                             double northing,
                             double elevation);
  
  static const SbVec3f & getCurrentTranslation(SoState * state);

  virtual SbBool matches(const SoElement * element) const;
  virtual SoElement * copyMatchInfo(void) const;

protected:

  double easting;
  double northing;
  double elevation;

  SbVec3f currtrans;
  SbMatrix gtransform;
};


#endif // !COIN_UTMELEMENT_H

