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

#ifndef SMALLCHANGE_RANGETRANSLATE1DRAGGER_H
#define SMALLCHANGE_RANGETRANSLATE1DRAGGER_H

#include <Inventor/draggers/SoDragger.h>
#include <Inventor/fields/SoSFVec2f.h>

#include <SmallChange/basic.h>


class SMALLCHANGE_DLL_API SmRangeTranslate1Dragger : public SoDragger {

  typedef SoDragger inherited;
  SO_KIT_HEADER(SmRangeTranslate1Dragger);
  SO_KIT_CATALOG_ENTRY_HEADER(topSep);

public:
  SmRangeTranslate1Dragger(void);
  static void initClass(void);
  virtual SbBool affectsState(void) const;

  SoSFVec2f range;
  // FIXME: shouldn't this rather be an SoSFFloat? 20031023 mortene.
  SoSFVec3f translation;

protected:
  virtual ~SmRangeTranslate1Dragger();

private:
  friend class SmRangeTranslate1DraggerP;
  class SmRangeTranslate1DraggerP * pimpl;
  void initSmRangeTranslate1Dragger();

};

#endif // ! SMALLCHANGE_RANGETRANSLATE1DRAGGER_H
