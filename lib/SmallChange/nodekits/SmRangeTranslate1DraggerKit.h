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

#ifndef SMALLCHANGE_RANGETRANSLATE1DRAGGERKIT_H
#define SMALLCHANGE_RANGETRANSLATE1DRAGGERKIT_H

#include <Inventor/nodekits/SoInteractionKit.h>
#include <Inventor/fields/SoMFVec2f.h>

class SmRangeTranslate1DraggerKit : public SoInteractionKit {

  typedef SoInteractionKit inherited;
  SO_KIT_HEADER(SmRangeTranslate1DraggerKit);
  SO_KIT_CATALOG_ENTRY_HEADER(topSep);

public:
  SmRangeTranslate1DraggerKit(void);
  static void initClass(void);
  virtual SbBool affectsState(void) const;

  SoMFVec2f range;
  SoSFVec3f translation;

protected:
  virtual ~SmRangeTranslate1DraggerKit();

private:
  friend class SmRangeTranslate1DraggerKitP;
  class SmRangeTranslate1DraggerKitP * pimpl;
  void initSmRangeTranslate1Dragger();

};

#endif // !SMALLCHANGE_RANGETRANSLATE1DRAGGERKIT_H

