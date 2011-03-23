#ifndef SMALLCHANGE_TRACKPOINTKIT_H
#define SMALLCHANGE_TRACKPOINTKIT_H

/**************************************************************************\
 *
 *  This file is part of the SmallChange extension library for Coin.
 *  Copyright (C) 1998-2006 by Systems in Motion.  All rights reserved.
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
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFUShort.h>
#include <SmallChange/basic.h>

class SbVec3d;
class SmTrack;
class SoMFVec3d; 
class SoMFTime;

class SMALLCHANGE_DLL_API SmTrackPointKit : public SoBaseKit {
  typedef SoBaseKit inherited;
  SO_KIT_HEADER(SmTrackPointKit);

  SO_KIT_CATALOG_ENTRY_HEADER(topSeparator);
  SO_KIT_CATALOG_ENTRY_HEADER(utmPosition);
  SO_KIT_CATALOG_ENTRY_HEADER(appearanceKit);
  SO_KIT_CATALOG_ENTRY_HEADER(track);

public:
  SoSFFloat trackLength;
  SoSFUShort pointInterval;
  SoSFUShort lineInterval;
  SoSFUShort tickInterval;
  SoSFFloat tickSize;

public:
  static void initClass(void);
  SmTrackPointKit(void);

  void addTrackPoint(const SbVec3d & pos, 
                     const SbTime & timestamp = SbTime::getTimeOfDay());
  void addTrackPoints(const SoMFVec3d & positions, const SoMFTime & timestamps ); 
  void setTrackPoints(const SoMFVec3d & positions, const SoMFTime & timestamps );

  SmTrack * getTrack();
	
protected:
  virtual ~SmTrackPointKit(void);

};

#endif // SMALLCHANGE_TRACKPOINTKIT_H
