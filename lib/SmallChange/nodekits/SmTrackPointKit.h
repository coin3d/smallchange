#ifndef SMALLCHANGE_TRACKPOINTKIT_H
#define SMALLCHANGE_TRACKPOINTKIT_H

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

#include <Inventor/SbTime.h>
#include <Inventor/nodekits/SoBaseKit.h>
#include <Inventor/fields/SoSFInt32.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoMFTime.h>
#include <SmallChange/basic.h>

class SbVec3f;

class SMALLCHANGE_DLL_API SmTrackPointKit : public SoBaseKit {
  typedef SoBaseKit inherited;
  SO_KIT_HEADER(SmTrackPointKit);

  SO_KIT_CATALOG_ENTRY_HEADER(topSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(utmPosition);
  SO_KIT_CATALOG_ENTRY_HEADER(coord3);
  SO_KIT_CATALOG_ENTRY_HEADER(drawStyle);
  SO_KIT_CATALOG_ENTRY_HEADER(pointSet);

public:
  SoSFFloat trackLength;
  SoMFTime timeStamps;

public:
  static void initClass(void);
  SmTrackPointKit(void);

  void updateNumPoints(void);
  void addTrackPoint(const SbVec3f & pos, 
                     const SbTime & timestamp = SbTime::getTimeOfDay());

protected:
  virtual ~SmTrackPointKit(void);

};

#endif // SMALLCHANGE_TRACKPOINTKIT_H

