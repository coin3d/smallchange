#ifndef SMALLCHANGE_SOANGLE1DRAGGER_H
#define SMALLCHANGE_SOANGLE1DRAGGER_H

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

#include <Inventor/draggers/SoDragger.h>
#include <Inventor/fields/SoSFRotation.h>
#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <Inventor/sensors/SoFieldSensor.h>

#include <SmallChange/basic.h>

class SoSensor;
class SoFieldSensor;
class SbCylinderProjector;


class SMALLCHANGE_DLL_API SoAngle1Dragger : public SoDragger {
  typedef SoDragger inherited;

  SO_KIT_HEADER(SoAngle1Dragger);

  SO_KIT_CATALOG_ENTRY_HEADER(rotatorSwitch);
  SO_KIT_CATALOG_ENTRY_HEADER(rotator);
  SO_KIT_CATALOG_ENTRY_HEADER(activeRotator);
  SO_KIT_CATALOG_ENTRY_HEADER(material);
  SO_KIT_CATALOG_ENTRY_HEADER(geometry);
  SO_KIT_CATALOG_ENTRY_HEADER(activeMaterial);
  SO_KIT_CATALOG_ENTRY_HEADER(activeGeometry);


public:
  static void initClass(void);
  SoAngle1Dragger(void);

  // Fields
  SoSFFloat angle;

  void setProjector(SbCylinderProjector * p);
  const SbCylinderProjector * getProjector(void) const;

protected:
  ~SoAngle1Dragger();
  virtual SbBool setUpConnections(SbBool onoff, SbBool doitalways = FALSE);

  virtual void copyContents(const SoFieldContainer * fromfc,
                            SbBool copyconnections);

  static void startCB(void * f, SoDragger * d);
  static void motionCB(void * f, SoDragger * d);
  static void doneCB(void * f, SoDragger * d);

  void dragStart(void);
  void drag(void);
  void dragFinish(void);

private:
  friend class SoAngle1DraggerP;
  class SoAngle1DraggerP* pimpl;

};

#endif // !SMALLCHANGE_SOANGLE1DRAGGER_H
